.DATA
x DW 42
y DB 10
arr DB 1,2,3,4

.CODE
start: MOV AX, x
loop: ADD AX, y
JMP loop

