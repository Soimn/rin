00007FF7751810EC  int3            
00007FF7751810ED  movzx           ecx, byte ptr [r12]
00007FF7751810F2  movzx           eax, cl
00007FF7751810F5  mov             qword ptr [rbp-0x59], 0x0
00007FF7751810FD  and             al, 0xdf
00007FF7751810FF  mov             qword ptr [rbp-0x51], r14
00007FF775181103  sub             al, 0x41
00007FF775181105  cmp             al, 0x19
00007FF775181107  jbe             0x7ff775182151 <LexFile+0x1101>
00007FF77518110D  cmp             cl, 0x5f
00007FF775181110  jz              0x7ff775182151 <LexFile+0x1101>
00007FF775181116  mov             rcx, r12
00007FF775181119  nop             dword ptr [rax], eax
00007FF775181120  movzx           edx, byte ptr [rcx]
00007FF775181123  lea             eax, ptr [rdx-0x1]
00007FF775181126  cmp             al, 0x20
00007FF775181128  jnb             0x7ff77518113e <LexFile+0xee>
00007FF77518112A  nop             word ptr [rax+rax*1], ax
00007FF775181130  movzx           edx, byte ptr [rcx+0x1]
00007FF775181134  inc             rcx
00007FF775181137  lea             eax, ptr [rdx-0x1]
00007FF77518113A  cmp             al, 0x20
00007FF77518113C  jb              0x7ff775181130 <LexFile+0xe0>
00007FF77518113E  lea             r11, ptr [rcx+0x1]
00007FF775181142  mov             qword ptr [rbp-0x41], r11
00007FF775181146  cmp             dl, 0x2f
00007FF775181149  jnz             0x7ff775181171 <LexFile+0x121>
00007FF77518114B  cmp             byte ptr [r11], dl
00007FF77518114E  jnz             0x7ff775181175 <LexFile+0x125>
00007FF775181150  movzx           eax, byte ptr [rcx+0x2]
00007FF775181154  add             rcx, 0x2
00007FF775181158  test            al, al
00007FF77518115A  jz              0x7ff775181120 <LexFile+0xd0>
00007FF77518115C  nop             dword ptr [rax], eax
00007FF775181160  cmp             al, 0xa
00007FF775181162  jz              0x7ff775181120 <LexFile+0xd0>
00007FF775181164  movzx           eax, byte ptr [rcx+0x1]
00007FF775181168  inc             rcx
00007FF77518116B  test            al, al
00007FF77518116D  jnz             0x7ff775181160 <LexFile+0x110>
00007FF77518116F  jmp             0x7ff775181120 <LexFile+0xd0>
00007FF775181171  mov             qword ptr [rbp-0x41], r11
00007FF775181175  movzx           edx, byte ptr [rcx]
00007FF775181178  cmp             dl, 0x2f
00007FF77518117B  jnz             0x7ff7751811d0 <LexFile+0x180>
00007FF77518117D  cmp             byte ptr [r11], 0x2a
00007FF775181181  jnz             0x7ff7751811d0 <LexFile+0x180>
00007FF775181183  add             rcx, 0x2
00007FF775181187  mov             r8, r10
00007FF77518118A  nop             word ptr [rax+rax*1], ax
00007FF775181190  movzx           edx, byte ptr [rcx]
00007FF775181193  cmp             dl, 0x2f
00007FF775181196  jnz             0x7ff7751811a7 <LexFile+0x157>
00007FF775181198  cmp             byte ptr [rcx+0x1], 0x2a
00007FF77518119C  jnz             0x7ff7751811a7 <LexFile+0x157>
00007FF77518119E  inc             r8
00007FF7751811A1  add             rcx, 0x2
00007FF7751811A5  jmp             0x7ff7751811c6 <LexFile+0x176>
00007FF7751811A7  cmp             dl, 0x2a
00007FF7751811AA  jnz             0x7ff7751811bb <LexFile+0x16b>
00007FF7751811AC  cmp             byte ptr [rcx+0x1], 0x2f
00007FF7751811B0  jnz             0x7ff7751811c3 <LexFile+0x173>
00007FF7751811B2  dec             r8
00007FF7751811B5  add             rcx, 0x2
00007FF7751811B9  jmp             0x7ff7751811c6 <LexFile+0x176>
00007FF7751811BB  test            dl, dl
00007FF7751811BD  jz              0x7ff775181120 <LexFile+0xd0>
00007FF7751811C3  inc             rcx
00007FF7751811C6  test            r8, r8
00007FF7751811C9  jnz             0x7ff775181190 <LexFile+0x140>
00007FF7751811CB  jmp             0x7ff775181120 <LexFile+0xd0>
00007FF7751811D0  movzx           eax, dl
00007FF7751811D3  mov             r12, rcx
00007FF7751811D6  and             al, 0xdf
00007FF7751811D8  sub             al, 0x41
00007FF7751811DA  cmp             al, 0x19
00007FF7751811DC  jbe             0x7ff775182113 <LexFile+0x10c3>
00007FF7751811E2  cmp             dl, 0x5f
00007FF7751811E5  jz              0x7ff775182113 <LexFile+0x10c3>
00007FF7751811EB  test            dl, dl
00007FF7751811ED  jnz             0x7ff7751811f9 <LexFile+0x1a9>
00007FF7751811EF  mov             word ptr [rbp-0x59], r10w
00007FF7751811F4  jmp             0x7ff775182196 <LexFile+0x1146>
00007FF7751811F9  lea             eax, ptr [rdx-0x21]
00007FF7751811FC  mov             r12, r11
00007FF7751811FF  cmp             eax, 0x5d
00007FF775181202  jnbe            0x7ff77518157d <LexFile+0x52d>
00007FF775181208  cdqe            
00007FF77518120A  movzx           eax, byte ptr [r9+rax*1+0x2414]
00007FF775181213  mov             ecx, dword ptr [r9+rax*4+0x23a4]
00007FF77518121B  add             rcx, r9
00007FF77518121E  jmp             rcx
00007FF775181220  mov             eax, 0x203
