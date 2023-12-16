section .text
%define SYS_EXIT, 60; 
%define ONE, 1;простите за мою гениальность в названиях :)
%define TEN, 10;
%define ZERO, 0;

global exit
global string_length
global print_string
global print_error
global print_newline
global print_char
global print_int
global print_uint
global string_equals
global read_char
global read_word
global parse_uint
global parse_int
global string_copy 
 
; Принимает код возврата и завершает текущий процесс
exit: 
    mov rax, SYS_EXIT
    syscall
 
; Принимает указатель на нуль-терминированную строку, возвращает её длину
string_length:
    xor rsi, rsi

    .loop:
        mov al, [rdi + rsi]

        test al, al
        je .end

        inc rsi

        jmp .loop
    .end:
        mov rax, rsi
        ret

; Принимает указатель на нуль-терминированную строку, выводит её в stdout
print_string:
    call string_length
    mov rsi, rdi
    mov rdi, ONE
    mov rdx, rax
    mov rax, ONE

    syscall

    ret

; Принимает код символа и выводит его в stdout
print_char:
    push rdi

    mov rax, ONE
    mov rsi, rsp
    mov rdi, ONE
    mov rdx, ONE

    syscall

    pop rdi
    ret

; Переводит строку (выводит символ с кодом 0xA)
print_newline:
    mov rdi, TEN
    jmp print_char

; Выводит беззнаковое 8-байтовое число в десятичном формате 
; Совет: выделите место в стеке и храните там результаты деления
; Не забудьте перевести цифры в их ASCII коды.
print_uint:
    mov r9, rsp
    mov rsi, TEN
    dec rsp
    mov byte[rsp], ZERO

    mov rax, rdi

    .loop:
        xor rdx, rdx
        div rsi

        dec rsp
        add rdx, 0x30 ;GET ASCII code of number
        mov [rsp], dl

        test rax, rax
        jnz .loop

    mov rdi, rsp
    call print_string
    mov rsp, r9
    ret

; Выводит знаковое 8-байтовое число в десятичном формате 
print_int:
    cmp rdi, ZERO
    jns .unsigned

    push rdi

    mov rdi, 0x2D ;"-" ASCII
    call print_char
    
    pop rdi

    neg rdi
    jmp print_int
 
    .unsigned:
        jmp print_uint

; Принимает два указателя на нуль-терминированные строки, возвращает 1 если они равны, 0 иначе
string_equals:
    xor rax, rax
    xor r10, r10 ;counter
    .loop:
        mov al, [rdi + r10]
        cmp al, [rsi + r10] 
        jne .false 
        cmp rax, 0x0
        je .true
        inc r10 
        jmp .loop
    .false:
        mov rax, ZERO
        ret
    .true:
        mov rax, ONE
        ret

; Читает один символ из stdin и возвращает его. Возвращает 0 если достигнут конец потока
read_char:
    mov rax, ZERO
    xor rdi, rdi
    mov rdx, ONE
    push rax
    mov rsi, rsp

    syscall

    pop rax

    ret

; Принимает: адрес начала буфера, размер буфера
; Читает в буфер слово из stdin, пропуская пробельные символы в начале, .
; Пробельные символы это пробел 0x20, табуляция 0x9 и перевод строки 0xA.
; Останавливается и возвращает 0 если слово слишком большое для буфера
; При успехе возвращает адрес буфера в rax, длину слова в rdx.
; При неудаче возвращает 0 в rax
; Эта функция должна дописывать к слову нуль-терминатор

read_word:
    mov r8, rdi
    mov r9, rsi
    xor r10, r10 ;counter

    .skip_first_symbols:
        call read_char

        cmp rax, 0x20
        je .skip_first_symbols

        cmp rax, 0x09
        je .skip_first_symbols

        cmp rax, 0xa
        je .skip_first_symbols

    .loop:

        cmp rax, 0x09
        je .end

        cmp rax, 0xa
        je .end

        cmp rax, 0x0
        je .end

        cmp r9, r10
        je .out_of_buffer

        mov byte[r8 + r10], al

        inc r10
        
        call read_char
        jmp .loop

    .end:
        xor al, al
        mov byte[r8 + r10], al
        mov rax, r8
        mov rdx, r10
        ret
    .out_of_buffer:
        mov rax, ZERO
        ret
 

; Принимает указатель на строку, пытается
; прочитать из её начала беззнаковое число.
; Возвращает в rax: число, rdx : его длину в символах
; rdx = 0 если число прочитать не удалось
parse_uint:
    xor r10, r10 ;counter
    xor rdx, rdx
    xor rax, rax
    mov r8, TEN
    .loop:
        mov dl, byte[rdi + r10]
        cmp rdx, 0x30
        jl .end
        cmp rdx, 0x39
        jg .end
        sub rdx, 0x30 ;transform ASCII to human
        push rdx 
        mul r8 ;this command reset rdx
        pop rdx
        add rax, rdx
        inc r10
        jmp .loop
    .end:
        mov rdx, r10
        ret




; Принимает указатель на строку, пытается
; прочитать из её начала знаковое число.
; Если есть знак, пробелы между ним и числом не разрешены.
; Возвращает в rax: число, rdx : его длину в символах (включая знак, если он был) 
; rdx = 0 если число прочитать не удалось
parse_int:
    cmp byte[rdi], 0x2D
    jnz .unsigned
    inc rdi
    call parse_uint
    neg rax
    inc rdx
    ret
    .unsigned:
        jmp parse_uint
        

; Принимает указатель на строку, указатель на буфер и длину буфера
; Копирует строку в буфер
; Возвращает длину строки если она умещается в буфер, иначе 0
string_copy:
    xor r10, r10 ;counter
    push rsi
    call string_length
    pop rsi
    mov r8, rax ;length
    cmp r8, rdx
    jge .out_of_buffer
    xor rax, rax
    .loop:
        mov al, byte[rdi + r10]
        mov byte[rsi + r10], al
        inc r10
        cmp al, ZERO
        jne .loop
        ; cmp r8, r10
        ; jg .loop
        mov rax, r10
        ret

    .out_of_buffer:
        mov rax, 0
        ret
