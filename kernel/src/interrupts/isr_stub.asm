
[BITS 64]
[EXTERN isr_exception_handler:function]

%macro push_all 0
	push rax
	push rbx
	push rcx
	push rdx
	push rbp
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	mov rax, cr0
	push rax
	mov rax, cr2
	push rax
	mov rax, cr3
	push rax
	mov rax, cr4
	push rax
	mov rax, dr0
	push rax
	mov rax, dr1
	push rax
	mov rax, dr2
	push rax
	mov rax, dr3
	push rax
	mov rax, dr6
	push rax
	mov rax, dr7
	push rax
%endmacro

%macro pop_all 0
	add rsp, 80 ; "Pop" CRx and DRx registers from the stack.
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
%endmacro

%macro define_isr_with_code 1
GLOBAL isr%1:function
ALIGN 4
isr%1:
	push qword %1 	; ISR number

	push_all

	mov rdi, rsp 	; Set 'rdi'(First function argument) to the pointer of the
					; interrupt stack frame

	call isr_exception_handler

	pop_all

	add rsp, 8		; "POP" the ISR number off the stack.
	iretq
%endmacro

%macro define_isr_without_code 1
GLOBAL isr%1:function
isr%1:
	push qword 0	; Dummy error code
	push qword %1 	; ISR number

	push_all

	mov rdi, rsp 	; Set 'rdi'(First function argument) to the pointer of the
					;interrupt stack frame

	call isr_exception_handler

	pop_all

	add rsp, 16		; "POP" the ISR number and dummy error code off the stack.
	iretq
%endmacro

; ISR 0: Divide by Zero Exception
define_isr_without_code 0

; ISR 1: Debug Exception
define_isr_without_code 1

; ISR 2: NMI Exception
define_isr_without_code 2

; ISR 3: Breakpoint Exception
define_isr_without_code 3

; ISR 4: Overflow Exception
define_isr_without_code 4

; ISR 5: Bound-Range Exception
define_isr_without_code 5

; ISR 6: Invalid opcode Exception
define_isr_without_code 6

; ISR 7: Device not available Exception
define_isr_without_code 7

; ISR 8: Double fault Exception
define_isr_with_code 8

; ISR 9: Coprocessor-Segment-Overrun - Reserved in long mode

; ISR 10: Invalid-TSS Exception
define_isr_with_code 10

; ISR 11: Segment-Not-Present Exception
define_isr_with_code 11

; ISR 12: Stack Exception
define_isr_with_code 12

; ISR 13: General-Protection Exception
define_isr_with_code 13

; ISR 14: Page Fault Exception
define_isr_with_code 14

; ISR 15: Reserved

; ISR 16: x87 Floating-Point Exception
define_isr_without_code 16

; ISR 17: Aligment-Check Exception
define_isr_without_code 17

; ISR 18: Machine-Check Exception
define_isr_without_code 18

; ISR 19: SIMD Floating-Point Exception
define_isr_without_code 19

; ISR 20: Reserved

; ISR 21: Control Exception
define_isr_without_code 21

; ISR 22-27: Reserved

; ISR 28: Hypervisor Injection Exception
define_isr_without_code 28

; ISR 29: VMM Communication Exception
define_isr_without_code 29

; ISR 30: Security Exception
define_isr_without_code 30

; ISR 31: Reserved


