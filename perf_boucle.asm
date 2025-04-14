.DATA
pas db 1
max db 10000
.CODE
MOV AX,0
loop: ADD AX,[pas]
PUSH AX
POP CX
CMP AX,[max]
JNZ loop
HALT