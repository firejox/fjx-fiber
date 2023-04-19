.section .text
.globl fiber_switch_impl
.hidden fiber_switch_impl
fiber_switch_impl:
    pushq   %rbp
    pushq   %rax
    pushq   %rbx
    pushq   %r12
    pushq   %r13
    pushq   %r14
    pushq   %r15
    call .L_switch_resume
    popq    %r15
    popq    %r14
    popq    %r13
    popq    %r12
    popq    %rbx
    popq    %rax
    popq    %rbp
    retq
.L_switch_resume:
    movq    (%rdi), %rax
    movq    %rsp, (%rdi)
    movq    %rax, %rsp
    retq

.globl fiber_load_unaryop
.hidden fiber_load_unaryop
fiber_load_unaryop:
    movq    (%rsp), %rax
    movq    8(%rsp), %rdi
    callq   *%rax
    addq    $24, %rsp
    retq

.globl fiber_load_entrance
.hidden fiber_load_entrance
fiber_load_entrance:
    movq    (%rsp), %rax
    movq    8(%rsp), %rdi
    callq   *%rax
    movq    16(%rsp), %rax
    movq    24(%rsp), %rdi
    callq   *%rax
    addq    $32, %rsp
    retq
