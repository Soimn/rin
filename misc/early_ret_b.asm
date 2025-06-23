00007FF7751810EC  int3            
00007FF7751810ED  mov             qword ptr [rbp-0x59], 0x0
00007FF7751810F5  mov             rcx, r12
00007FF7751810F8  mov             qword ptr [rbp-0x51], r14
00007FF7751810FC  nop             dword ptr [rax], eax
00007FF775181100  movzx           edx, byte ptr [rcx]
00007FF775181103  lea             eax, ptr [rdx-0x1]
00007FF775181106  cmp             al, 0x20
00007FF775181108  jnb             0x7ff77518111e <LexFile+0xce>
00007FF77518110A  nop             word ptr [rax+rax*1], ax
00007FF775181110  movzx           edx, byte ptr [rcx+0x1]
00007FF775181114  inc             rcx
00007FF775181117  lea             eax, ptr [rdx-0x1]
00007FF77518111A  cmp             al, 0x20
00007FF77518111C  jb              0x7ff775181110 <LexFile+0xc0>
00007FF77518111E  lea             r11, ptr [rcx+0x1]
00007FF775181122  mov             qword ptr [rbp-0x41], r11
00007FF775181126  cmp             dl, 0x2f
00007FF775181129  jnz             0x7ff775181151 <LexFile+0x101>
00007FF77518112B  cmp             byte ptr [r11], dl
00007FF77518112E  jnz             0x7ff775181155 <LexFile+0x105>
00007FF775181130  movzx           eax, byte ptr [rcx+0x2]
00007FF775181134  add             rcx, 0x2
00007FF775181138  test            al, al
00007FF77518113A  jz              0x7ff775181100 <LexFile+0xb0>
00007FF77518113C  nop             dword ptr [rax], eax
00007FF775181140  cmp             al, 0xa
00007FF775181142  jz              0x7ff775181100 <LexFile+0xb0>
00007FF775181144  movzx           eax, byte ptr [rcx+0x1]
00007FF775181148  inc             rcx
00007FF77518114B  test            al, al
00007FF77518114D  jnz             0x7ff775181140 <LexFile+0xf0>
00007FF77518114F  jmp             0x7ff775181100 <LexFile+0xb0>
00007FF775181151  mov             qword ptr [rbp-0x41], r11
00007FF775181155  movzx           edx, byte ptr [rcx]
00007FF775181158  cmp             dl, 0x2f
00007FF77518115B  jnz             0x7ff7751811b0 <LexFile+0x160>
00007FF77518115D  cmp             byte ptr [r11], 0x2a
00007FF775181161  jnz             0x7ff7751811b0 <LexFile+0x160>
00007FF775181163  add             rcx, 0x2
00007FF775181167  mov             r8, r10
00007FF77518116A  nop             word ptr [rax+rax*1], ax
00007FF775181170  movzx           edx, byte ptr [rcx]
00007FF775181173  cmp             dl, 0x2f
00007FF775181176  jnz             0x7ff775181187 <LexFile+0x137>
00007FF775181178  cmp             byte ptr [rcx+0x1], 0x2a
00007FF77518117C  jnz             0x7ff77518119b <LexFile+0x14b>
00007FF77518117E  inc             r8
00007FF775181181  add             rcx, 0x2
00007FF775181185  jmp             0x7ff7751811a6 <LexFile+0x156>
00007FF775181187  cmp             dl, 0x2a
00007FF77518118A  jnz             0x7ff77518119b <LexFile+0x14b>
00007FF77518118C  cmp             byte ptr [rcx+0x1], 0x2f
00007FF775181190  jnz             0x7ff7751811a3 <LexFile+0x153>
00007FF775181192  dec             r8
00007FF775181195  add             rcx, 0x2
00007FF775181199  jmp             0x7ff7751811a6 <LexFile+0x156>
00007FF77518119B  test            dl, dl
00007FF77518119D  jz              0x7ff775181100 <LexFile+0xb0>
00007FF7751811A3  inc             rcx
00007FF7751811A6  test            r8, r8
00007FF7751811A9  jnz             0x7ff775181170 <LexFile+0x120>
00007FF7751811AB  jmp             0x7ff775181100 <LexFile+0xb0>
00007FF7751811B0  movzx           eax, dl
00007FF7751811B3  mov             r12, rcx
00007FF7751811B6  and             al, 0xdf
00007FF7751811B8  sub             al, 0x41
00007FF7751811BA  cmp             al, 0x19
00007FF7751811BC  jbe             0x7ff7751820d0 <LexFile+0x1080>
00007FF7751811C2  cmp             dl, 0x5f
00007FF7751811C5  jz              0x7ff7751820d0 <LexFile+0x1080>
00007FF7751811CB  test            dl, dl
00007FF7751811CD  jnz             0x7ff7751811d9 <LexFile+0x189>
00007FF7751811CF  mov             word ptr [rbp-0x59], r10w
00007FF7751811D4  jmp             0x7ff77518210c <LexFile+0x10bc>
00007FF7751811D9  lea             eax, ptr [rdx-0x21]
00007FF7751811DC  mov             r12, r11
00007FF7751811DF  cmp             eax, 0x5d
00007FF7751811E2  jnbe            0x7ff775181543 <LexFile+0x4f3>
00007FF7751811E8  cdqe            
00007FF7751811EA  movzx           eax, byte ptr [r9+rax*1+0x238c]
00007FF7751811F3  mov             ecx, dword ptr [r9+rax*4+0x231c]
00007FF7751811FB  add             rcx, r9
00007FF7751811FE  jmp             rcx
00007FF775181200  mov             eax, 0x203
