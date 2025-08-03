typedef struct Checker
{
	bool stub;
} Checker;

typedef union Value
{
	u128 integer;
	f64 floating;
	bool boolean;
	Typeid type_id;
} Value;

typedef enum Value_Flag
{
	ValueFlag_ComptimeKnown = 1,
} Value_Flag;

typedef struct Check_Result
{
	Typeid type;
	Value value;
	u32 value_flags;
	bool valid;
} Check_Result;

static Check_Result
Checker__CheckExpr(Checker* state, AST_Header* expr)
{
	if (expr->kind >= ASTKind__FirstBinaryExpr && expr->kind < ASTKind__PastLastBinaryExpr)
	{
		AST_Binary_Expr* binary = (AST_Binary_Expr*)expr;

		if (ASTPtr_IsNil(&expr->lhs))
		{
			// NOTE: This is not possible with parsed code, but can happen with a generated AST
			//// ERROR: Missing left operand to binary operator
			NOT_IMPLEMENTED;
			return (Check_Result){ .valid = false };
		}

		Check_Result left = Checker__CheckExpr(state, ASTPtr_ToPtr(&binary->lhs));
		if (!left.valid) return left;

		if (ASTPtr_IsNil(&expr->rhs))
		{
			// NOTE: This is not possible with parsed code, but can happen with a generated AST
			//// ERROR: Missing right operand to binary operator
			NOT_IMPLEMENTED;
			return (Check_Result){ .valid = false };
		}

		Check_Result right = Checker__CheckExpr(state, ASTPtr_ToPtr(&binary->rhs));
		if (!right.valid) return right;

		if (binary->kind == ASTKind_Shl || binary->kind == ASTKind_Shr)
		{
			if (!Typeid_IsInteger(left.type))
			{
				//// ERROR: Left operand to shift operator must be of an integer type
				NOT_IMPLEMENTED;
				return (Check_Result){ .valid = false };
			}
			else if (!Typeid_IsInteger(right.type))
			{
				//// ERROR: Right operand to shift operator must be of an integer type
				NOT_IMPLEMENTED;
				return (Check_Result){ .valid = false };
			}

			if (Typeid_IsSoft(left.type) && !(right.value_flags & ValueFlag_ComptimeKnown))
			{
				//// ERROR: Right operand of shift operator with a soft left operand must be known at compile time
				NOT_IMPLEMENTED;
				return (Check_Result){ .valid = false };
			}

			if ((left.value_flags & ValueFlag_ComptimeKnown) && (right.value_flags & ValueFlag_ComptimeKnown))
			{
NOT_IMPLEMENTED;

				return (Check_Result){
					.type        = left.type,
					.value       = ,
					.value_flags = ValueFlag_ComptimeKnown,
					.valid       = true,
				};
			}
			else
			{
				return (Check_Result){
					.type        = left.type,
					.value       = {0},
					.value_flags = 0,
					.valid       = true,
				};
			}
		}
		else
		{
			Typeid common_type;
			Value left_value;
			Value right_value;

			if (Typeid_Equal(left.type, right.type))
			{
				common_type = left.type;
				left_value  = left.value;
				right_value = right.value;
			}
			else if (Typeid_IsSoft(left.type) && Typeid_IsSoft(right.type))
			{
				// NOTE: This makes 2*2.1 possible and is added as an edge case since 2.0*2.1 feels dumb
				if (Typeid_Equal(left.type, Typeid_SoftInt) && Typeid_Equal(right.type, Typeid_SoftFloat))
				{
					common_type = Typeid_SoftFloat;
					left_value  = ;
					right_value = right.value;
				}
				else if (Typeid_Equal(left.type, Typeid_SoftInt) && Typeid_Equal(right.type, Typeid_SoftFloat))
				{
					common_type = Typeid_SoftFloat;
					left_value  = left.value;
					right_value = ;
				}
				else
				{
					//// ERROR: No common soft type
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}
			}
			else if (Typeid_IsSoft(left.type))
			{
				if (!)
				{
					//// ERROR: Cannot harden x to y
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}
				else
				{
					common_type = right.type;
				}
			}
			else if (Typeid_IsSoft(right.type))
			{
				if (!)
				{
					//// ERROR: Cannot harden x to y
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}
				else
				{
					common_type = left.type;
				}
			}
			else
			{
				//// ERROR: No common type
				NOT_IMPLEMENTED;
				return (Check_Result){ .valid = false };
			}

			switch (binary->kind)
			{
				// int & float
				case ASTKind_Mul:
				case ASTKind_Div:
				case ASTKind_Rem:
				case ASTKind_Add:
				case ASTKind_Sub:
				{
					NOT_IMPLEMENTED;
				} break;

				// int & bool
				case ASTKind_And:
				case ASTKind_Or:
				case ASTKind_Xor:
				{
					NOT_IMPLEMENTED;
				} break;

				// int, float, bool
				case ASTKind_CmpEq:
				case ASTKind_CmpNeq:
				{
					NOT_IMPLEMENTED;
				} break;

				// int, float
				case ASTKind_CmpLt:
				case ASTKind_CmpLtEq:
				case ASTKind_CmpGt:
				case ASTKind_CmpGtEq:
				{
					NOT_IMPLEMENTED;
				} break;

				// boolean
				case ASTKind_LAnd:
				case ASTKind_LOr:
				{
					NOT_IMPLEMENTED;
				} break;

				default:
				{
					//// ERROR: Illegal kind
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				} break;
			}
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
					.type        = Typeid_SoftInt,
					.value       = { .integer = ((AST_Int*)expr)->value },
					.value_flags = ValueFlag_ComptimeKnown,
					.valid       = true,
				};
			} break;

			case ASTKind_Int128:
			{
				return (Check_Result){
					.type        = Typeid_SoftInt,
					.value       = { .integer = ((AST_Int128*)expr)->value },
					.value_flags = ValueFlag_ComptimeKnown,
					.valid       = true,
				};
			} break;

			case ASTKind_Float:
			{
				return (Check_Result){
					.type        = Typeid_SoftFloat,
					.value       = { .floating = ((AST_Float*)expr)->value },
					.value_flags = ValueFlag_ComptimeKnown,
					.valid       = true,
				};
			} break;

			case ASTKind_Bool:
			{
				return (Check_Result){
					.type        = Typeid_SoftBool,
					.value       = { .boolean = ((AST_Bool*)expr)->value },
					.value_flags = ValueFlag_ComptimeKnown,
					.valid       = true,
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

				if (ASTPtr_IsNil(&pointer_to->elem_type))
				{
					// NOTE: This is not possible with parsed code, but can happen with a generated AST
					//// ERROR: Missing operand to pointer type prefix operator
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}

				Check_Result operand = Checker__CheckExpr(state, ASTPtr_ToPtr(&pointer_to->elem_type));
				if (!operand.valid) return operand;

				if (!Typeid_Equal(operand.type, Typeid_Typeid))
				{
					//// ERROR: Operand to pointer type prefix operator must be a typeid
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}

				if (!(operand.value_flags & ValueFlag_ComptimeKnown))
				{
					//// ERROR: Operand to pointer type prefix operator must be known at compile time
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}

				return (Check_Result){
					.type        = Typeid_Typeid,
					.value       = { .type_id = Typeid_PointerTo(operand.value.type_id) },
					.value_flags = ValueFlag_ComptimeKnown,
					.valid       = true,
				};
			} break;

			case ASTKind_SliceOf:
			{
				AST_SliceOf* slice_of = (AST_SliceOf*)expr;

				if (ASTPtr_IsNil(&slice_of->elem_type))
				{
					// NOTE: This is not possible with parsed code, but can happen with a generated AST
					//// ERROR: Missing operand to slice type prefix operator
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}

				Check_Result operand = Checker__CheckExpr(state, ASTPtr_ToPtr(&slice_of->elem_type));
				if (!operand.valid) return operand;

				if (!Typeid_Equal(operand.type, Typeid_Typeid))
				{
					//// ERROR: Operand to slice type prefix operator must be a typeid
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}

				if (!(operand.value_flags & ValueFlag_ComptimeKnown))
				{
					//// ERROR: Operand to slice type prefix operator must be known at compile time
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}

				return (Check_Result){
					.type        = Typeid_Typeid,
					.value       = { .type_id = Typeid_SliceOf(operand.value.type_id) },
					.value_flags = ValueFlag_ComptimeKnown,
					.valid       = true,
				};
			} break;

			case ASTKind_ArrayOf:
			{
				AST_ArrayOf* array_of = (AST_ArrayOf*)expr;

				if (ASTPtr_IsNil(&array_of->len))
				{
					// NOTE: This is not possible with parsed code, but can happen with a generated AST
					//// ERROR: Missing length operand to array type prefix operator
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}

				Check_Result len = Checker__CheckExpr(state, ASTPtr_ToPtr(&array_of->len));
				if (!len.valid) return len;

				if (!Typeid_IsInteger(len.type))
				{
					//// ERROR: Length operand to array type prefix operator must be of an integer type
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}

				if (!(len.value_flags & ValueFlag_ComptimeKnown))
				{
					//// ERROR: Length operand to array type prefix operator must be known at compile time
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}

				u32 safe_len = (u32)len.value.integer.lo;
				if (len.value.integer.hi != 0 || len.value.integer.lo > ~(u32)0)
				{
					//// ERROR: Lenght operand to array type prefix must fit in 32 bits
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}

				if (ASTPtr_IsNil(&array_of->elem_type))
				{
					// NOTE: This is not possible with parsed code, but can happen with a generated AST
					//// ERROR: Missing operand to array type prefix operator
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}

				Check_Result operand = Checker__CheckExpr(state, ASTPtr_ToPtr(&array_of->elem_type));
				if (!operand.valid) return operand;

				if (!Typeid_Equal(operand.type, Typeid_Typeid))
				{
					//// ERROR: Operand to array type prefix operator must be a typeid
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}

				if (!(operand.value_flags & ValueFlag_ComptimeKnown))
				{
					//// ERROR: Operand to array type prefix operator must be known at compile time
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}

				return (Check_Result){
					.type        = Typeid_Typeid,
					.value       = { .type_id = Typeid_ArrayOf(operand.value.type_id, safe_len) },
					.value_flags = ValueFlag_ComptimeKnown,
					.valid       = true,
				};
			} break;

			case ASTKind_Deref:
			{
				AST_Deref* deref = (AST_Deref*)expr;

				if (ASTPtr_IsNil(&deref->operand))
				{
					// NOTE: This is not possible with parsed code, but can happen with a generated AST
					//// ERROR: Missing operand to pointer dereference operator
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}

				Check_Result operand = Checker__CheckExpr(state, ASTPtr_ToPtr(&deref->operand));
				if (!operand.valid) return operand;

				if (!Typeid_IsPointer(operand.type))
				{
					//// ERROR: Only values of pointer type can be dereferenced
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}

				// NOTE: The deref operator ignores whether or not the operand is comptime known

				return (Check_Result){
					.type        = Typeid_ElementType(operand.type),
					.value       = {0},
					.value_flags = 0,
					.valid       = true,
				};
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
			case ASTKind_Dec:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Conditional:
			{
				AST_Conditional* conditional = (AST_Conditional*)expr;

				if (ASTPtr_IsNil(&conditional->condition))
				{
					// NOTE: This is not possible with parsed code, but can happen with a generated AST
					//// ERROR: Missing condition of conditional expression
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}

				Check_Result condition = Checker__CheckExpr(state, ASTPtr_ToPtr(&conditional->condition));
				if (!condition.valid) return condition;

				if (!Typeid_IsBoolean(condition.type))
				{
					//// ERROR: Condition of conditional expression must be implicitly convertible to bool
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}

				if (ASTPtr_IsNil(&conditional->true_val))
				{
					// NOTE: This is not possible with parsed code, but can happen with a generated AST
					//// ERROR: Missing true value of conditional expression
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}

				Check_Result true_val = Checker__CheckExpr(state, ASTPtr_ToPtr(&conditional->true_val));
				if (!true_val.valid) return true_val;

				if (ASTPtr_IsNil(&conditional->false_val))
				{
					// NOTE: This is not possible with parsed code, but can happen with a generated AST
					//// ERROR: Missing false value of conditional expression
					NOT_IMPLEMENTED;
					return (Check_Result){ .valid = false };
				}

				Check_Result false_val = Checker__CheckExpr(state, ASTPtr_ToPtr(&conditional->false_val));
				if (!false_val.valid) return false_val;

				NOT_IMPLEMENTED;
			} break;

			default:
			{
				//// ERROR: This should never happen with parsed input
				NOT_IMPLEMENTED;
				return (Check_Result){ .valid = false };
			} break;
		}
	}
}
