.intel_syntax noprefix
.globl main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 208
    mov rax, rbp
    sub rax, 8
    push rax
    push 4
    pop rdi
    pop rax
    mov [rax], rdi
    push rdi
    pop rax
    mov rax, rbp
    sub rax, 16
    push rax
    push 4
    pop rdi
    pop rax
    mov [rax], rdi
    push rdi
    pop rax
    mov rax, rbp
    sub rax, 8
    push rax
    pop rax
    mov rax, [rax]
    push rax
    mov rax, rbp
    sub rax, 16
    push rax
    pop rax
    mov rax, [rax]
    push rax
    pop rdi
    pop rax
    cmp rax, rdi
    sete al
    movzb rax, al
    push rax
    pop rax
    mov rsp, rbp
    pop rbp
    ret
