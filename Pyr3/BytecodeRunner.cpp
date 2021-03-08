#include "BytecodeRunner.h"

BytecodeRunner::BytecodeRunner(Interpret* interpet, vector<ByteCode*> bytecodes, int register_size) {
	this->interpet = interpet;
	this->bytecodes = bytecodes;
	this->registers.reserve(register_size);
	
	for (int i = 0; i < register_size; i++) {
		this->registers.push_back(Register());
	}
}

ByteCode* BytecodeRunner::get_bytecode(int address) {
	if (address < bytecodes.size())
		return bytecodes[address];

	interpet->report_error("bytecode register overload");

	assert(false);
	return NULL;
}

bool BytecodeRunner::is_binop(Bytecode_Instruction bc_inst) {
	if (bc_inst == BYTECODE_BINOP_BITWISE_AND || bc_inst == BYTECODE_BINOP_BITWISE_OR || bc_inst == BYTECODE_BINOP_DIV ||
		bc_inst == BYTECODE_BINOP_GREATER || bc_inst == BYTECODE_BINOP_GREATEREQUAL || bc_inst == BYTECODE_BINOP_ISEQUAL ||
		bc_inst == BYTECODE_BINOP_ISNOTEQUAL || bc_inst == BYTECODE_BINOP_LESS || bc_inst == BYTECODE_BINOP_LESSEQUAL ||
		bc_inst == BYTECODE_BINOP_LOGIC_AND || bc_inst == BYTECODE_BINOP_LOGIC_OR || bc_inst == BYTECODE_BINOP_MINUS ||
		bc_inst == BYTECODE_BINOP_MOD || bc_inst == BYTECODE_BINOP_PLUS || bc_inst == BYTECODE_BINOP_TIMES)
		return true;
	return false;
}

void BytecodeRunner::loop() {
	for (current_address = 0; current_address < bytecodes.size(); current_address++) {
		run_expression(current_address);
	}
	for (int i = 0; i < registers.size(); i++) {
		printf("\nv%d: %15I64d, %15f", i, registers[i]._s64, registers[i]._float);
	}	
}

void BytecodeRunner::run(int address) {
	run_expression(address);
}

int BytecodeRunner::run_expression(int address) {
	auto bc = get_bytecode(address);

	if (bc->instruction == BYTECODE_ASSING_TO_BIG_CONSTANT) {
		this->registers[bc->index_r] = bc->big_constant;
		return bc->index_r;
	}
	if (bc->instruction == BYTECODE_INTEGER_ADD_TO_CONSTANT) {
		this->registers[bc->index_r]._float = (this->registers[bc->index_a]._float + bc->big_constant._float);
		return bc->index_r;
	}
	if (bc->instruction == BYTECODE_MOVE_A_TO_R) {
		this->registers[bc->index_r] = this->registers[bc->index_a];
		return bc->index_r;
	}
	if (is_binop(bc->instruction)) {
		return run_binop(address);
	}

	return 0;
}

int BytecodeRunner::run_binop(int address) {
	auto bc = get_bytecode(address);

	if (bc->instruction == BYTECODE_BINOP_PLUS) {
		this->registers[bc->index_r]._float = this->registers[bc->index_a]._float + this->registers[bc->index_b]._float;
	}
	else if (bc->instruction == BYTECODE_BINOP_MINUS) {
		this->registers[bc->index_r]._float = this->registers[bc->index_a]._float - this->registers[bc->index_b]._float;
	}
	else if (bc->instruction == BYTECODE_BINOP_DIV) {
		this->registers[bc->index_r]._float = this->registers[bc->index_a]._float / this->registers[bc->index_b]._float;
	}
	else if (bc->instruction == BYTECODE_BINOP_TIMES) {
		this->registers[bc->index_r]._float = this->registers[bc->index_a]._float * this->registers[bc->index_b]._float;
	}
	else if (bc->instruction == BYTECODE_BINOP_MOD) {
		this->registers[bc->index_r]._s64 = this->registers[bc->index_a]._s64 % this->registers[bc->index_b]._s64;
	}

	return bc->index_r;
}