00007FF65EB91050  mov             qword ptr [rsp+0x8], rbx
00007FF65EB91055  push            rdi
00007FF65EB91056  sub             rsp, 0x30
00007FF65EB9105A  mov             rax, qword ptr [rdx+0x20]
00007FF65EB9105E  xor             edi, edi
00007FF65EB91060  mov             r9d, dword ptr [rdx+0x28]
00007FF65EB91064  mov             r10, rdx
00007FF65EB91067  mov             qword ptr [rcx], rdi
00007FF65EB9106A  mov             rbx, rcx
00007FF65EB9106D  mov             qword ptr [rcx+0x8], rdi
00007FF65EB91071  mov             r11d, 0x1
00007FF65EB91077  nop             word ptr [rax+rax*1], ax
00007FF65EB91080  movzx           r8d, byte ptr [rax]
00007FF65EB91084  lea             ecx, ptr [r8-0x1]
00007FF65EB91088  cmp             cl, 0x20
00007FF65EB9108B  jnb             0x7ff65eb910ad <Lexer__NextToken+0x5d>
00007FF65EB9108D  nop             dword ptr [rax], eax
00007FF65EB91090  cmp             r8b, 0xa
00007FF65EB91094  mov             ecx, edi
00007FF65EB91096  movzx           r8d, byte ptr [rax+0x1]
00007FF65EB9109B  setz            cl
00007FF65EB9109E  inc             rax
00007FF65EB910A1  add             r9d, ecx
00007FF65EB910A4  lea             ecx, ptr [r8-0x1]
00007FF65EB910A8  cmp             cl, 0x20
00007FF65EB910AB  jb              0x7ff65eb91090 <Lexer__NextToken+0x40>
00007FF65EB910AD  lea             rdx, ptr [rax+0x1]
00007FF65EB910B1  cmp             r8b, 0x2f
00007FF65EB910B5  jnz             0x7ff65eb910da <Lexer__NextToken+0x8a>
00007FF65EB910B7  cmp             byte ptr [rdx], r8b
00007FF65EB910BA  jnz             0x7ff65eb910da <Lexer__NextToken+0x8a>
00007FF65EB910BC  movzx           ecx, byte ptr [rax+0x2]
00007FF65EB910C0  add             rax, 0x2
00007FF65EB910C4  test            cl, cl
00007FF65EB910C6  jz              0x7ff65eb91080 <Lexer__NextToken+0x30>
00007FF65EB910C8  cmp             cl, 0xa
00007FF65EB910CB  jz              0x7ff65eb91080 <Lexer__NextToken+0x30>
00007FF65EB910CD  movzx           ecx, byte ptr [rax+0x1]
00007FF65EB910D1  inc             rax
00007FF65EB910D4  test            cl, cl
00007FF65EB910D6  jnz             0x7ff65eb910c8 <Lexer__NextToken+0x78>
00007FF65EB910D8  jmp             0x7ff65eb91080 <Lexer__NextToken+0x30>
00007FF65EB910DA  cmp             byte ptr [rax], 0x2f
00007FF65EB910DD  jnz             0x7ff65eb91142 <Lexer__NextToken+0xf2>
00007FF65EB910DF  cmp             byte ptr [rdx], 0x2a
00007FF65EB910E2  jnz             0x7ff65eb91142 <Lexer__NextToken+0xf2>
00007FF65EB910E4  add             rax, 0x2
00007FF65EB910E8  mov             rcx, r11
00007FF65EB910EB  nop             dword ptr [rax+rax*1], eax
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
00007FF65EB91132  add             r9d, eax
00007FF65EB91135  mov             rax, rdx
00007FF65EB91138  test            rcx, rcx
00007FF65EB9113B  jnz             0x7ff65eb910f0 <Lexer__NextToken+0xa0>
00007FF65EB9113D  jmp             0x7ff65eb91080 <Lexer__NextToken+0x30>
00007FF65EB91142  mov             qword ptr [r10+0x20], rax
00007FF65EB91146  mov             dword ptr [r10+0x28], r9d
