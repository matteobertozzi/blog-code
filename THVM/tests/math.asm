; Simple math example

    mov r0, 0
    mov r1, 2

; Test Add
    add r0, 2           ; r0 = r0 + 2
    add r0, r1          ; r0 = r0 + r1 (2 + 2)
    print $r0

; Test Subtraction
    sub r0, -1          ; r0 = r0 - (-1)
    sub r0, r1          ; r0 = r0 - r1 (3 - 2)
    print $r0

; Test Multiplication
    mul r0, 2           ; r0 = r0 * 2
    mul r0, r1          ; r0 = r0 * r1
    print $r0

; Test Division
    div r0, 2           ; r0 = r0 / 2
    div r0, r1          ; r0 = r0 / r1
    print $r0

; Test Module
    mod r0, 2           ; r0 = r0 % 2
    print $r0

; Test Shift
    mov r0, 1
    shl r0, r1           ; r0 = r0 << r1
    print $r0

    shr r0, r1           ; r0 = r0 >> r1
    print $r1
