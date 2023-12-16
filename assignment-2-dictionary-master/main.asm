%include "lib.inc"
%include "words.inc"
%define buf_size 256
%define ZERO, 0
%define ONE, 1
%define TWO, 2
%define EIGTH, 8
global _start

section .data

    overflow_message: db "Overflow error", 10, 0
    not_found_message: db "Key is not found", 10, 0

section .text

extern find_word

_start:
    sub rsp, buf_size
	mov rdi, rsp
    mov rsi, buf_size
	call read_word
	test rax, rax
	jz .overflow
    

    mov rdi, rsp
    mov rsi, NEXT_ELEMENT
    call find_word
    test rax, rax
    jz .not_found

    mov rdi, rax
	add rdi, EIGHT
	push rdi
	call string_length
	pop rdi
	add rax, rdi
	inc rax

	mov rdi, rax
	call print_string
    call print_newline
    
    xor rdi, rdi
    jmp exit

    .overflow:
        mov rdi, overflow_message
        jmp .print_error
    .not_found:
        mov rdi, not_found_message
    .print_error:
        call print_error_string
        mov rdi, ONE
        jmp exit

print_error_string:
    push rdi
    call string_length
    pop rdi
    mov rsi, rdi
    mov rdi, TWO
    mov rdx, rax
    mov rax, ONE

    syscall

    ret
