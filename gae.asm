BITS 64

call below
db "Hello, world!", 0x0a

below:
; write
mov rax, 1
mov rdi, 1
; Pop the value from the return address 
; and store it into the register
pop rsi 
mov rdx, 14
syscall

; exit
mov rax, 60
mov rdi, 0
syscall
