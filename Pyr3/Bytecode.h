#pragma once
#include "Interpret.h"
#include "String.h"

const int INSTRUCTION_COUNT = 29;

enum Bytecode_Instruction {
	BYTECODE_UNINITIALIZED = 0,
	BYTECODE_NOOP = 1,
	BYTECODE_ASSING_TO_BIG_CONSTANT = 2,
	BYTECODE_INTEGER_ADD_TO_CONSTANT = 3,
	BYTECODE_MOVE_A_TO_R = 4,
	BYTECODE_MOVE_A_REGISTER_TO_R = 23,

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

	BYTECODE_JUMP_IF = 20,
	BYTECODE_JUMP = 21,

	BYTECODE_RESERVE_STACK = 22,
	BYTECODE_PUSH_TO_STACK = 24,
	BYTECODE_POP_FROM_STACK = 25,
	BYTECODE_CALL_PROCEDURE = 26,

	BYTECODE_ADDRESS_OF = 27,
	BYTECODE_MOVE_A_BY_REFERENCE_TO_R = 28,
	BYTECODE_RETURN = 29
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

		void* _pointer;
	};
};

struct ByteCode {
	Bytecode_Instruction instruction = BYTECODE_UNINITIALIZED;

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

	int index_instruction = -1;

	AST_Expression* debug_expression = NULL;
	int line_number = 0;

	String comment;
	int serial = 0;
};

struct Call_Record {
	String name = NULL;
	AST_Procedure* procedure = NULL;
	vector<AST_Expression*> arguments;
};