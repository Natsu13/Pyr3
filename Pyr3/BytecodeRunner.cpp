#include "BytecodeRunner.h"

BytecodeRunner::BytecodeRunner(Interpret* interpret, vector<ByteCode*> bytecodes, int register_size, int memory_size) {
	this->interpret = interpret;
	this->bytecodes = bytecodes;

	this->registers = Array<Register>();
	this->registers.reserve(register_size * 10);
	
	for (int i = 0; i < register_size + 1; i++) { 
		//hack + 1 because else it throw illegal address access idk why? only happend when alocated struct and then 
		//push value to first position if first is empty then it's fine.... c++ pls
		auto x = new Register();
		this->registers.push_back(x);
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
	/*
	for (int i = 0; i < registers.size() - 1; i++) {
		printf("\nv%d: %15I64d, %15f", i, registers[i]._s64, registers[i]._float);
	}
	

	printf("\nv%d: %15I64d, %15f", 6, registers[6]._s64, registers[6]._float);
	printf("\nv%d: %15I64d, %15f", 4, registers[4]._s64, registers[4]._float);
	*/
}

void BytecodeRunner::run(int address) {
	run_expression(address);
}

int BytecodeRunner::run_expression(int address) {
	auto bc = get_bytecode(address);

	switch (bc->instruction) {
		case BYTECODE_INSTRICT_ASSERT: {
			auto x = this->registers[bc->index_r]._s64;
			assert(this->registers[bc->index_r]._s64 == 1);
			return 0;
		}
		case BYTECODE_INSTRICT_PRINT: {
			auto x = this->registers[bc->index_r];
			printf("\n%d", this->registers[bc->index_r]._s64);
			return 0;
		}
		case BYTECODE_ASSING_TO_BIG_CONSTANT: {
			this->registers[bc->index_r] = bc->big_constant;
			return bc->index_r;
		}
		case BYTECODE_INTEGER_ADD_TO_CONSTANT: {
			this->registers[bc->index_r]._s64 = (this->registers[bc->index_r]._s64 + bc->big_constant._s64);
			return bc->index_r;
		}

		case BYTECODE_JUMP: {
			current_address = bc->index_r - 1;
			return 0;
		}
		case BYTECODE_JUMP_IF: {
			if (this->registers[bc->index_a]._s64) {
				current_address = bc->index_r - 1;
			}

			return 0;
		}
		case BYTECODE_JUMP_IF_NOT: {
			if (!this->registers[bc->index_a]._s64) {
				current_address = bc->index_r - 1;
			}

			return 0;
		}

		case BYTECODE_MOVE_A_TO_R: {
			//printf("\n  v%d <= %lld(%d)", bc->index_r, this->registers[bc->index_a]._s64, bc->index_a);
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
		case BYTECODE_MOVE_A_TO_R_PLUS_OFFSET: {
			void* pos = &this->registers[bc->index_r]._pointer + bc->index_b;
			//pos = (void*)this->registers[bc->index_a]._s64;
			memcpy(pos, &this->registers[bc->index_a]._s64, sizeof(this->registers[bc->index_a]._s64));
			return 0;
		}
		case BYTECODE_MOVE_A_PLUS_OFFSET_TO_R: {
			void* pos = &this->registers[bc->index_a]._pointer + bc->index_b;
			this->registers[bc->index_r] = *(static_cast<Register*>(pos));
			return bc->index_r;
		}
		case BYTECODE_RESERVE_MEMORY_TO_R: {
			this->registers[bc->index_r]._pointer = malloc(bc->index_a);
			auto owo = this->registers[bc->index_r]._pointer;
			return bc->index_r;
		}

		case BYTECODE_CALL_PROCEDURE: {
			Register raddr = Register();
			raddr._s64 = current_address;
			addressStack.push_back(raddr);

			auto procedure = static_cast<Call_Record*>(bc->big_constant._pointer);
			if (procedure->calcualted_offset == -1) {
				procedure->calcualted_offset = procedure->offset - procedure->procedure->bytecode_index;
			}
			if (procedure->calcualted_offset < 0) {
				procedure->calcualted_offset = 0;
			}

			/*Register raddr = Register();
			raddr._s64 = procedure->calcualted_offset;
			addressStack.push_back(raddr);*/

			for (int i = 0; i < procedure->arguments.size(); i++) {
				auto arg = procedure->arguments[i];
				stack.push_back(this->registers[arg->bytecode_address]);
			}
			
			Stack_Record* st = new Stack_Record();
			st->start_index = procedure->procedure->bytecode_index;
			for (int i = st->start_index; i <= st->start_index + procedure->calcualted_offset; i++) {
				st->registers.push_back(this->registers[i]);
				//printf("\n  <== %lld(v%d)", this->registers[i]._s64, i);
			}
			Register rstack = Register();
			rstack._pointer = st;
			addressStack.push_back(rstack);

			//this->registers.set_offset(this->registers.get_offset() + procedure->calcualted_offset);
			//printf("\n ---- >OFFSET %d -----", this->registers.get_offset());
			
			current_address = procedure->procedure->bytecode_address - 1; //je to for kde se na konci pøièítá adresa

			return 0;
		}

		case BYTECODE_RETURN: {
			// return to prev address from stack address register

			//load address of call record and replace address
			if (addressStack.size() == 0) {
				current_address = bytecodes.size();
				//return from entry function to end of the program
			}
			else {
				auto reg = addressStack.back();
				/*if (reg > 0 && this->registers.get_offset() > 0) {					
					this->registers.set_offset(this->registers.get_offset() - reg);
				}*/
				Stack_Record* srec = (Stack_Record*)reg._pointer;
				for (int i = 0; i < srec->registers.size(); i++) {
					auto xxxx = srec->registers[i];
					this->registers[srec->start_index + i] = srec->registers[i];

					//printf("\n  ==> %lld(v%d)", this->registers[srec->start_index + i]._s64, srec->start_index + i);
				}
				addressStack.pop_back();
				//printf("\n ---- <OFFSET %d -----", this->registers.get_offset());

				reg = addressStack.back();
				current_address = reg._s64;
				addressStack.pop_back();				
			}

			return 0;
		}

		case BYTECODE_ADDRESS_OF: {
			this->registers[bc->index_r]._pointer = &(this->registers[bc->index_a]);
			return bc->index_r;
		}
		case BYTECODE_PUSH_TO_STACK: {			
			//printf("\n  ->[PUSH] %lld(v%d)", this->registers[bc->index_r]._s64, bc->index_r);
			stack.push_back(this->registers[bc->index_r]);
			return 0;
		}

		case BYTECODE_POP_FROM_STACK: {
			assert(stack.size() > 0);
			this->registers[bc->index_r] = stack.back();
			//printf("\n  <-[POP] %lld(v%d)", this->registers[bc->index_r]._s64, bc->index_r);
			auto xo = this->registers[bc->index_r]._s64;
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

	this->registers[bc->index_r]._u64 = 0;

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
		//printf("\n%lld * %lld = %lld", this->registers[bc->index_a]._s64, this->registers[bc->index_b]._s64, this->registers[bc->index_r]._s64);
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
		//printf("\n%lld > %lld = %lld", this->registers[bc->index_a]._s64, this->registers[bc->index_b]._s64, this->registers[bc->index_r]._s64);
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
	else if (bc->instruction == BYTECODE_BINOP_LOGIC_AND) {
		this->registers[bc->index_r]._u8 = this->registers[bc->index_a]._s64 && this->registers[bc->index_b]._s64;
	}
	else if (bc->instruction == BYTECODE_BINOP_LOGIC_OR) {
		this->registers[bc->index_r]._u8 = this->registers[bc->index_a]._s64 || this->registers[bc->index_b]._s64;
	}
	else {
		assert(false);
	}

	return bc->index_r;
}