#define NOMINMAX            1
#define WIN32_LEAN_AND_MEAN 1
#define WIN32_MEAN_AND_LEAN 1
#define VC_EXTRALEAN        1
#include <windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN
#undef WIN32_MEAN_AND_LEAN
#undef VC_EXTRALEAN
#undef far
#undef near

#include <stdio.h>

#include "rin.h"

void
AssertHandler(char* file, int line, char* expr)
{
	fprintf(stderr, "ASSERTION FAILED\n%s(%d): %s\n", file, line, expr);
	ExitProcess((UINT)-1);
}

void*
ReserveMemory(umm size)
{
	void* result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);

	if (result == 0)
	{
		//// ERROR: Out of memory
		ExitProcess((UINT)-1);
	}

	return result;
}

void
CommitMemory(void* base, umm size)
{
	if (VirtualAlloc(base, size, MEM_COMMIT, PAGE_READWRITE) == 0)
	{
		//// ERROR: Out of memory
		ExitProcess((UINT)-1);
	}
}

void
ReleaseMemory(void* base)
{
	VirtualFree(base, 0, MEM_RELEASE);
}

void
Indent(umm n)
{
	for (umm i = 0; i < n; ++i) printf(" ");
}

void
PrintAST(AST* ast, umm indent)
{
	Indent(indent);

	if (ast == 0) printf("(null)");
	else
	{
		switch (ast->kind)
		{
			case AST_Ident:
			{
				printf("AST_Ident(%.*s)", ((AST_Ident_Expr*)ast)->ident.len, ((AST_Ident_Expr*)ast)->ident.data);
			} break;

			case AST_String:
			{
				// TODO
				NOT_IMPLEMENTED;
			} break;

			case AST_Int:
			{
				printf("AST_Int(%llu)", ((AST_Int_Expr*)ast)->value);
			} break;

			case AST_Float:
			{
				// TODO
				NOT_IMPLEMENTED;
			} break;

			case AST_Bool:
			{
				printf("AST_Bool(%s)", (((AST_Bool_Expr*)ast)->value ? "true" : "false"));
			} break;

			case AST_Compound:
			{
				printf("AST_Compound\n");
				PrintAST(((AST_Compound_Expr*)ast)->inner, indent+1);
			} break;

			case AST_ProcType:
			{
				AST_Proc_Type_Expr* proc = (AST_Proc_Type_Expr*)ast;
				
				printf("AST_Proc_Type\n");

				Indent(indent+1);
				printf("params:\n");
				PrintAST(proc->params, indent+2);

				Indent(indent+1);
				printf("ret_types:\n");
				PrintAST(proc->ret_types, indent+2);
			} break;

			case AST_StructType:
			{
			} break;

			case AST_ProcLit:
			{
				AST_Proc_Lit_Expr* proc = (AST_Proc_Lit_Expr*)ast;
				
				printf("AST_Proc_Lit\n");

				Indent(indent+1);
				printf("params:\n");
				PrintAST(proc->params, indent+2);

				Indent(indent+1);
				printf("ret_types:\n");
				PrintAST(proc->ret_types, indent+2);

				Indent(indent+1);
				printf("body:\n");
				PrintAST(proc->body, indent+2);
			} break;

			case AST_Deref:
			{
			} break;

			case AST_Call:
			{
			} break;

			case AST_Slice:
			{
			} break;

			case AST_Index:
			{
			} break;

			case AST_Member:
			{
			} break;

			case AST_StructLit:
			{
			} break;

			case AST_PostInc:
			{
			} break;

			case AST_PostDec:
			{
			} break;

			case AST_Pos:
			{
			} break;

			case AST_Neg:
			{
			} break;

			case AST_LNot:
			{
			} break;

			case AST_Not:
			{
			} break;

			case AST_Ref:
			{
			} break;

			case AST_PointerTo:
			{
			} break;

			case AST_SliceOf:
			{
			} break;

			case AST_ArrayOf:
			{
			} break;

			case AST_PreInc:
			{
			} break;

			case AST_PreDec:
			{
			} break;

			case AST_Mul:
			{
			} break;

			case AST_Div:
			{
			} break;

			case AST_Rem:
			{
			} break;

			case AST_And:
			{
			} break;

			case AST_Shl:
			{
			} break;

			case AST_Shr:
			{
			} break;

			case AST_Sar:
			{
			} break;

			case AST_Add:
			{
			} break;

			case AST_Sub:
			{
			} break;

			case AST_Or:
			{
			} break;

			case AST_Xor:
			{
			} break;

			case AST_CmpEQ:
			{
			} break;

			case AST_CmpNeq:
			{
			} break;

			case AST_CmpLe:
			{
			} break;

			case AST_CmpLeEQ:
			{
			} break;

			case AST_CmpGe:
			{
			} break;

			case AST_CmpGeEQ:
			{
			} break;

			case AST_LAnd:
			{
			} break;

			case AST_LOr:
			{
			} break;

			case AST_Conditional:
			{
			} break;

			case AST_Var:
			{
			} break;

			case AST_Const:
			{
				printf("AST_Const\n");
				Indent(indent+1);
				printf("names:\n");
				for (AST* name = ((AST_Const_Decl*)ast)->names; name != 0; name = name->next) PrintAST(name, indent+2);
				Indent(indent+1);
				printf("type:\n");
				PrintAST(((AST_Const_Decl*)ast)->type, indent+2);
				Indent(indent+1);
				printf("values:\n");
				for (AST* value = ((AST_Const_Decl*)ast)->values; value != 0; value = value->next) PrintAST(value, indent+2);
			} break;

			case AST_Block:
			{
				printf("AST_Block\n");
				Indent(indent+1);
				printf("label:\n");
				PrintAST(((AST_Block_Stmnt*)ast)->label, indent+2);
				Indent(indent+1);
				printf("body:\n");
				for (AST* stmnt = ((AST_Block_Stmnt*)ast)->body; stmnt != 0; stmnt = stmnt->next) PrintAST(stmnt, indent+2);
			} break;

			case AST_If:
			{
			} break;

			case AST_While:
			{
			} break;

			case AST_Return:
			{
				printf("AST_Return\n");

				for (AST* arg = ((AST_Return_Stmnt*)ast)->args; arg != 0; arg = arg->next)
				{
					PrintAST(arg, indent+1);
				}
			} break;

			case AST_Break:
			{
			} break;

			case AST_Continue:
			{
			} break;

			case AST_MulEQ:
			{
			} break;

			case AST_DivEQ:
			{
			} break;

			case AST_RemEQ:
			{
			} break;

			case AST_AndEQ:
			{
			} break;

			case AST_ShlEQ:
			{
			} break;

			case AST_ShrEQ:
			{
			} break;

			case AST_SarEQ:
			{
			} break;

			case AST_AddEQ:
			{
			} break;

			case AST_SubEQ:
			{
			} break;

			case AST_OrEQ:
			{
			} break;

			case AST_XorEQ:
			{
			} break;

			case AST_LAndEQ:
			{
			} break;

			case AST_LOrEQ:
			{
			} break;

			case AST_EQ:
			{
			} break;

			case AST_Arg:
			{
				printf("AST_Arg\n");
				Indent(indent+1);
				printf("name:\n");
				PrintAST(((AST_Argument*)ast)->name, indent+2);
				Indent(indent+1);
				printf("value:\n");
				PrintAST(((AST_Argument*)ast)->value, indent+2);
			} break;

			case AST_Param:
			{
			} break;

			case AST_RetType:
			{
			} break;
		}
	}

	printf("\n");
}

