#include "BytecodeDebuger.h"

void BytecodeDebuger::debug() {
	printf("\n");

	for (int i = 0; i < instructions.size(); i++) {
		auto instruction = instructions[i];

		if (instruction->comment != NULL && !instruction->comment.isEmpty()) {
			printf("//%s\n", instruction->comment.data);
		}
		printf("%7d: ", instruction->index_instruction);

		switch (instruction->instruction) {
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
				printf("%12s %s(%s)\n", "call", call->name.data, args.data);
				break;
			}
			case BYTECODE_INTEGER_ADD_TO_CONSTANT: {
				printf("%12s v%d += %I64d\n", "add_int", instruction->index_r, instruction->big_constant._s64);
				break;
			}
			case BYTECODE_ASSING_TO_BIG_CONSTANT: {
				printf("%12s v%d = %I64d\n", "constant", instruction->index_r, instruction->big_constant._s64);
				break;
			}
			case BYTECODE_MOVE_A_TO_R: {
				printf("%12s v%d, v%d\n", "mov", instruction->index_r, instruction->index_a);
				break;
			}
			case BYTECODE_MOVE_A_REGISTER_TO_R: {
				printf("%12s v%d, [v%d]\n", "mov", instruction->index_r, instruction->index_a);
				break;
			}
			case BYTECODE_MOVE_A_BY_REFERENCE_TO_R: {
				printf("%12s v%d, *v%d\n", "mov", instruction->index_r, instruction->index_a);
				break;
			}
			case BYTECODE_JUMP: {
				printf("%12s %d\n", "jump", instruction->index_r);
				break;
			}
			case BYTECODE_JUMP_IF: {
				printf("%12s v%d == 1, %d\n", "jump_if", instruction->index_a, instruction->index_r);
				break;
			}
			case BYTECODE_PUSH_TO_STACK: {
				printf("%12s v%d\n", "push", instruction->index_r);
				break;
			}
			case BYTECODE_RETURN: {
				printf("%12s\n", "return");
				break;
			}
			case BYTECODE_POP_FROM_STACK: {
				printf("%12s v%d\n", "pop", instruction->index_r);
				break;
			}
			case BYTECODE_ADDRESS_OF: {
				printf("%12s v%d = [v%d]\n", "mov", instruction->index_r, instruction->index_a);
				break;
			}			
			case BYTECODE_BINOP_ISEQUAL:
			case BYTECODE_BINOP_ISNOTEQUAL:
			case BYTECODE_BINOP_PLUS:		
			case BYTECODE_BINOP_MINUS:
			case BYTECODE_BINOP_DIV: 
			case BYTECODE_BINOP_MOD:
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

				printf("%12s v%d, v%d %s v%d\n", "binop", instruction->index_r, instruction->index_a, operation.data, instruction->index_b);
				break;
			}
			default: {
				printf("%12s %s\n", "unkown", InstructionNames[instruction->instruction - 1]);
				break;
			}
		}		
	}
}