#include "BytecodeRunner.h"

#define RegisterVal(name, reg, type) \
	void* name;\
	{\
		 AST_Type_Definition* tdef = (AST_Type_Definition*)type;\
		if (tdef->internal_type == AST_Type_s64) name = (void*)reg._s64;\
		if (tdef->internal_type == AST_Type_float) name = (void*)(int*)&reg._float;\
		if (tdef->internal_type == AST_Type_char) name = (void*)reg._u8;\
		if (tdef->internal_type == AST_Type_string) name = (void*)((New_String*)reg._pointer)->data;\
		if (tdef->internal_type == AST_Type_pointer) name = (void*)reg._pointer;\
	}\

void* CallDllFunction(HMODULE hModule, const char* fun, vector<Register> regs, vector<AST_Type*> types) {
	if (regs.size() == 0) {
		FARPROC funci = GetProcAddress(hModule, fun);
		return (void*)funci();
	}
	else if (regs.size() == 1) {
		RegisterVal(a1, regs[0], types[0]);
		auto tuple = make_tuple(a1);
		return (void*)DLLCALL::DLLCall<void*>(hModule, fun, tuple);
	}
	else if (regs.size() == 2) {
		RegisterVal(a1, regs[0], types[0]);
		RegisterVal(a2, regs[1], types[1]);

		auto tuple = make_tuple(a1, a2);
		return (void*)DLLCALL::DLLCall<void*>(hModule, fun, tuple);
	}
	else if (regs.size() == 3) {
		RegisterVal(a1, regs[0], types[0]);
		RegisterVal(a2, regs[1], types[1]);
		RegisterVal(a3, regs[2], types[2]);
		auto tuple = make_tuple(a1, a2, a3);
		return (void*)DLLCALL::DLLCall<void*>(hModule, fun, tuple);
	}
	else if (regs.size() == 4) {
		RegisterVal(a1, regs[0], types[0]);
		RegisterVal(a2, regs[1], types[1]);
		RegisterVal(a3, regs[2], types[2]);
		RegisterVal(a4, regs[3], types[3]);
		auto tuple = make_tuple(a1, a2, a3, a4);
		return (void*)DLLCALL::DLLCall<void*>(hModule, fun, tuple);
	}
	else if (regs.size() == 5) {
		RegisterVal(a1, regs[0], types[0]);
		RegisterVal(a2, regs[1], types[1]);
		RegisterVal(a3, regs[2], types[2]);
		RegisterVal(a4, regs[3], types[3]);
		RegisterVal(a5, regs[4], types[4]);
		auto tuple = make_tuple(a1, a2, a3, a4, a5);
		return (void*)DLLCALL::DLLCall<void*>(hModule, fun, tuple);
	}
	else if (regs.size() == 6) {
		RegisterVal(a1, regs[0], types[0]);
		RegisterVal(a2, regs[1], types[1]);
		RegisterVal(a3, regs[2], types[2]);
		RegisterVal(a4, regs[3], types[3]);
		RegisterVal(a5, regs[4], types[4]);
		RegisterVal(a6, regs[5], types[5]);

		auto tuple = make_tuple(a1, a2, a3, a4, a5, a6);
		return (void*)DLLCALL::DLLCall<void*>(hModule, fun, tuple);
	}
	else if (regs.size() == 12) {
		RegisterVal(a1, regs[0], types[0]);
		RegisterVal(a2, regs[1], types[1]);
		RegisterVal(a3, regs[2], types[2]);
		RegisterVal(a4, regs[3], types[3]);
		RegisterVal(a5, regs[4], types[4]);
		RegisterVal(a6, regs[5], types[5]);
		RegisterVal(a7, regs[6], types[6]);
		RegisterVal(a8, regs[7], types[7]);
		RegisterVal(a9, regs[8], types[8]);
		RegisterVal(a10, regs[9], types[9]);
		RegisterVal(a11, regs[10], types[10]);
		RegisterVal(a12, regs[11], types[11]);

		auto tuple = make_tuple(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
		return (void*)DLLCALL::DLLCall<void*>(hModule, fun, tuple);
	}

	abort();
}

BytecodeRunner::BytecodeRunner(Interpret* interpret, vector<ByteCode*> bytecodes, vector<AST_Type*> bytecode_types, int register_size, int memory_size) {
	this->interpret = interpret;
	this->bytecodes = bytecodes;
	this->bytecode_types = bytecode_types;

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

HMODULE BytecodeRunner::get_hmodule(const char* name) {
	for (int i = 0; i < foreignLibrary.size(); i++) {
		auto lib = foreignLibrary[i];
		if (COMPARE(name, lib.name)) {
			return lib.mod;
		}
	}

	auto lib = ForeignLibrary{ name, LoadLibraryA(name) };
	foreignLibrary.push_back(lib);

	return lib.mod;
}

#define ASSIGN_TO_REGISTER_BY_TYPE_WITH_OFFSET(index, pointer, offset) \
	{\
		auto type = (AST_Type_Definition*)bytecode_types[pointer];\
		if(type->internal_type == AST_Type_string) { New_String* pos = (New_String*)this->registers[pointer]._pointer; this->registers[index]._u8 = pos->data[offset]; }\
		else { \
			void* pos = (int8_t*)this->registers[pointer]._pointer + (offset); \
			this->registers[index]._s64 = *(int64_t*)pos; \
		}\
	}

int BytecodeRunner::run_expression(int address) {
	auto bc = get_bytecode(address);

	auto oplplp = this->registers[4]._pointer;

	switch (bc->instruction) {
		case BYTECODE_INSTRICT_ASSERT: {
			auto x = this->registers[bc->index_r]._s64;			
			assert(this->registers[bc->index_r]._s64 == 1);
			return 0;
		}
		case BYTECODE_INSTRICT_PRINT: {
			auto x = this->registers[bc->index_r];
			//auto type = (AST_Type_Definition*)bytecode_types[bc->index_r];
			//auto c = (New_String*)this->registers[bc->index_r]._pointer;
			printf("\n%lld", this->registers[bc->index_r]._s64);
			//printf("\n%c", (char)this->registers[bc->index_r]._u8);
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
		case BYTECODE_MOVE_A_BY_REFERENCE_PLUS_OFFSET_TO_R: {			
			//auto type = (AST_Type_Definition*)bytecode_types[bc->index_a];
			ASSIGN_TO_REGISTER_BY_TYPE_WITH_OFFSET(bc->index_r, bc->index_a, this->registers[bc->index_b]._s64);
			/*
			if (type->internal_type == AST_Type_string) {
				New_String* pos = (New_String*)this->registers[bc->index_a]._pointer;
				this->registers[bc->index_r]._u8 = pos->data[this->registers[bc->index_b]._s64];
			}
			else {
				void* pos = (int8_t*)this->registers[bc->index_a]._pointer + (this->registers[bc->index_b]._s64);
				this->registers[bc->index_r]._s64 = *(int64_t*)pos;
			}*/
			//printf("\nmov v%d = %lld (*v%d + %lld)", bc->index_r, this->registers[bc->index_r]._s64, bc->index_a, this->registers[bc->index_b]._s64);

			return bc->index_r;
		}
		case BYTECODE_MOVE_A_REGISTER_TO_R:{
			this->registers[bc->index_r] = this->registers[this->registers[bc->index_a]._s64];
			return bc->index_r;
		}
		case BYTECODE_MOVE_A_TO_R_PLUS_OFFSET: {
			void* pos = (int8_t*)this->registers[bc->index_r]._pointer + bc->index_b;
			//pos = &this->registers[bc->index_a]._s64;
			memcpy(pos, &this->registers[bc->index_a]._s64, sizeof(this->registers[bc->index_a]._s64));
			return bc->index_r;
		}
		case BYTECODE_MOVE_A_TO_R_PLUS_OFFSET_REG: {
			//printf("\nmov *v%d + %lld = %lld", bc->index_r, this->registers[bc->index_b]._s64, this->registers[bc->index_a]._s64);
			void* pos = (int8_t*)this->registers[bc->index_r]._pointer + this->registers[bc->index_b]._s64;
			//void* pos = &this->registers[bc->index_r]._pointer + this->registers[bc->index_b]._s64;
			memcpy(pos, &this->registers[bc->index_a]._s64, sizeof(this->registers[bc->index_a]._s64));	

			return bc->index_r;
		}
		case BYTECODE_MOVE_A_PLUS_OFFSET_TO_R: {
			void* pos = (int8_t*)this->registers[bc->index_a]._pointer + bc->index_b;
			this->registers[bc->index_r] = *(static_cast<Register*>(pos));
			return bc->index_r;
		}
		case BYTECODE_RESERVE_MEMORY_TO_R: {
			this->registers[bc->index_r]._pointer = malloc(bc->index_a);
			auto xox = this->registers[bc->index_r]._pointer;
			memset(this->registers[bc->index_r]._pointer, 0, bc->index_a);//fill it with NULL
			auto xcasf = this->registers[4]._pointer;
			return bc->index_r;
		}

		case BYTECODE_CALL_PROCEDURE: {
			Register raddr = Register();
			raddr._s64 = current_address;

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

			vector<Register> regs;
			vector<AST_Type*> types;
			for (int i = 0; i < procedure->arguments.size(); i++) {
				auto arg = procedure->arguments[i];
				auto reg = this->registers[arg->bytecode_address];

				if (procedure->arguments[i]->type == AST_IDENT) {
					AST_Ident* ident = (AST_Ident*)procedure->arguments[i];
					
					if (ident->type_declaration != NULL && ident->type_declaration->inferred_type->kind == AST_TYPE_STRUCT) {
						AST_Struct* strct = (AST_Struct*)ident->type_declaration->inferred_type;
						Register copy_reg = Register();
						copy_reg._pointer = malloc(strct->size);
						memcpy(copy_reg._pointer, reg._pointer, strct->size);
						reg = copy_reg;
						//Copy the struct so wee don't give to the function reference but only copy of it!
					}
				}
				//auto xx = this->registers[arg->bytecode_address]._pointer;
				regs.push_back(reg);
				types.push_back(this->bytecode_types[arg->bytecode_index]);
			}
			
			if (procedure->procedure->flags & AST_PROCEDURE_FLAG_FOREIGN) {
				auto lit = static_cast<AST_Literal*>(procedure->procedure->foreign_library_expression);				
				auto result = CallDllFunction(get_hmodule(lit->string_value.data), procedure->name.data, regs, types);

				if (procedure->procedure->returnType != NULL) {
					auto type = static_cast<AST_Type_Definition*>(bytecode_types[procedure->return_register]);
					if (type->internal_type == AST_Type_pointer) {
						registers[procedure->return_register]._pointer = result;
					}
					else if (type->internal_type == AST_Type_string) {
						auto str = String((const char*)result);
						registers[procedure->return_register]._pointer = new New_String{ str.data, str.size };
					}
					else {
						registers[procedure->return_register]._s64 = (int64_t)result;
					}

					stack.push_back(registers[procedure->return_register]);
				}
				return 0;
			}

			addressStack.push_back(raddr);
			for (int i = 0; i < regs.size(); i++) {
				stack.push_back(regs[i]);
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
			auto xo = this->registers[bc->index_r]._pointer;
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
		//printf("\n%lld (v%d) * %lld (v%d) = %lld", this->registers[bc->index_a]._s64, bc->index_a, this->registers[bc->index_b]._s64, bc->index_b, this->registers[bc->index_r]._s64);
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