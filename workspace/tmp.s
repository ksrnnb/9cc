.intel_syntax noprefix
.globl main
main:
    push 5
    push 1
    pop rdi
    pop rax
    sub rax, rdi
    push rax
    pop rax
    ret
