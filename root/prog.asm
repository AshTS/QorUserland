.section .data
.align 4
newln:
 .bytes 10

space:
spaces:
 .bytes "                "

numbers:
 .bytes "0123456789ABCDEF"

header:
 .bytes "       0  1  2  3  4  5  6  7   8  9  A  B  C  D  E  F" 10 0

.section .text
.align 16

main:
 push ra
 push s0
 push s1
 push s2
 push s3
 push s4

 li s4, 1
 slli s4, s4, 12

 // Fail if anything but two arguments have been passed
 addi s0, a0, -2
 li s3, 1
 bne s0, zero, main_fail
 
 // Open the file
 addi a1, a1, 8
 ld a0, a1, 0
 li a1, 1
 li a7, 2
 ecall
 li s3, 2
 blt a0, zero, main_fail
 add s0, a0, zero


 // Add 1024 to the stack pointer to add the buffer

 sub sp, sp, s4

  // Copy the current stack pointer
 add s1, sp, zero


 // Read to the buffer
 add a0, s0, zero
 add a1, s1, zero
 add a2, s4, zero
 addi a2, a2, -1
 li a7, 0
 ecall

 add s2, a0, zero

 // Close File
 add a0, s0, zero
 li a7, 3
 ecall

 
 add a1, s2, zero
 add a0, s1, zero
 call hex_dump

 add sp, sp, s4

 li a0, 0
 pop s4
 pop s3
 pop s2
 pop s1
 pop s0
 pop ra
 ret
main_fail:
 add a0, s3, zero
 pop s4
 pop s3
 pop s2
 pop s1
 pop s0
 pop ra
 ret

_start:
 call main
 li a7, 60
 ecall

// write_str(char*) -> ()
// Prints the string at the address passed
write_str:
 push ra
 push s0
 push s1
 add s0, a0, zero
 li s1, 0
write_loop:
 lb a0, s0, 0
 beq a0, zero, write_loop_exit
 addi s1, s1, 1
 addi s0, s0, 1
 j write_loop
write_loop_exit:
 sub s0, s0, s1
 li a7, 1
 li a0, 1

 add a1, s0, zero
 add a2, s1, zero
 ecall

 pop s1
 pop s0
 pop ra
 ret

// write_hex(char) -> ()
//   Prints the hexadecimal representation of the number passed
write_hex:
 push ra
 push s0

 andi s0, a0, 255

 srli a0, s0, 4
 andi a0, a0, 15
 la a1, numbers
 add a1, a1, a0
 
 li a7, 1
 li a0, 1
 li a2, 1
 ecall

 andi a0, s0, 15
 la a1, numbers
 add a1, a1, a0
 li a7, 1
 li a0, 1
 li a2, 1
 ecall

 pop s0
 pop ra
 ret 

write_space:
 li a7, 1
 li a0, 1
 la a1, space
 li a2, 1
 ecall
 ret

write_newline:
 li a7, 1
 li a0, 1
 la a1, newln
 li a2, 1
 ecall
 ret

// void hex_dump(void* buffer, size_t length)
hex_dump:
 push ra
 push s0
 push s1
 push s2
 push s3
 push s4

 add s0, a0, zero
 add s1, a1, zero

// {
//     write_str("       0  1  2  3  4  5  6  7   8  9  A  B  C  D  E  F\n");
 la a0, header
 call write_str
//     
//     size_t row = 0;
 li s2, 0
// 
hex_dump_L0:
//     while (row * 16 < length)
//     {
 slli a0, s2, 4
 bge a0, s1, hex_dump_L1

//         write_space();
 call write_space
//         write_hex(row >> 8);
 slli a0, s2, 8
 call write_hex
//         write_hex(row);
 add a0, s2, zero
 call write_hex
//         write_space();
 call write_space
// 
//         size_t i = 0;
 li s3, 0
// 
hex_dump_L2:
//         while (i < 16)
 li a0, 16
 bge s3, a0, hex_dump_L3

//         {
//             char* this = buffer + row * 16 + i;
 slli s4, s2, 4
 add s4, s4, s0
 add s4, s4, s3

 

//             write_hex(*this);
 lb a0, s4, 0
 call write_hex
// 
//             write_space();

 call write_space
//             
 li a0, 7
 bne s3, a0, hex_dump_L4
//             if (i == 7)
//             {
//                 write_space();
 call write_space
//             }

hex_dump_L4:
// 
//             i++;
 addi s3, s3, 1
//         }
 j hex_dump_L2

 li s3, 0

hex_dump_L3:

// 
//         write_newline();

 call write_newline
// 
//         row++;
 addi s2, s2, 1
//     }
 j hex_dump_L0
hex_dump_L1:
// }
 pop s4
 pop s3
 pop s2
 pop s1
 pop s0
 pop ra
 ret