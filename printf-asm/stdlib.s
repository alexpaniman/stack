; ==> syscalls numbers:
system_write = 1
system_exit  = 60

; ==> file descriptors:
console_out  = 1

macro .cdeclify {
    pop  rax			; Save ret address
    push r9 r8 rcx rdx rsi rdi
    push rax			; Restore ret address
}

macro .uncdeclify {
    pop rax			; Save ret address
    add rsp, 6 * 0x8

    push rax			; Restore ret address
}

macro .puts label, buffer_size {
    mov rax, system_write
    mov rdi, console_out
    lea rsi, [label]
    mov rdx, buffer_size
    syscall
}
