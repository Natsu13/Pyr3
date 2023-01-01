#include "BytecodeDebuger.h"

void BytecodeDebuger::debug() {
	printf("\n");

	for (int i = 0; i < instructions.size(); i++) {
		auto instruction = instructions[i];

		printf("%7d: ", instruction->index_instruction);
#if _DEBUG
		//printf("%10d: ", instruction->serial);
#endif

		switch (instruction->instruction) {
			case BYTECODE_NOOP: {
				printf("%12s", "noop");
				break;
			}
			case BYTECODE_EMIT_TYPE: {
				auto type_to = (AST_Type*)instruction->big_constant._pointer;

				printf("%12s %s -> v%d", "emit_type", typeResolver->typeToString(type_to).data, instruction->index_r);
				break;
			}
			case BYTECODE_CAST: {
				auto type_from = (AST_Type*)types[instruction->index_a];
				auto type_to = (AST_Type*)types[instruction->index_r];

				printf("%12s v%lld(%s) -> v%d(%s)", "cast", instruction->index_a, typeResolver->typeToString(type_from).data, instruction->index_r, typeResolver->typeToString(type_to).data);
				break;
			}
			case BYTECODE_C_CALL_FROM_PROCEDURE: {
				auto procedure = (AST_Procedure*)instruction->big_constant._pointer;
				printf("%12s v%d -> v%d", "c_call", procedure->bytecode_address, instruction->index_r);
				break;
			}
			case BYTECODE_CALL_PROCEDURE: {
				auto call = static_cast<Call_Record*>(instruction->big_constant._pointer);
				auto procedure = call->procedure;

				String args = "";
				for (int q = 0; q < call->arguments.size(); q++) {
					auto arg = call->arguments[q];
					if (!args.isEmpty()) args += ", ";

					char buffer[33];
					_itoa_s(arg->bytecode_address, buffer, 10);
					args += "v"; 
					args += (&buffer[0]);
				}				
				printf("%12s %s:%d(%s)", "call", call->name.data, call->procedure->bytecode_address, args.data);
				if (call->return_register != -1) {
					printf(" -> v%d", call->return_register);
				}
				break;
			}
			case BYTECODE_INTEGER_ADD_TO_CONSTANT: {
				printf("%12s v%d += %I64d", "add_int", instruction->index_r, instruction->big_constant._s64);
				break;
			}
			case BYTECODE_RESERVE_MEMORY_TO_R_PLUS_OFFSET: {
				printf("%12s [v%d + %lld] >> %lld", "malloc", instruction->index_r, instruction->index_b, instruction->index_a);
				break;
			}
			case BYTECODE_RESERVE_MEMORY_TO_R: {
				if (instruction->options == OPTION_RESERVE_MEMORY_TO_R_BY_ADDRESS) {
					printf("%12s *v%d >> %lld", "malloc", instruction->index_r, instruction->index_a);
				}
				if (instruction->options == 0) {
					printf("%12s v%d >> %lld", "malloc", instruction->index_r, instruction->index_a);
				}
				else {
					printf("%12s v%d >> v%lld", "malloc", instruction->index_r, instruction->index_a);
				}
				break;
			}
			case BYTECODE_ASSING_TO_BIG_CONSTANT: {
				auto type = (AST_Type_Definition*)types[instruction->index_r];
				if (type->internal_type == AST_Type_string) {
					New_String* str = (New_String*)instruction->big_constant._pointer;
					printf("%12s v%d = string(%lld)", "constant", instruction->index_r, str->size);
				}
				else {
					printf("%12s v%d = %I64d", "constant", instruction->index_r, instruction->big_constant._s64);
				}				
				break;
			}
			case BYTECODE_MOVE_A_TO_R: {
				printf("%12s v%d, v%lld", "mov1", instruction->index_r, instruction->index_a);
				break;
			}
			case BYTECODE_MOVE_A_REGISTER_TO_R: {
				printf("%12s v%d, [v%lld]", "mov2", instruction->index_r, instruction->index_a);
				break;
			}
			case BYTECODE_MOVE_A_BY_REFERENCE_TO_R: {
				printf("%12s v%d, *v%lld", "mov3", instruction->index_r, instruction->index_a);
				break;
			}
			case BYTECODE_MOVE_A_TO_R_PLUS_OFFSET: {
				printf("%12s v%lld, [v%d + %lld]", "mov4", instruction->index_a, instruction->index_r, instruction->index_b);
				break;
			}
			case BYTECODE_MOVE_A_PLUS_OFFSET_TO_R: {
				printf("%12s v%d, *v%lld + %lld", "mov5", instruction->index_r, instruction->index_a, instruction->index_b);
				break;
			}
			case BYTECODE_MOVE_A_TO_R_PLUS_OFFSET_REG: {
				/*
				void* pos = (int8_t*)this->registers[bc->index_r]._pointer + this->registers[bc->index_b]._s64;
				memcpy(pos, &this->registers[bc->index_a]._s64, sizeof(this->registers[bc->index_a]._s64));	
				*/
				if ((instruction->options & OPTION_MOVE_A_TO_R_PLUS_OFFSET_REG_NOREVERSE) == OPTION_MOVE_A_TO_R_PLUS_OFFSET_REG_NOREVERSE) {
					printf("%12s v%d = *v%lld + v%lld", "mov6", instruction->index_a, instruction->index_r, instruction->index_b);
				}
				else {
					printf("%12s *v%d + v%lld = v%lld", "mov6", instruction->index_r, instruction->index_b, instruction->index_a);
				}
				
				break;
			}
			case BYTECODE_MOVE_A_BY_REFERENCE_PLUS_OFFSET_TO_R: {
				printf("%12s v%d, *v%lld + v%lld", "mov7", instruction->index_r, instruction->index_a, instruction->index_b);
				break;
			}
			case BYTECODE_MOVE_CONSTANT_TO_R_BY_REFERENCE: {
				printf("%12s *v%d = %lld", "mov8", instruction->index_r, instruction->index_a);
				break;
			}
			case BYTECODE_MOVE_CONSTANT_TO_R_PLUS_OFFSET: {
				printf("%12s [v%d + %lld] = %lld", "mov9", instruction->index_r, instruction->index_b, instruction->index_a);
				break;
			}
			case BYTECODE_JUMP: {
				printf("%12s %d", "jump", instruction->index_r + 1);
				break;
			}
			case BYTECODE_JUMP_IF: {
				printf("%12s v%lld == 1, %d", "jump_if", instruction->index_a, instruction->index_r + 1);
				break;
			}
			case BYTECODE_JUMP_IF_NOT: {
				printf("%12s v%lld != 1, %d", "jump_if", instruction->index_a, instruction->index_r + 1);
				break;
			}
			case BYTECODE_PUSH_TO_STACK: {
				printf("%12s v%d", "push", instruction->index_r);
				break;
			}
			case BYTECODE_RETURN: {
				printf("%12s", "return");
				break;
			}
			case BYTECODE_POP_FROM_STACK: {
				printf("%12s v%d", "pop", instruction->index_r);
				break;
			}
			case BYTECODE_ADDRESS_OF: {
				printf("%12s v%d = [v%lld]", "mov", instruction->index_r, instruction->index_a);
				break;
			}			
			case BYTECODE_BINOP_ISEQUAL:
			case BYTECODE_BINOP_ISNOTEQUAL:
			case BYTECODE_BINOP_PLUS:		
			case BYTECODE_BINOP_MINUS:
			case BYTECODE_BINOP_DIV: 
			case BYTECODE_BINOP_MOD:
			case BYTECODE_BINOP_LOGIC_AND:
			case BYTECODE_BINOP_LOGIC_OR:
			case BYTECODE_BINOP_GREATER:
			case BYTECODE_BINOP_GREATEREQUAL:
			case BYTECODE_BINOP_LESS:
			case BYTECODE_BINOP_LESSEQUAL:
			case BYTECODE_BINOP_TIMES: {
				String operation = "";

				if(instruction->instruction == BYTECODE_BINOP_DIV)
					operation = "/";
				else if(instruction->instruction == BYTECODE_BINOP_TIMES)
					operation = "*";
				else if (instruction->instruction == BYTECODE_BINOP_PLUS)
					operation = "+";
				else if (instruction->instruction == BYTECODE_BINOP_MINUS)
					operation = "-";
				else if (instruction->instruction == BYTECODE_BINOP_MOD)
					operation = "%";
				else if (instruction->instruction == BYTECODE_BINOP_ISEQUAL)
					operation = "==";
				else if (instruction->instruction == BYTECODE_BINOP_ISNOTEQUAL)
					operation = "!=";
				else if (instruction->instruction == BYTECODE_BINOP_LOGIC_AND)
					operation = "&&";
				else if (instruction->instruction == BYTECODE_BINOP_LOGIC_OR)
					operation = "||";
				else if (instruction->instruction == BYTECODE_BINOP_GREATER)
					operation = ">";
				else if (instruction->instruction == BYTECODE_BINOP_GREATEREQUAL)
					operation = ">=";
				else if (instruction->instruction == BYTECODE_BINOP_LESS)
					operation = "<";
				else if (instruction->instruction == BYTECODE_BINOP_LESSEQUAL)
					operation = "<=";

				printf("%12s v%d, v%lld ", "binop", instruction->index_r, instruction->index_a);
				printf("%s", operation.data);
				printf(" v%lld ", instruction->index_b);
				break;
			}
			case BYTECODE_INSTRICT_PRINT: {
				printf("%12s v%d", "print", instruction->index_r);
				break;
			}
			case BYTECODE_INSTRICT_ASSERT: {
				printf("%12s v%d != 0", "assert", instruction->index_r);
				break;
			}
			default: {
				printf("%12s %s", "unkown", InstructionNames[instruction->instruction - 1]);
				break;
			}
		}		

		if (instruction->comment != NULL && !instruction->comment.isEmpty()) {
			printf("\t//%s", instruction->comment.data);
		}

		printf("\n");
	}
}