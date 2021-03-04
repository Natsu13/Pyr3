#include "BytecodeDebuger.h"

void BytecodeDebuger::debug() {
	printf("\n");

	for (int i = 0; i < instructions.size(); i++) {
		auto instruction = instructions[i];

		switch (instruction->instruction) {
		case BYTECODE_INTEGER_ADD_TO_CONSTANT: {
			printf("%12s v%d = v%d + %I64d\n", "constant", instruction->index_o, instruction->index_a, instruction->big_constant._s64);
		}break;
		case BYTECODE_ASSING_TO_BIG_CONSTANT: {
			printf("%12s v%d = %I64d\n", "constant", instruction->index_o, instruction->big_constant._s64);
		}break;
		case BYTECODE_MOVE_A_TO_R: {
			printf("%12s v%d, v%d\n", "mov", instruction->index_o, instruction->index_a);
		}break;
		case BYTECODE_BINOP_PLUS:
		case BYTECODE_BINOP_MINUS:
		case BYTECODE_BINOP_DIV: 
		case BYTECODE_BINOP_TIMES: {
			CString operation = "";

			if(instruction->instruction == BYTECODE_BINOP_DIV)
				operation = "/";
			else if(instruction->instruction == BYTECODE_BINOP_TIMES)
				operation = "*";
			else if (instruction->instruction == BYTECODE_BINOP_PLUS)
				operation = "+";
			else if (instruction->instruction == BYTECODE_BINOP_MINUS)
				operation = "-";

			printf("%12s v%d, v%d %s v%d\n", "binop", instruction->index_o, instruction->index_a, operation.data, instruction->index_b);
		}break;
		default: {
			printf("%12s %s\n", "unkwn", InstructionNames[instruction->instruction - 1]);
		}break;
		}		
	}
}