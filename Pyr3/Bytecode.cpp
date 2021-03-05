#pragma once
#include "Bytecode.h"

const char* InstructionNames[INSTRUCTION_COUNT] = {
	"BYTECODE_UNINITIALIZED",
	"BYTECODE_NOOP",
	"BYTECODE_ASSING_TO_BIG_CONSTANT",
	"BYTECODE_INTEGER_ADD_TO_CONSTANT",
	"BYTECODE_MOVE_A_TO_R",
	"BYTECODE_BINOP_PLUS",
	"BYTECODE_BINOP_MINUS"
	"BYTECODE_BINOP_TIMES",
	"BYTECODE_BINOP_DIV",
	"BYTECODE_BINOP_MOD",
	"BYTECODE_BINOP_ISEQUAL",
	"BYTECODE_BINOP_ISNOTEQUAL",
	"BYTECODE_BINOP_GREATER",
	"BYTECODE_BINOP_GREATEREQUAL",
	"BYTECODE_BINOP_LESS",
	"BYTECODE_BINOP_LESSEQUAL",
	"BYTECODE_BINOP_LOGIC_AND",
	"BYTECODE_BINOP_LOGIC_OR",
	"BYTECODE_BINOP_BITWISE_AND",
	"BYTECODE_BINOP_BITWISE_OR",
};