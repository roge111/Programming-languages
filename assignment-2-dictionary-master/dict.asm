section .text

extern string_equals
extern string_length
global find_word

find_word:
    .loop:
        push rdi
        push rsi
        add rsi, 8
        call string_equals
        pop rsi
        pop rdi
        
        cmp rax, 1
        je .true

        mov rsi, [rsi]
        test rsi, rsi
        jnz .loop

    .false:
        xor rax, rax
        ret
    .true:
        mov rax, rsi
        ret