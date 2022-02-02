.intel_syntax noprefix
.globl main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 208
    mov rax, rbp
    sub rax, 8
    push rax
    push 10
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
    push 1
    pop rdi
    pop rax
    cmp rax, rdi
    sete al
    movzb rax, al
    push rax
    pop rax
    cmp rax, 0
    je  .Lelse0
    push 5
   jmp .Lend0
.Lelse0:
    push 0
.Lend0:
    pop rax
    mov rsp, rbp
    pop rbp
    ret
