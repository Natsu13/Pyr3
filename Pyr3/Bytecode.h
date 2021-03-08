#pragma once
#include "Interpret.h"

const int INSTRUCTION_COUNT = 19;

enum Bytecode_Instruction {
	BYTECODE_UNINITIALIZED = 0,
	BYTECODE_NOOP = 1,
	BYTECODE_ASSING_TO_BIG_CONSTANT = 2,
	BYTECODE_INTEGER_ADD_TO_CONSTANT = 3,
	BYTECODE_MOVE_A_TO_R = 4,

	BYTECODE_BINOP_PLUS = 5,
	BYTECODE_BINOP_MINUS = 6,
	BYTECODE_BINOP_TIMES = 7,
	BYTECODE_BINOP_DIV = 8,
	BYTECODE_BINOP_MOD = 9,
	BYTECODE_BINOP_ISEQUAL = 10,
	BYTECODE_BINOP_ISNOTEQUAL = 11,
	BYTECODE_BINOP_GREATER = 12,
	BYTECODE_BINOP_GREATEREQUAL = 13,
	BYTECODE_BINOP_LESS = 14,
	BYTECODE_BINOP_LESSEQUAL = 15,
	BYTECODE_BINOP_LOGIC_AND = 16,
	BYTECODE_BINOP_LOGIC_OR = 17,
	BYTECODE_BINOP_BITWISE_AND = 18,
	BYTECODE_BINOP_BITWISE_OR = 19,

};

const char* InstructionNames[];

struct Register {
	union {
		uint8_t  _u8;
		uint16_t _u16;
		uint32_t _u32;
		uint64_t _u64;

		int8_t  _s8;
		int16_t _s16;
		int32_t _s32;
		int64_t _s64;

		float _float;

		char* _pointer;
	};
};

struct ByteCode {
	Bytecode_Instruction instruction = BYTECODE_UNINITIALIZED;

	/*ByteCode(int i, int a, int b, int o) :instruction(i), index_r(o) {
		this->index_a = 0;
		this->index_b = 0;

		if (a != -1) {
			this->index_a = a;
		}
		if (b != -1) {
			this->index_b = b;
		}
	}*/

	union {
		struct {
			int index_a;
			int index_b;
		};

		Register big_constant;
	};
	
	int index_r = -1;
	int options = -1;
	int flags;

	AST_Expression* debug_expression = NULL;
	int line_number = 0;
};