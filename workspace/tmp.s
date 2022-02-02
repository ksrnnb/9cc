.intel_syntax noprefix
.globl main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 208
    push 1
    push 2
    pop rdi
    pop rax
    cmp rax, rdi
    setle al
    movzb rax, al
    push rax
    pop rax
    mov rsp, rbp
    pop rbp
    ret
