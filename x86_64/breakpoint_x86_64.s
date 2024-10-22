global start

section .text

global _push_registers_x86_64
global _push_registers_x86_64_end

global _set_argument_x86_64
global _set_argument_x86_64_end

global _check_breakpoint_x86_64
global _check_breakpoint_x86_64_end

global _breakpoint_x86_64
global _breakpoint_x86_64_end

global _pop_registers_x86_64
global _pop_registers_x86_64_end

_push_registers_x86_64:
	push rsp
	push rbp
	push rax
	push rbx
	push rcx
	push rdx
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
_push_registers_x86_64_end:
	nop
_set_argument_x86_64:
	lea rdi, [rsp + 0x80]
_set_argument_x86_64_end:
	nop
_check_breakpoint_x86_64:
	cmp rax, 1
	jne short $+4h
_check_breakpoint_x86_64_end:
	nop
_breakpoint_x86_64:
	int3
_breakpoint_x86_64_end:
	nop
_pop_registers_x86_64:
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rdx
	pop rcx
	pop rbx
	pop rax
	pop rbp
	pop rsp
_pop_registers_x86_64_end:
	nop