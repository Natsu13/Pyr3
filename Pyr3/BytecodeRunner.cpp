#include "BytecodeRunner.h"

BytecodeRunner::BytecodeRunner(Interpret* interpret, vector<ByteCode*> bytecodes, int register_size, int memory_size) {
	this->interpret = interpret;
	this->bytecodes = bytecodes;
	this->registers.reserve(register_size);
	
	for (int i = 0; i < register_size; i++) {
		this->registers.push_back(Register());
	}

	current_address = 0;
}

void BytecodeRunner::set_current_address(int address) {
	assert(address >= 0);
	assert(address <= bytecodes.size());

	current_address = address;
}

ByteCode* BytecodeRunner::get_bytecode(int address) {
	if (address < bytecodes.size())
		return bytecodes[address];

	interpret->report_error("bytecode register overload");

	assert(false);
	return NULL;
}

int BytecodeRunner::get_address_of_procedure(String procedure_name, AST_Block* block) {
	for (int i = 0; i < block->expressions.size(); i++) {
		auto expr = block->expressions[i];

		if (expr->type != AST_DECLARATION) continue;

		auto declaration = static_cast<AST_Declaration*>(expr); 
		if (declaration->value->type != AST_PROCEDURE) continue;

		if (declaration->ident->name->value == procedure_name)
			return declaration->value->bytecode_address;
	}

	if (block->flags & AST_BLOCK_FLAG_MAIN_BLOCK) {
		assert(false && "Can't find address of procedure");
		return 0;
	}

	return get_address_of_procedure(procedure_name, block->scope);
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
	for (; current_address < bytecodes.size(); current_address++) {
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

	switch (bc->instruction) {
		case BYTECODE_ASSING_TO_BIG_CONSTANT: {
			this->registers[bc->index_r] = bc->big_constant;
			return bc->index_r;
		}
		case BYTECODE_INTEGER_ADD_TO_CONSTANT: {
			this->registers[bc->index_r]._s64 = (this->registers[bc->index_r]._s64 + bc->big_constant._s64);
			return bc->index_r;
		}

		case BYTECODE_MOVE_A_TO_R: {
			this->registers[bc->index_r] = this->registers[bc->index_a];
			return bc->index_r;
		}
		case BYTECODE_MOVE_A_BY_REFERENCE_TO_R: {
			this->registers[bc->index_r] = *(static_cast<Register*>(this->registers[bc->index_a]._pointer));
			return bc->index_r;
		}
		case BYTECODE_MOVE_A_REGISTER_TO_R:{
			this->registers[bc->index_r] = this->registers[this->registers[bc->index_a]._s64];
			return 0;
		}

		case BYTECODE_CALL_PROCEDURE: {
			auto reg = new Register();
			reg->_s64 = current_address;
			stack.push_back(*reg);//Store return address

			auto procedure = static_cast<Call_Record*>(bc->big_constant._pointer);

			for (int i = 0; i < procedure->arguments.size(); i++) {
				auto arg = procedure->arguments[i];
				stack.push_back(this->registers[arg->bytecode_address]);
			}
			
			current_address = procedure->procedure->bytecode_address - 1; //je to for kde se na konci pøièítá adresa

			return 0;
		}

		case BYTECODE_RETURN: {
			// return to prev address from stack address register

			//load address of call record and replace address
			auto reg = stack.front();
			current_address = reg._s64;
			stack.erase(stack.begin());

			return 0;
		}

		case BYTECODE_ADDRESS_OF: {
			this->registers[bc->index_r]._pointer = &(this->registers[bc->index_a]);
			return bc->index_r;
		}
		case BYTECODE_PUSH_TO_STACK: {
			stack.push_back(this->registers[bc->index_r]);
			return 0;
		}

		case BYTECODE_POP_FROM_STACK: {
			assert(stack.size() > 0);

			this->registers[bc->index_r] = stack.back();
			stack.pop_back();
			return 0;
		}		
	}

	if (is_binop(bc->instruction)) {
		return run_binop(address);
	}

	return 0;
}

int BytecodeRunner::run_binop(int address) {
	auto bc = get_bytecode(address);

	if (bc->instruction == BYTECODE_BINOP_PLUS) {
		this->registers[bc->index_r]._s64 = this->registers[bc->index_a]._s64 + this->registers[bc->index_b]._s64;
	}
	else if (bc->instruction == BYTECODE_BINOP_MINUS) {
		this->registers[bc->index_r]._s64 = this->registers[bc->index_a]._s64 - this->registers[bc->index_b]._s64;
	}
	else if (bc->instruction == BYTECODE_BINOP_DIV) {
		this->registers[bc->index_r]._s64 = this->registers[bc->index_a]._s64 / this->registers[bc->index_b]._s64;
	}
	else if (bc->instruction == BYTECODE_BINOP_TIMES) {
		this->registers[bc->index_r]._s64 = this->registers[bc->index_a]._s64 * this->registers[bc->index_b]._s64;
	}
	else if (bc->instruction == BYTECODE_BINOP_MOD) {
		this->registers[bc->index_r]._s64 = this->registers[bc->index_a]._s64 % this->registers[bc->index_b]._s64;
	}
	else if (bc->instruction == BYTECODE_BINOP_ISEQUAL) {
		this->registers[bc->index_r]._u8 = this->registers[bc->index_a]._s64 == this->registers[bc->index_b]._s64;
	}
	else if (bc->instruction == BYTECODE_BINOP_ISNOTEQUAL) {
		this->registers[bc->index_r]._u8 = this->registers[bc->index_a]._s64 != this->registers[bc->index_b]._s64;
	}
	else if (bc->instruction == BYTECODE_BINOP_GREATER) {
		this->registers[bc->index_r]._u8 = this->registers[bc->index_a]._s64 > this->registers[bc->index_b]._s64;
	}
	else if (bc->instruction == BYTECODE_BINOP_GREATEREQUAL) {
		this->registers[bc->index_r]._u8 = this->registers[bc->index_a]._s64 >= this->registers[bc->index_b]._s64;
	}
	else if (bc->instruction == BYTECODE_BINOP_LESS) {
		this->registers[bc->index_r]._u8 = this->registers[bc->index_a]._s64 < this->registers[bc->index_b]._s64;
	}
	else if (bc->instruction == BYTECODE_BINOP_LESSEQUAL) {
		this->registers[bc->index_r]._u8 = this->registers[bc->index_a]._s64 <= this->registers[bc->index_b]._s64;
	}

	return bc->index_r;
}