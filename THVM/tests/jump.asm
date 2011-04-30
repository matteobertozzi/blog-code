; Test jump!

    jmp run_app
    print 0 Unreachable code

topper_jumper:
    print Jump on Top
    jmp quit_app

run_app:
    print Run App

    mov r0, 1
    jnz r0, jump_nz
    print 1 Unreachable code

jump_nz:
    print NZ
    dec r0
    jz r0, jump_z
    print 2 Unreachable code

jump_z:
    print Z
    jlt r0, 2, jump_lt
    print 3 Unreachable code

jump_lt:
    print LT
    add r0, 10
    jgt r0, 2, jump_gt
    print 4 Unreachable code

jump_gt:
    print GT
    je r0, 10, jump_eq
    print 5 Unreachable code

jump_eq:
    print EQ
    jne r0, 5, jump_neq
    print 6 Unreachable code

jump_neq:
    print NEQ
    jle r0, 10, jump_leq
    print 7 Unreachable code

jump_leq:
    print LEQ
    jge r0, 10, jump_geq
    print 8 Unreachable code

jump_geq:
    print NEQ
    jmp topper_jumper
    print 9 Unreachable code

quit_app:
    print ok, game over
