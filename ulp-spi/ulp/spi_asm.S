.data
    .global init_str_len
init_str_len:
.long 0x1e
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
    .global init_partial_str
init_partial_str:
.long 0x10
.long 0x18
.long 0x18
.long 0x8
.long 0x18
.long 0x18
.long 0x8
.long 0x0
.long 0x0
.long 0x0
.long 0x0
.long 0x0
.long 0x0
.long 0x0
.long 0x0
.long 0x0
.long 0x0
.long 0x0
.long 0x0
.long 0x0
.long 0x13
.long 0x14
.long 0x44
.long 0x12
.long 0x0
.long 0x0
.long 0x0
.long 0x0
.long 0x0
.long 0x0
    .global entry
.text
.bss
entry.loopcnt:
    .space 4
.text
.bss
entry.datasize:
    .space 4
.text
.bss
entry.mydatabyte:
    .space 4
.text
.bss
entry.bitsSent:
    .space 4
.text
.bss
entry.thisbit:
    .space 4
.text
entry:
move r2,0
move r3,entry.loopcnt
st r2,r3,0
move r2,0
move r3,entry.datasize
st r2,r3,0
reg_wr 257,30,30,1
wait 0xffff
reg_wr 258,30,30,1
wait 0xffff
L.2:
reg_wr 258,21,21,1
reg_wr 258,26,26,1
reg_wr 258,31,31,1
move r2,0
move r3,entry.loopcnt
st r2,r3,0
move r2,init_str_len
ld r2,r2,0
move r3,entry.datasize
st r2,r3,0
jump L.4
L.3:
move r2,entry.loopcnt
ld r2,r2,0
move r1,swap
add r2,r2,r1
ld r2,r2,0
move r3,entry.mydatabyte
st r2,r3,0
move r2,8
move r3,entry.bitsSent
st r2,r3,0
move r2,entry.loopcnt
ld r2,r2,0
add r2,r2,1
move r3,entry.loopcnt
st r2,r3,0
jump L.7
L.6:
move r2,1
move r1,entry.bitsSent
ld r1,r1,0
add r1,r1,1
move r3,entry.bitsSent
st r1,r3,0
move r1,entry.mydatabyte
ld r1,r1,0
and r0,r1,128
move r3,entry.thisbit
st r0,r3,0
lsh r1,r1,1
move r3,entry.mydatabyte
st r1,r3,0
move r1,entry.thisbit
ld r1,r1,0
sub r2,r2,r1 #{ if r1 >= r2 goto L.9
add r2,r2,r1
jump L.9, eq
jump L.9, ov #}
reg_wr 258,21,21,1
jump L.10
L.9:
reg_wr 257,21,21,1
L.10:
reg_wr 258,26,26,1
wait 255
reg_wr 257,26,26,1
wait 255
L.7:
move r2,entry.bitsSent
ld r2,r2,0
move r1,8
sub r2,r2,r1 #{ if r2 < r1 goto L.6
add r2,r2,r1
jump L.6, ov #}
L.4:
move r2,entry.loopcnt
ld r2,r2,0
move r1,entry.datasize
ld r1,r1,0
sub r2,r2,r1 #{ if r2 < r1 goto L.3
add r2,r2,r1
jump L.3, ov #}
reg_wr 258,21,21,1
reg_wr 257,31,31,1
L.1:

    .global end_program
end_program:
L.11:

.bss
    .global swap
swap:
    .space 120
.text
halt
