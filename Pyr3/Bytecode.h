#pragma once
#include "Interpret.h"
#include "String.h"

const int INSTRUCTION_COUNT = 29;

enum Bytecode_Instruction {
	BYTECODE_UNINITIALIZED = 0,
	BYTECODE_NOOP = 1,
	BYTECODE_ASSING_TO_BIG_CONSTANT = 2,
	BYTECODE_INTEGER_ADD_TO_CONSTANT = 3,

	//This all will have option flag soo one instruction
	BYTECODE_MOVE_A_TO_R = 4,
	BYTECODE_MOVE_A_REGISTER_TO_R = 23,
	BYTECODE_MOVE_A_PLUS_OFFSET_TO_R = 33,
	BYTECODE_MOVE_A_BY_REFERENCE_PLUS_OFFSET_TO_R = 35,
	BYTECODE_MOVE_A_TO_R_PLUS_OFFSET = 31,
	BYTECODE_MOVE_A_TO_R_PLUS_OFFSET_REG = 34,
	BYTECODE_MOVE_A_BY_REFERENCE_TO_R = 28,
	BYTECODE_MOVE_CONSTANT_TO_R_BY_REFERENCE = 61,
	BYTECODE_MOVE_CONSTANT_TO_R_PLUS_OFFSET = 62,

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
	BYTECODE_JUMP_IF_NOT = 32,

	BYTECODE_RESERVE_STACK = 22,
	BYTECODE_PUSH_TO_STACK = 24,
	BYTECODE_POP_FROM_STACK = 25,
	BYTECODE_CALL_PROCEDURE = 26,

	BYTECODE_ADDRESS_OF = 27,	
	BYTECODE_RETURN = 29,
	BYTECODE_RESERVE_MEMORY_TO_R = 30,
	BYTECODE_RESERVE_MEMORY_TO_R_PLUS_OFFSET = 60,

	BYTECODE_INSTRICT_PRINT = 50,
	BYTECODE_INSTRICT_ASSERT = 51,

	BYTECODE_CAST = 52,
	BYTECODE_C_CALL_FROM_PROCEDURE = 53,	

	BYTECODE_EMIT_TYPE = 54
};

const char* InstructionNames[];

enum Bytecode_Instruction_Options {
	OPTION_RESERVE_MEMORY_TO_R_BY_ADDRESS = 0x100,
	OPTION_MOVE_UNBOX = 0x110,
	OPTION_MOVE_A_TO_R_PLUS_OFFSET_REG_NOREVERSE = 0x111
};

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
			int64_t index_a;
			int64_t index_b;
		};

		Register big_constant;
	};
	
	int index_r = -1;
	int options = 0;
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
	int offset = 0;
	int calcualted_offset = -1;
	int return_register = -1;
};

struct Stack_Record {
	int start_index = 0;
	vector<Register> registers;
};