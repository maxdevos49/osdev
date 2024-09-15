[BITS 64]

SECTION .data
_gdtr:
	dw 0 ; For limit storage
	dq 0 ; For base storage

SECTION .text
GLOBAL load_gdt:function
load_gdt:
	; void load_gdt(uint16_t limit, uint64_t base)
	; INPUT:
	;	First param `limit` is contained in 'di'.
	; 	Second param 'base` is contained in rsi
	; OUTPUT:
	;	void
	mov   [_gdtr], di
	mov   [_gdtr+2], rsi
	lgdt  [_gdtr]
	ret

GLOBAL reload_segments:function
reload_segments:
	; void reload_segments(uint16_t code_segment, uint16_t data_segment)
	; INPUT:
	;	First param 'code_segment' is contained in 'di'.
	;	Second param 'data_segment' is contained in 'si'.
	; OUTPUT:
	;	void

	; Indirectly update the new code segment via a far return.

	movzx rdx, di
	lea  rcx, .reload_cs	; Load address of .reload_CS into Rax
	push rdx				; Push code segment to stack
	push rcx				; Push this value to the stack
	retfq					; Perform a far return. (Pops return address and code segment)
.reload_cs:

	; Update the new data segment.

	mov cx, si	; 'si' holds the offset of the data segment.
	mov ds, cx
	mov es, cx
	mov fs, cx
	mov gs, cx
	mov ss, cx

	ret

