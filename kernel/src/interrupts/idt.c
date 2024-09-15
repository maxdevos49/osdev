#include <stdint.h>
#include "idt.h"
#include "macro.h"
#include "instruction.h"
#include "error.h"
#include "string/utility.h"
#include "memory/stack.h"

struct IDT_REGISTER
{
	uint16_t limit;
	uint64_t base;
} ATTR_PACK;

// Long mode Interrupt-Gate/Trap Gate Descriptor
// AMD64 Programmer’s Manual, Volume 2: PG 102
struct INTERRUPT_GATE_DESCRIPTOR
{
	uint16_t target_offset_low;
	uint16_t target_selector;
	uint8_t
		ist : 3,
		reserved_0 : 5;
	uint8_t flags;
	uint16_t target_offset_mid;
	uint32_t target_offset_high;
	uint32_t reserved_1;
} ATTR_PACK;
_Static_assert(sizeof(struct INTERRUPT_GATE_DESCRIPTOR) == sizeof(uint64_t) * 2);

static struct INTERRUPT_GATE_DESCRIPTOR _idt[256] = {0};

// AMD64 Architecture programmers manual volume 2: Pg 284
struct INTERRUPT_STACK
{
	// Placed by the stub.
	uint64_t dr7;
	uint64_t dr6;
	uint64_t dr3;
	uint64_t dr2;
	uint64_t dr1;
	uint64_t dr0;
	uint64_t cr4;
	uint64_t cr3;
	uint64_t cr2;
	uint64_t cr0;
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rbp;
	uint64_t rdx;
	uint64_t rcx;
	uint64_t rbx;
	uint64_t rax;

	uint64_t vector;

	// Sometimes placed by CPU. Sometimes placed by the stub

	uint64_t error_code;

	// Placed by the CPU

	uint64_t return_rip;
	uint64_t return_cs;
	uint64_t return_rflags;
	uint64_t return_rsp;
	uint64_t return_ss;
};

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr21();
extern void isr28();
extern void isr29();
extern void isr30();

void load_idt(void);

void init_idt(void)
{
	printf(KINFO "Disabling interrupts\n");
	disable_interrupts();

	printf(KINFO "Setting up IDT entries...\n");

	set_interrupt_gate(0, (uintptr_t)isr0, 8, 0x8E);
	set_interrupt_gate(1, (uintptr_t)isr1, 8, 0x8E);
	set_interrupt_gate(2, (uintptr_t)isr2, 8, 0x8E);
	set_interrupt_gate(3, (uintptr_t)isr3, 8, 0x8E);
	set_interrupt_gate(4, (uintptr_t)isr4, 8, 0x8E);
	set_interrupt_gate(5, (uintptr_t)isr5, 8, 0x8E);
	set_interrupt_gate(6, (uintptr_t)isr6, 8, 0x8E);
	set_interrupt_gate(7, (uintptr_t)isr7, 8, 0x8E);
	set_interrupt_gate(8, (uintptr_t)isr8, 8, 0x8E);
	set_interrupt_gate(10, (uintptr_t)isr10, 8, 0x8E);
	set_interrupt_gate(11, (uintptr_t)isr11, 8, 0x8E);
	set_interrupt_gate(12, (uintptr_t)isr12, 8, 0x8E);
	set_interrupt_gate(13, (uintptr_t)isr13, 8, 0x8E);
	set_interrupt_gate(14, (uintptr_t)isr14, 8, 0x8E);
	set_interrupt_gate(16, (uintptr_t)isr16, 8, 0x8E);
	set_interrupt_gate(17, (uintptr_t)isr17, 8, 0x8E);
	set_interrupt_gate(18, (uintptr_t)isr18, 8, 0x8E);
	set_interrupt_gate(19, (uintptr_t)isr19, 8, 0x8E);
	set_interrupt_gate(21, (uintptr_t)isr21, 8, 0x8E);
	set_interrupt_gate(28, (uintptr_t)isr28, 8, 0x8E);
	set_interrupt_gate(29, (uintptr_t)isr29, 8, 0x8E);
	set_interrupt_gate(30, (uintptr_t)isr30, 8, 0x8E);

	printf(KINFO "Loading IDT Register...\n");
	printf("\tIDT base: %p\n", &_idt);
	printf("\tIDT limit: %'d bytes\n", sizeof(_idt));
	load_idt();
	printf(KOK "IDT Register loaded\n", sizeof(_idt));
}

void load_idt(void)
{
	struct IDT_REGISTER idtr = {
		.limit = sizeof(_idt),
		.base = (uintptr_t)&_idt};

	asm volatile("lidt %0" ::"m"(idtr));
}

void set_interrupt_gate(uint16_t index, uintptr_t target, uint16_t target_selector, uint16_t flags)
{
	_idt[index].target_offset_low = target & 0xffff;
	_idt[index].target_selector = target_selector;
	_idt[index].ist = 0;
	_idt[index].flags = flags;
	_idt[index].target_offset_mid = (target >> 16) & 0xffff;
	_idt[index].target_offset_high = (target >> 32) & 0xffffffff;
}

const char *exception_messages[] = {
	"Division Error",
	"Debug",
	"Non-maskable Interrupt",
	"Breakpoint",
	"Overflow",
	"Bound Range Exceeded",
	"Invalid Opcode",
	"Device Not Available",
	"Double Fault",
	"Reserved - 0x9",
	"Invalid TSS",
	"Segment Not Present",
	"Stack-Segment Fault",
	"General Protection Fault",
	"Page Fault",
	"Reserved - 0xf",
	"x87 Floating-Point",
	"Alignment Check",
	"Machine Check",
	"SIMD Floating-Point",
	"Virtualization",
	"Control Protection",
	"Reserved  - 0x16",
	"Reserved  - 0x17",
	"Reserved  - 0x18",
	"Reserved  - 0x19",
	"Reserved  - 0x1a",
	"Reserved  - 0x1b",
	"Hypervisor Injection",
	"VMM Communication",
	"Security",
	"Reserved - 0x1f",
};

void isr_exception_handler(struct INTERRUPT_STACK *stack)
{
	printf(KPANIC "%s Exception (%#x)", exception_messages[stack->vector], stack->vector);

	if (stack->error_code != 0)
	{
		printf(" | Error Code: %#016x\n", stack->error_code);
	}
	else
	{
		printf("\n");
	}

	strace(10);

	printf("RAX=%p  RBX=%p  RCX=%p  RDX=%p\n", stack->rax, stack->rbx, stack->rcx, stack->rdx);
	printf("RSI=%p  RDI=%p  RBP=%p  RSP=%p\n", stack->rsi, stack->rdi, stack->rbp, stack->return_rsp);
	printf("R8 =%p  R9 =%p  R10=%p  R11=%p\n", stack->r8, stack->r9, stack->r10, stack->r11);
	printf("R12=%p  R13=%p  R14=%p  R15=%p\n", stack->r12, stack->r13, stack->r14, stack->r15);
	printf("RIP=%p  RFL=%p\n", stack->return_rip, stack->return_rflags);
	printf("CR0=%p  CR2=%p  CR3=%p  CR4=%p\n", stack->cr0, stack->cr2, stack->cr3, stack->cr4);
	printf("DR0=%p  DR1=%p  DR2=%p  DR3=%p\n", stack->dr0, stack->dr1, stack->dr2, stack->dr3);
	printf("DR6=%p  DR7=%p\n", stack->dr6, stack->dr7);

	abort("System will now halt\n");
}
