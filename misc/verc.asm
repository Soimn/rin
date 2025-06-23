00007FF65EB91050  mov             qword ptr [rsp+0x8], rbx
00007FF65EB91055  push            rdi
00007FF65EB91056  sub             rsp, 0x30
00007FF65EB9105A  mov             rax, qword ptr [rdx+0x20]
00007FF65EB9105E  xor             edi, edi
00007FF65EB91060  mov             r10d, dword ptr [rdx+0x28]
00007FF65EB91064  mov             r9, rdx
00007FF65EB91067  mov             qword ptr [rcx], rdi
00007FF65EB9106A  mov             rbx, rcx
00007FF65EB9106D  mov             qword ptr [rcx+0x8], rdi
00007FF65EB91071  mov             r11d, 0x1
00007FF65EB91077  nop             word ptr [rax+rax*1], ax
00007FF65EB91080  movzx           r8d, byte ptr [rax]
00007FF65EB91084  lea             ecx, ptr [r8-0x1]
00007FF65EB91088  cmp             cl, 0x20
00007FF65EB9108B  jnb             0x7ff65eb910a1 <Lexer__NextToken+0x51>
00007FF65EB9108D  nop             dword ptr [rax], eax
00007FF65EB91090  movzx           r8d, byte ptr [rax+0x1]
00007FF65EB91095  inc             rax
00007FF65EB91098  lea             ecx, ptr [r8-0x1]
00007FF65EB9109C  cmp             cl, 0x20
00007FF65EB9109F  jb              0x7ff65eb91090 <Lexer__NextToken+0x40>
00007FF65EB910A1  lea             rdx, ptr [rax+0x1]
00007FF65EB910A5  cmp             r8b, 0x2f
00007FF65EB910A9  jnz             0x7ff65eb910d2 <Lexer__NextToken+0x82>
00007FF65EB910AB  cmp             byte ptr [rdx], r8b
00007FF65EB910AE  jnz             0x7ff65eb910d2 <Lexer__NextToken+0x82>
00007FF65EB910B0  movzx           ecx, byte ptr [rax+0x2]
00007FF65EB910B4  add             rax, 0x2
00007FF65EB910B8  test            cl, cl
00007FF65EB910BA  jz              0x7ff65eb91080 <Lexer__NextToken+0x30>
00007FF65EB910BC  nop             dword ptr [rax], eax
00007FF65EB910C0  cmp             cl, 0xa
00007FF65EB910C3  jz              0x7ff65eb91080 <Lexer__NextToken+0x30>
00007FF65EB910C5  movzx           ecx, byte ptr [rax+0x1]
00007FF65EB910C9  inc             rax
00007FF65EB910CC  test            cl, cl
00007FF65EB910CE  jnz             0x7ff65eb910c0 <Lexer__NextToken+0x70>
00007FF65EB910D0  jmp             0x7ff65eb91080 <Lexer__NextToken+0x30>
00007FF65EB910D2  cmp             byte ptr [rax], 0x2f
00007FF65EB910D5  jnz             0x7ff65eb91142 <Lexer__NextToken+0xf2>
00007FF65EB910D7  cmp             byte ptr [rdx], 0x2a
00007FF65EB910DA  jnz             0x7ff65eb91142 <Lexer__NextToken+0xf2>
00007FF65EB910DC  add             rax, 0x2
00007FF65EB910E0  mov             rcx, r11
00007FF65EB910E3  nop             dword ptr [rax], eax
00007FF65EB910E7  nop             word ptr [rax+rax*1], ax
00007FF65EB910F0  movzx           r8d, byte ptr [rax]
00007FF65EB910F4  lea             rdx, ptr [rax+0x1]
00007FF65EB910F8  cmp             r8b, 0x2f
00007FF65EB910FC  jnz             0x7ff65eb9110c <Lexer__NextToken+0xbc>
00007FF65EB910FE  cmp             byte ptr [rdx], 0x2a
00007FF65EB91101  jnz             0x7ff65eb9110c <Lexer__NextToken+0xbc>
00007FF65EB91103  inc             rcx
00007FF65EB91106  add             rax, 0x2
00007FF65EB9110A  jmp             0x7ff65eb91138 <Lexer__NextToken+0xe8>
00007FF65EB9110C  cmp             r8b, 0x2a
00007FF65EB91110  jnz             0x7ff65eb91120 <Lexer__NextToken+0xd0>
00007FF65EB91112  cmp             byte ptr [rdx], 0x2f
00007FF65EB91115  jnz             0x7ff65eb91129 <Lexer__NextToken+0xd9>
00007FF65EB91117  dec             rcx
00007FF65EB9111A  add             rax, 0x2
00007FF65EB9111E  jmp             0x7ff65eb91138 <Lexer__NextToken+0xe8>
00007FF65EB91120  test            r8b, r8b
00007FF65EB91123  jz              0x7ff65eb91080 <Lexer__NextToken+0x30>
00007FF65EB91129  mov             eax, edi
00007FF65EB9112B  cmp             r8b, 0xa
00007FF65EB9112F  setz            al
00007FF65EB91132  add             r10d, eax
00007FF65EB91135  mov             rax, rdx
00007FF65EB91138  test            rcx, rcx
00007FF65EB9113B  jnz             0x7ff65eb910f0 <Lexer__NextToken+0xa0>
00007FF65EB9113D  jmp             0x7ff65eb91080 <Lexer__NextToken+0x30>
00007FF65EB91142  mov             qword ptr [r9+0x20], rax
00007FF65EB91146  mov             dword ptr [r9+0x28], r10d
