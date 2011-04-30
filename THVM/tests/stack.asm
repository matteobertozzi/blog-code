; Test push/pop opcode

    mov r0, 10
    mov r1, 25
    push                ; Save Register Stack

    print Pushed, $r0, $r1
    mov r0, 20
    mov r1, 26
    pop pc               ; Back to the old stack with current pc

    print Popped, $r0, $r1

    inc r1
    push

    print Pushed, $r0, $r1
    mov r0, 20
    add r0, r1
    pop pc, r0

    print Popped, $r0, $r1
