.DATA
x DW 42
y DB 10
arr DB 1,2,3,4

.CODE
MOV AX,128
MOV BX,1
ALLOC
JZ end
MOV DX,0
MOV [ES:DX],64
MOV CX,[ES:DX]
MOV [ES:CX],128
FREE

JMP start
MOV AX,10

start: MOV AX,[arr]
loop: ADD AX,1
CMP AX,[y]
JNZ loop

MOV AX,999
PUSH AX
POP CX
CMP CX,999
JZ end
ADD CX,10

end: HALT