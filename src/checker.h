typedef struct Checker
{
	bool stub;
} Checker;

typedef union Value
{
	s64 int64;
	u64 uint64;
	f64 float64;
} Value;

typedef struct Check_Result
{
	Typeid type;
	Value value;
	bool has_value;
	bool is_valid;
} Check_Result;

#define CHECK_FAILED (Check_Result){ .valid = false }

static Check_Result
Checker__CheckExpr(Checker* state, AST_Header* expr)
{
	if (expr->kind >= ASTKind__FirstBinaryExpr && expr->kind < ASTKind__PastLastBinaryExpr)
	{
		AST_Binary_Expr* binary = (AST_Binary_Expr*)expr;

		Check_Result left = Checker__CheckExpr(state, ASTPtr_ToPtr(&binary->lhs));
		if (!left.is_valid) return left;

		Check_Result right = Checker__CheckExpr(state, ASTPtr_ToPtr(&binary->rhs));
		if (!right.is_valid) return right;

		if (left.type == right.type)
		{
			common_type = left.type;
		}
		else if (Typeid_IsSoft(left.type))
		{
			if (left.type == Typeid_SoftInt && right.type == Typeid_SoftFloat)
			{
				// TODO: convert left
				common_type = Typeid_SoftFloat;
			}
			else if (Typeid_IsSoft(right.type))
			{
				//// ERROR: No common soft type
				NOT_IMPLEMENTED;
				return CHECK_FAILED;
			}
			else if (CanHardenTo(left.type, right.type))
			{
				// TODO: convert left
				common_type = right.type;
			}
			else
			{
				//// ERROR: Cannot harden to type
				NOT_IMPLEMENTED;
				return CHECK_FAILED;
			}
		}
		else if (Typeid_IsSoft(right.type))
		{
			if (right.type == Typeid_SoftInt && left.type == Typeid_SoftFloat)
			{
				// TODO: convert right
				common_type = Typeid_SoftFloat;
			}
			else if (Typeid_IsSoft(left.type))
			{
				//// ERROR: No common soft type
				NOT_IMPLEMENTED;
				return CHECK_FAILED;
			}
			else if (CanHardenTo(right.type, left.type))
			{
				// TODO: convert right
				common_type = left.type;
			}
			else
			{
				//// ERROR: Cannot harden to type
				NOT_IMPLEMENTED;
				return CHECK_FAILED;
			}
		}
		else
		{
			//// ERROR: Mismatched types
			NOT_IMPLEMENTED;
			return CHECK_FAILED;
		}

		switch (expr->kind)
		{
			// int
			case ASTKind_Shl:
			case ASTKind_Shr:
			{
				if (!Typeid_IsInteger(common_type))
				{
					//// ERROR: Shift operator requires integer operands
					NOT_IMPLEMENTED;
					return CHECK_FAILED;
				}

				NOT_IMPLEMENTED;
			} break;

			// int
			case ASTKind_Rem:
			{
				if (!Typeid_IsInteger(common_type))
				{
					//// ERROR: Remainder operator requires integer operands
					NOT_IMPLEMENTED;
					return CHECK_FAILED;
				}

				NOT_IMPLEMENTED;
			} break;

			// int, float
			case ASTKind_Mul:
			case ASTKind_Div:
			case ASTKind_Add:
			case ASTKind_Sub:
			{
				if (!Typeid_IsInteger(common_type) && !Typeid_IsFloat(common_type))
				{
					//// ERROR: X operator requires integer or floating point operands
					NOT_IMPLEMENTED;
					return CHECK_FAILED;
				}

				if (!left.has_value || !right.has_value)
				{
					return (Check_Result){
						.type      = common_type,
						.has_value = false,
						.is_valid  = true,
					};
				}
				else
				{
					NOT_IMPLEMENTED;

					return (Check_Result){
						.type      = common_type,
						.value     = ,
						.has_value = true,
						.is_valid  = true,
					};
				}
			} break;

			// comparable (int, float, bool, pointer, string)
			case ASTKind_CmpEq:
			case ASTKind_CmpNeq:
			{
				NOT_IMPLEMENTED;
			} break;

			// ordered (int, float, pointer)
			case ASTKind_CmpLt:
			case ASTKind_CmpLtEq:
			case ASTKind_CmpGt:
			case ASTKind_CmpGtEq:
			{
				NOT_IMPLEMENTED;
			} break;

			// int, bool
			case ASTKind_And:
			case ASTKind_Or:
			case ASTKind_Xor:
			{
				if (!Typeid_IsInteger(common_type) && !Typeid_IsBool(common_type))
				{
					//// ERROR: X operator requires integer or boolean operands
					NOT_IMPLEMENTED;
					return CHECK_FAILED;
				}

				NOT_IMPLEMENTED;
			} break;

			// bool
			case ASTKind_LAnd:
			case ASTKind_LOr:
			{
				if (!Typeid_IsInteger(common_type) && !Typeid_IsBool(common_type))
				{
					//// ERROR: X operator requires boolean operands
					NOT_IMPLEMENTED;
					return CHECK_FAILED;
				}

				NOT_IMPLEMENTED;
			} break;

			default:
			{
				//// ERROR: Illegal AST kind
				NOT_IMPLEMENTED;
				return CHECK_FAILED;
			} break;
		}
	}
	else
	{
		switch (expr->kind)
		{
			case ASTKind_Ident:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_String:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Char:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Int:
			{
				return (Check_Result){
					.type      = Typeid_SoftInt,
					.value     = { .int64 = ((AST_Int*)expr)->value },
					.has_value = true,
					.is_valid  = true,
				};
			} break;

			case ASTKind_Float:
			{
				return (Check_Result){
					.type      = Typeid_SoftFloat,
					.value     = { .float64 = ((AST_Float*)expr)->value },
					.has_value = true,
					.is_valid  = true,
				};
			} break;

			case ASTKind_Bool:
			{
				return (Check_Result){
					.type      = Typeid_SoftBool,
					.value     = { .bool8 = ((AST_Bool*)expr)->value },
					.has_value = true,
					.is_valid  = true,
				};
			} break;

			case ASTKind_Compound:
			{
				return Checker__CheckExpr(state, ASTPtr_ToPtr(&((AST_Compound*)expr)->inner_expr));
			} break;

			case ASTKind_ProcType:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_ProcLit:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_StructType:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_EnumType:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_PointerTo:
			{
				AST_PointerTo* pointer_to = (AST_PointerTo*)expr;

				Check_Result elem_type = Checker__CheckExpr(state, ASTPtr_ToPtr(&pointer_to->elem_type));
				if (!elem_type.is_valid) return elem_type;

				if (elem_type != Typeid_Typeid)
				{
					//// ERROR: Pointer type prefix operator requires a typeid as operand
					NOT_IMPLEMENTED;
					return CHECK_FAILED;
				}

				if (!elem_type.has_value)
				{
					//// ERROR: Operand to pointer type prefix operator must be known at compile time
					NOT_IMPLEMENTED;
					return CHECK_FAILED;
				}

				return (Check_Result){
					.type      = Typeid_Typeid,
					.value     = { .type_id = Typeid_PointerTo(elem_type.value.type_id) },
					.has_value = true,
					.is_valid  = true,
				};
			} break;

			case ASTKind_SliceOf:
			{
				AST_SliceOf* slice_of = (AST_SliceOf*)expr;

				Check_Result elem_type = Checker__CheckExpr(state, ASTPtr_ToPtr(&slice_of->elem_type));
				if (!elem_type.is_valid) return elem_type;

				if (elem_type != Typeid_Typeid)
				{
					//// ERROR: Slice type prefix operator requires a typeid as operand
					NOT_IMPLEMENTED;
					return CHECK_FAILED;
				}

				if (!elem_type.has_value)
				{
					//// ERROR: Operand to slice type prefix operator must be known at compile time
					NOT_IMPLEMENTED;
					return CHECK_FAILED;
				}

				return (Check_Result){
					.type      = Typeid_Typeid,
					.value     = { .type_id = Typeid_SliceOf(elem_type.value.type_id) },
					.has_value = true,
					.is_valid  = true,
				};
			} break;

			case ASTKind_ArrayOf:
			{
				AST_ArrayOf* array_of = (AST_ArrayOf*)expr;

				Check_Result len = Checker__CheckExpr(state, ASTPtr_ToPtr(&array_of->len));
				if (!len.is_valid) return len;

				if (!Typeid_IsInteger(len.type))
				{
					//// ERROR: Length operand to array type prefix operator must be an integer
					NOT_IMPLEMENTED;
					return CHECK_FAILED;
				}

				if (!len.has_value)
				{
					//// ERROR: Length operand to array type prefix operator must be known at compile time
					NOT_IMPLEMENTED;
					return CHECK_FAILED;
				}

				u32 len_value;

				NOT_IMPLEMENTED;

				Check_Result elem_type = Checker__CheckExpr(state, ASTPtr_ToPtr(&array_of->elem_type));
				if (!elem_type.is_valid) return elem_type;

				if (elem_type.type != Typeid_Typeid)
				{
					//// ERROR: Element type operand to array type prefix operator must be a typeid
					NOT_IMPLEMENTED;
					return CHECK_FAILED;
				}

				if (!elem_type.has_value)
				{
					//// ERROR: Element type operand to array type prefix operator must be known at compile time
					NOT_IMPLEMENTED;
					return CHECK_FAILED;
				}

				return (Check_Result){
					.type      = Typeid_Typeid,
					.value     = { .type_id = Typeid_ArrayOf(len_value, elem_type.value.type_id) },
					.has_value = true,
					.is_valid  = true,
				};
			} break;

			case ASTKind_DistinctOf:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Deref:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Call:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Index:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Slice:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Member:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_StructLit:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Pos:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Neg:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Not:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_LNot:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Ref:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Inc:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Dec:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Conditional:
			{
				NOT_IMPLEMENTED;
			} break;

			default:
			{
				//// ERROR: Illegal AST kind
				NOT_IMPLEMENTED;
				return CHECK_FAILED;
			} break;
		}
	}
}
