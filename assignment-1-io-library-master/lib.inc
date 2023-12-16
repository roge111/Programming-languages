section .text
 
 
; Принимает код возврата и завершает текущий процесс
exit: 
    mov rax, 60
    syscall

; Принимает указатель на нуль-терминированную строку, возвращает её длину
string_length:
    xor rax, rax
.loop:
    cmp byte [rdi + rax], 0
    je  .end
    inc rax
    jmp .loop
.end:
    ret

; Принимает указатель на нуль-терминированную строку, выводит её в stdout
print_string:
    push rdi
    call string_length
    pop rsi
    mov rdi, 1
    mov rdx, rax
    mov rax, 1
    syscall
    ret

; Принимает код символа и выводит его в stdout
print_char:
    push rdi
    mov rdi, rsp
    call print_string
    pop rdi
    ret

; Переводит строку (выводит символ с кодом 0xA)
print_newline:
    mov rdi, 10
    call print_char
    ret

; Выводит беззнаковое 8-байтовое число в десятичном формате 
; Совет: выделите место в стеке и храните там результаты деления
; Не забудьте перевести цифры в их ASCII коды.
print_uint:
    push rbp
    push rbx
    sub rsp, 64
    mov rbp, rsp

    mov rcx, 63
    mov byte [rbp + rcx], 0
    mov rax, rdi
    mov rbx, 10

.loop1:
    mov rdx, 0
    div rbx
    add rdx, '0'
    dec rcx
    mov byte [rbp + rcx], dl
    cmp rax, 0
    jnz .loop1

    mov rdi, rbp
    add rdi, rcx
    call print_string

    add rsp, 64
    pop rbx
    pop rbp
    ret

; Выводит знаковое 8-байтовое число в десятичном формате 
print_int:
    cmp rdi, 0
    jge .positive
    push    rdi
    mov rdi, '-'
    call print_char
    pop rdi
    neg rdi
.positive:
    call    print_uint
    ret

; Принимает два указателя на нуль-терминированные строки, возвращает 1 если они равны, 0 иначе
string_equals:
    xor rax, rax
.loop:
    mov cl, [rdi + rax]
    cmp cl, byte [rsi + rax]
    jne .nonequal
    cmp cl, 0
    je  .equal
    inc rax
    jmp .loop
.equal:
    mov rax, 1
    ret
.nonequal:
    mov rax, 0
    ret

; Читает один символ из stdin и возвращает его. Возвращает 0 если достигнут конец потока
read_char:
    xor rax, rax
    push rax
    mov rax, 0
    mov rdi, 0
    mov rsi, rsp
    mov rdx, 1
    syscall
    cmp rax, 1
    je  .ok
    pop rax
    xor rax, rax
    ret
.ok:
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
    push rbp
    push rbx
    mov rbp, rdi
    mov rbx, rsi

    cmp rbx, 2
    jl  .bad_end
    dec rbx
    
.loop_sp:
    call read_char
    cmp rax, ' '
    je .loop_sp
    cmp rax, 20h
    je .loop_sp
    cmp rax, 0Ah
    je .loop_sp
    cmp rax, 9h
    je .loop_sp
     cmp rax, 0
    je .bad_end

    ; non-spacial symbol!
    xor rcx, rcx
.loop_word:
    mov byte [rbp + rcx], al

    push rcx
    call read_char
    pop rcx

    cmp rax, ' '
    je .good_end
    cmp rax, 20h
    je .good_end
    cmp rax, 0Ah
    je .good_end
    cmp rax, 9h
    je .loop_sp 
    cmp rax, 0
    je .good_end

    inc rcx
    cmp rcx, rbx  
    je .bad_end   
    jmp .loop_word

.good_end:
    mov rax, rbp
    inc rcx
    mov byte [rbp + rcx], 0
    mov  rdx, rcx
    jmp .clear

.bad_end:
    mov rax, 0
    mov rdx, 0
.clear:
    pop rbx
    pop rbp
    ret
 

; Принимает указатель на строку, пытается
; прочитать из её начала беззнаковое число.
; Возвращает в rax: число, rdx : его длину в символах
; rdx = 0 если число прочитать не удалось
parse_uint:
    push rbx
    push r12
    xor rax, rax
    xor rbx, rbx 
    mov r12, 10
.loop:
    xor rcx, rcx
    mov cl,  byte [rdi + rbx]
    cmp cl, '0'
    jl .stop
    cmp cl, '9'
    jg .stop
    sub rcx, '0'
    mul r12
    add rax, rcx
    inc rbx
    jmp .loop

.stop:
    mov rdx, rbx
    pop r12
    pop rbx
    ret



; Принимает указатель на строку, пытается
; прочитать из её начала знаковое число.
; Если есть знак, пробелы между ним и числом не разрешены.
; Возвращает в rax: число, rdx : его длину в символах (включая знак, если он был) 
; rdx = 0 если число прочитать не удалось
parse_int:
    push rbx
    xor rbx, rbx
    mov cl, byte [rdi]
    cmp cl, '-'
    jne .read_body

    mov rbx, 1
    inc rdi

.read_body:
    call parse_uint

    cmp rdx, 0    ; не получилось
    je .stop
    cmp rbx, 0    ; знака нет, всё в порядке
    je .stop
    
    neg rax
    inc rdx
    
.stop:
    pop rbx
    ret 

; Принимает указатель на строку, указатель на буфер и длину буфера
; Копирует строку в буфер
; Возвращает длину строки если она умещается в буфер, иначе 0
string_copy:
    push rdi
    push rsi
    push rdx
    call string_length
    pop rdx
    pop rsi
    pop rdi

    inc rax
    cmp rax, rdx
    jg .bad_end

    xor rcx, rcx
.copy:
    mov dl, [rdi + rcx]
    mov [rsi + rcx], dl
    inc rcx
    cmp rcx, rax
    jl  .copy

.bad_end:
    mov rax, 0
    ret

