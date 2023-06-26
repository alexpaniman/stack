format ELF64

section '.text'

include "stdlib.s"	

;; void stringify_integer(char* output, int number, int base) 
public stringify_integer
stringify_integer:	
    	push rbp
    	mov rbp, rsp

    	push rbx
    	
    	mov eax, esi
    	mov ebx, edx

    	xor ecx, ecx

    .count_length:
    	xor edx, edx
    	div ebx

    	inc ecx
    	
    	cmp eax, 0x0
    	jne .count_length

    	mov eax, esi
    	add rdi, rcx

    	mov byte [rdi], 0
    	mov rsi, rdi

    .next_symbol:
    	xor edx, edx
    	div ebx

    	mov cl, [number_table + edx]

    	dec rdi
    	mov byte [rdi], cl

    	cmp eax, 0x0
    	jnz .next_symbol
    	
    	pop rbx
    	pop rbp
    	ret

section '.data'
    number_table:	
    	db '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ'

    max_base = $ - number_table
    	

section '.text'

public io_buffer_flush
io_buffer_flush:
    	.puts buffer_for_io, rcx
    	xor rcx, rcx
        ret

public io_reserve_space
io_reserve_space:
    	lea rax, [rcx + rbx]

	cmp rax, buffer_capacity
    	jb .finalize

    	push rsi rdi rdx
	call io_buffer_flush
    	pop  rdx rdi rsi
    	
    .finalize:
    	ret

io_print_char:
	;; Make sure, there's enough space for symbol
    	mov rbx, 1		

    	push rax
    	call io_reserve_space
    	pop  rax

    	mov [buffer_for_io + rcx], al
    	inc rcx

    	ret

string_length:	
    	xor rcx, rcx

    .next_symbol:
    	lodsb

    	inc rcx

    	cmp al, 0x0
    	jne .next_symbol

    	dec rcx

    	ret

string_copy:
    .write_string:	
	movsb

	cmp byte [rsi - 1], 0x0
	jne .write_string

	ret

public asm_printf    
asm_printf:
	.cdeclify

	push rbp
	mov rbp, rsp

    	xor rcx, rcx

    	mov rsi, qword [rbp + 16]
    	lea r10, qword [rbp + 24]

    	xor rax, rax

    	push rsi
    .next_symbol:
    	pop  rsi

    	lodsb
    	push rsi

    	cmp al, '%'
    	je .process_specifier

    	cmp al, 0x0
    	je .finalize

    	call io_print_char
    	jmp .next_symbol

    .process_specifier:
    	xor rax, rax

    	pop  rsi
    	lodsb 

    	push rsi

    	cmp al, '%'
    	je .process_percent

    	mov dl, al
    	sub al, 'a'

    	cmp al, jmp_table_size
    	ja .process_invalid

    	jmp qword [printf_jmp_table + 8 * eax]

    .process_percent:
    	mov eax, '%'
    	call io_print_char

    	jmp .next_symbol

    .process_binary:
    	mov rdx, 2
    	jmp .print_number

    .process_octal:
    	mov rdx, 8
    	jmp .print_number

    .process_decimal:
    	mov rdx, 10
    	jmp .print_number

    .process_hex:
    	mov rdx, 16

    .print_number:
    	mov rsi, [r10]
    	add r10, 8

	;; 64 bits + 0-terminator
    	mov rbx, 65
    	call io_reserve_space

    	lea rdi, [buffer_for_io + rcx]

    	push rcx
    	call stringify_integer
    	pop  rcx

    	sub rsi, rdi 
    	add rcx, rsi

    	jmp .next_symbol

    .process_char:
    	mov rax, [r10]
    	add r10, 8

    	call io_print_char
    	jmp .next_symbol

    .process_string:
    	mov rsi, [r10]

    	mov rbx, rcx
    	call string_length

    	cmp rcx, buffer_capacity
    	ja .buffer_too_small

    	lea rdi, [buffer_for_io + rbx]
    	mov rsi, [r10]

    	call string_copy
    	add rcx, rbx

    	add r10, 8
    	jmp .next_symbol

    .buffer_too_small:
	push rcx
    	call io_buffer_flush
    	pop  rcx

    	lea rsi, [r10]

    	.puts rsi, rbx
    	xor rcx, rcx

    	add r10, 8
    	jmp .next_symbol

    .process_invalid:
    	mov al, '%'
    	call io_print_char

	mov al, dl
    	call io_print_char

    	jmp .next_symbol

    .finalize:
    	call io_buffer_flush

	leave

	.uncdeclify
	ret

section '.data'

buffer_capacity = 256	
buffer_for_io:
    times buffer_capacity db 0

printf_jmp_table:   
    ;; For letter: a      [total:  1]
    times 1  dq asm_printf.process_invalid

    ;; ==> Specifiers: %b, %c, %d
    dq asm_printf.process_binary
    dq asm_printf.process_char
    dq asm_printf.process_decimal

    ;; For letters e -- n [total: 10]
    times 10 dq asm_printf.process_invalid

    ;; ==> Specifier: %o
    dq asm_printf.process_octal

    ;; For letters p -- r [total:  3]
    times 3  dq asm_printf.process_invalid

    ;; ==> Specifier: %s
    dq asm_printf.process_string

    ;; For letters t -- w [total:  4]
    times 4  dq asm_printf.process_invalid

    ;; ==> Specifier: %x
    dq asm_printf.process_hex

    ;; For letters y -- z [total:  2]
    times 2  dq asm_printf.process_invalid

jmp_table_size = $ - printf_jmp_table