int
main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	{ // TODO: find a better way of doing this
		for (umm i = 0; i < ARRAY_LEN(Token_KeywordIdents); ++i)
		{
			Token_KeywordIdents[i] = Ident_FromString(Token_KeywordStrings[i]);
		}
	}

	if (argc == 3 && CString_Match((u8*)argv[1], (u8*)"test_parse"))
	{
		u8* input = 0;

		FILE* file = 0;
		if (fopen_s(&file, argv[2], "rb") != 0) printf("ERROR: Failed to open input file %s\n", argv[2]);
		else
		{
			fseek(file, 0, SEEK_END);
			umm file_size = ftell(file);
			rewind(file);

			u8* contents = calloc(file_size + 1, 1);

			if (contents == 0 || fread(contents, 1, file_size, file) != file_size) printf("ERROR: failed to read input file\n");
			else
			{
				input = contents;
			}

			fclose(file);
		}

		if (input != 0)
		{
			Arena arena = Arena_Create(1ULL << 30);

			AST* ast = 0;
			bool result = Parser_ParseFile(arena, input, &ast);

			printf("Parsing %s\n", (result ? "succeeded" : "failed"));

			if (result)
			{
				printf("\n");
				PrintAST(ast, 0);
			}
		}
	}
	else printf("idk what you are trying to do man, but this is not it\n");

	return 0;
}
