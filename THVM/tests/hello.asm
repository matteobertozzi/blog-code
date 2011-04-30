; This is the first THASM Hello World

    mov rx, 10

loop:
    print Hello World, $rx

    dec rx
    jnz rx, loop

loop2:
    print Going Up, $rx

    inc rx
    jne rx, 5, loop2
