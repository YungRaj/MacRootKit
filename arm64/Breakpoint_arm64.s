global start

section .text

#define LOAD_IMMEDIATE_32(reg, val)                \
	mov  reg, #(((val) >> 0x00) & 0xFFFF);         \
	movk reg, #(((val) >> 0x10) & 0xFFFF), lsl#16

#define LOAD_IMMEDIATE_64(reg, val)                \
	mov  reg, #(((val) >> 0x00) & 0xFFFF);         \
	movk reg, #(((val) >> 0x10) & 0xFFFF), lsl#16; \
	movk reg, #(((val) >> 0x20) & 0xFFFF), lsl#32; \
	movk reg, #(((val) >> 0x30) & 0xFFFF), lsl#48

#define LOAD_FROM_LABEL(reg, label) \
	adr reg, label;                 \
	ldr reg, [reg]

global _push_registers_arm64
global _push_registers_arm64_end

global _set_argument_arm64
global _set_argument_arm64_end

global _check_breakpoint_arm64
global _check_breakpoint_arm64_end

global _breakpoint_arm64
global _breakpoint_arm64_end

global _pop_register_arm64
global _pop_registers_arm64_end

_push_registers_arm64:
	sub sp, sp, 0x110
	stp x0, x1, [sp, 0x100]
	stp x2, x3, [sp, 0xF0]
	stp x4, x5, [sp, 0xE0]
	stp x6, x7, [sp, 0xD0]
	stp x8, x9, [sp, 0xC0]
	stp x10, x11, [sp, 0xB0]
	stp x12, x13, [sp, 0xA0]
	stp x14, x15, [sp, 0x90]
	stp x16, x17, [sp, 0x80]
	stp x18, x19, [sp, 0x70]
	stp x20, x21, [sp, 0x60]
	stp x22, x23, [sp, 0x50]
	stp x24, x25, [sp, 0x40]
	stp x26, x27, [sp, 0x30]
	stp x28, x29, [sp, 0x20]
	stp x30, sp, [sp, 0x10]
_push_registers_arm64_end:
	nop
_set_argument_arm64:
	lea rdi, [rsp + 0x80]
_set_argument_arm64_end:
	nop
_check_breakpoint_arm64:
	cmp rax, 1
	b.ne 0x4
_check_breakpoint_arm64_end:
	nop
_breakpoint_arm64:
	brk #0
_breakpoint_arm64_end:
	nop
_pop_registers_arm64:
	ldp x30, sp, [sp, 0x10]
	ldp x28, x29, [sp, 0x20]
	ldp x26, x27, [sp, 0x30]
	ldp x24, x25, [sp, 0x40]
	ldp x22, x23, [sp, 0x50]
	ldp x20, x21, [sp, 0x60]
	ldp x18, x19, [sp, 0x70]
	ldp x16, x17, [sp, 0x80]
	ldp x14, x15, [sp, 0x90]
	ldp x12, x13, [sp, 0xA0]
	ldp x10, x11, [sp, 0xB0]
	ldp x8, x9, [sp, 0xC0]
	ldp x6, x7, [sp, 0xD0]
	ldp x4, x5, [sp, 0xE0]
	ldp x2, x3, [sp, 0xF0]
	ldp x0, x1, [sp, 0x100]
	add sp, sp, 0x110
_pop_registers_arm64_end:
	nop