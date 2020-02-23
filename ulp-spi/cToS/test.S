.data
    .global init_str
init_str:
.long 0x2
.long 0x2
.long 0x1
.long 0x11
.long 0x12
.long 0x12
.long 0x22
.long 0x22
.long 0x66
.long 0x69
.long 0x69
.long 0x59
.long 0x58
.long 0x99
.long 0x99
.long 0x88
.long 0x0
.long 0x0
.long 0x0
.long 0x0
.long 0xf8
.long 0xb4
.long 0x13
.long 0x51
.long 0x35
.long 0x51
.long 0x51
.long 0x19
.long 0x1
.long 0x0
    .global entry
.text
.bss
entry.i:
    .space 4
.text
.bss
entry.j:
    .space 4
.text
entry:
move r2,0
move r3,entry.j
st r2,r3,0
move r2,0
move r3,entry.i
st r2,r3,0
jump L.5
L.2:
move r2,entry.i
ld r2,r2,0
move r1,faker
add r1,r2,r1
move r0,init_str
add r2,r2,r0
ld r2,r2,0
st r2,r1,0
L.3:
move r2,entry.i
ld r2,r2,0
add r2,r2,1
move r3,entry.i
st r2,r3,0
L.5:
move r2,entry.i
ld r2,r2,0
move r1,30
sub r2,r2,r1 #{ if r2 < r1 goto L.2
add r2,r2,r1
jump L.2, ov #}
L.1:

.bss
    .global faker
faker:
    .space 120
.text
halt
