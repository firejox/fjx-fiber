.code32
.section .text
.globl  fiber_switch_impl
.hidden fiber_switch_impl
fiber_switch_impl:
    pushl    %ebp
    movl     %esp, %ebp
    pushl    %eax
    pushl    %esi
    pushl    %edi
    pushfl
    movl     8(%ebp), %ecx
    call    .L_switch_resume
    popfl
    popl     %edi
    popl     %esi
    popl     %eax
    popl     %ebp
    ret
.L_switch_resume:
    movl     (%ecx), %eax
    movl     %esp, (%ecx)
    movl     %eax, %esp
    ret

.globl  fiber_load_unaryop
.hidden fiber_load_unaryop
fiber_load_unaryop:
    movl     (%esp), %eax
    movl     4(%esp), %ecx
    movl     %ecx, (%esp)
    call     *%eax
    addl     $12, %esp
    ret

.globl  fiber_load_entrace
.hidden fiber_load_entrace
fiber_load_entrance:
    movl     (%esp), %eax
    movl     4(%esp), %ecx
    movl     %ecx, (%esp)
    call     *%eax
    movl     8(%esp), %eax
    movl     12(%esp), %ecx
    movl     %ecx, (%esp)
    call     *%eax
    addl     $16, %esp
    ret
