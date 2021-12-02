#include "BytecodeRunner.h"

#define RegisterVal(name, reg, type) \
	void* name = NULL;\
	{\
		if(type->kind == AST_TYPE_DEFINITION) { \
			AST_Type_Definition* tdef = (AST_Type_Definition*)type; \
			if (tdef->internal_type == AST_Type_s64) name = (void*)reg._s64; \
			else if (tdef->internal_type == AST_Type_float) name = (void*)(int*)&reg._float; \
			else if (tdef->internal_type == AST_Type_char) name = (void*)reg._u8; \
			else if (tdef->internal_type == AST_Type_string) name = (void*)((New_String*)reg._pointer)->data; \
			else if (tdef->internal_type == AST_Type_pointer) name = (void*)reg._pointer; \
			else if (tdef->internal_type == AST_Type_struct) name = (void*)reg._pointer; \
		} else if(type->kind == AST_TYPE_STRUCT || type->kind == AST_TYPE_POINTER){ \
			name = (void*)reg._pointer;\
		}\
	}\

void* CallDllFunction(HMODULE hModule, const char* fun, vector<Register> regs, vector<AST_Type*> types) {
	if (regs.size() == 0) {
		FARPROC funci = GetProcAddress(hModule, fun);
		return (void*)funci();
	}
	else if (regs.size() == 1) {
		RegisterVal(a1, regs[0], types[0]);

		/*
		WNDCLASSEXA* x = (WNDCLASSEXA*)a1;

		if (x != NULL) {
			try {
				//auto fr = ((FunRegister*)(x->lpfnWndProc));
				//void* xxx = (void*)fr;
				//auto xx = (*fr)((void*)NULL, (void*)154, (void*)1, (void*)845);

				//void* qqq = ((FunRegister*)(x->lpfnWndProc));
				//x->lpfnWndProc = qqq;
				//auto cql = qqq(NULL, 0, 0, 0);
				//auto cal = ((FunRegister*)(x->lpfnWndProc))->_function(NULL, 0, 0, 0);
				//auto col = x->lpfnWndProc(NULL, 0, 0, 0);

				//x->lpfnWndProc(NULL, 52, 45, 93);
			}
			catch (...) {
				auto ooo = 5;
			}
		}*/

		WNDCLASSEXA* x = (WNDCLASSEXA*)a1;

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

BytecodeRunner::BytecodeRunner(Interpret* interpret, vector<ByteCode*> bytecodes, vector<AST_Type*> types, int register_size, int memory_size) {
	this->interpret = interpret;
	this->bytecodes = bytecodes;
	this->types = types;

	this->registers = vector<Register>();
	this->registers.reserve(register_size * 10);
	
	for (int i = 0; i < register_size + 1; i++) { 
		//hack + 1 because else it throw illegal address access idk why? only happend when alocated struct and then 
		//push value to first position if first is empty then it's fine.... c++ pls
		//auto x = Register();
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
//r,a,offset number //33,16,8
#define ASSIGN_TO_REGISTER_BY_TYPE_WITH_OFFSET(index, pointer, offset) \
	{\
		/*auto type = get_type((AST_Type*)types[index]);*/ \
		auto type = (AST_Type*)types[index]; \
		void* pos = (int8_t*)this->registers[pointer]._pointer;\
		auto type_out = (AST_Type*)types[pointer]; \
		auto output = pos;\
		if (type_out->kind == AST_TYPE_POINTER) {\
			output = *((void**)pos); \
		}\
		output = (int8_t*)output + offset;\
		if(type->kind == AST_TYPE_DEFINITION) { \
			auto tdef = (AST_Type_Definition*)type;\
			if(tdef->internal_type == AST_Type_string) { \
				New_String* pos = (New_String*)this->registers[pointer]._pointer; \
				this->registers[index]._u8 = pos->data[offset]; \
			} /*\
			else if(tdef->internal_type == AST_Type_s8) this->registers[index]._s8 = *(int8_t*)pos;\
			else if(tdef->internal_type == AST_Type_s16) this->registers[index]._s16 = *(int16_t*)pos;\
			else if(tdef->internal_type == AST_Type_s32) this->registers[index]._s32 = *(int32_t*)pos;\
			else if(tdef->internal_type == AST_Type_s64) this->registers[index]._s64 = *(int64_t*)pos;\
			else if(tdef->internal_type == AST_Type_u8) this->registers[index]._u8 = *(uint8_t*)pos;\
			else if(tdef->internal_type == AST_Type_u16) this->registers[index]._u16 = *(uint16_t*)pos;\
			else if(tdef->internal_type == AST_Type_u32) this->registers[index]._u32 = *(uint32_t*)pos;\
			else if(tdef->internal_type == AST_Type_u64) this->registers[index]._u64 = *(uint64_t*)pos;\
			else if(tdef->internal_type == AST_Type_char) this->registers[index]._u32 = *(uint32_t*)pos;\
			else if(tdef->internal_type == AST_Type_float) this->registers[index]._float = *(float*)pos;\
			*/\
			else { this->registers[index]._s64 = *(int64_t*)output; }\
			/*else this->registers[index]._pointer = pos;*/\
		} else if(type->kind == AST_TYPE_POINTER || type->kind == AST_TYPE_STRUCT || type->kind == AST_TYPE_ARRAY) {\
			this->registers[index]._pointer = output;\
		} else if(type->kind == AST_TYPE_ENUM) {\
			this->registers[index]._s64 = *(int32_t*)output;\
		} else { \
			assert(false);\
		}\
	}

#define GET_VALUE_FROM_REGISTER(variable, index, result_type) \
	void* variable = NULL;\
	{\
		auto type = (AST_Type*)types[result_type];\
		if(type->kind == AST_TYPE_DEFINITION) {\
			auto tdef = (AST_Type_Definition*)type;\
			if(tdef->internal_type == AST_Type_s8) variable = (void*)this->registers[index]._s8;\
			else if(tdef->internal_type == AST_Type_s16) variable = (void*)this->registers[index]._s16;\
			else if(tdef->internal_type == AST_Type_s32) variable = (void*)this->registers[index]._s32;\
			else if(tdef->internal_type == AST_Type_s64) variable = (void*)this->registers[index]._s64;\
			else if(tdef->internal_type == AST_Type_u8) variable = (void*)this->registers[index]._u8;\
			else if(tdef->internal_type == AST_Type_u16) variable = (void*)this->registers[index]._u16;\
			else if(tdef->internal_type == AST_Type_u32) variable = (void*)this->registers[index]._u32;\
			else if(tdef->internal_type == AST_Type_u64) variable = (void*)this->registers[index]._u64;\
			else if(tdef->internal_type == AST_Type_char) variable = (void*)this->registers[index]._u32;\
			else if(tdef->internal_type == AST_Type_float) variable = (void*)(int*)&this->registers[index]._float;\
			else variable = this->registers[index]._pointer;\
		} else {\
			assert(false);\
		}\
	}

#define GET_SIZE_OF_TYPE(variable, index) \
	int variable = 0;\
	{\
		auto type = (AST_Type*)types[index];\
		if(type->kind == AST_TYPE_DEFINITION) {\
			auto tdef = (AST_Type_Definition*)type;\
			if(tdef->internal_type == AST_Type_s8) variable = 1;\
			else if(tdef->internal_type == AST_Type_s16) variable = 2;\
			else if(tdef->internal_type == AST_Type_s32) variable = 4;\
			else if(tdef->internal_type == AST_Type_s64) variable = 8;\
			else if(tdef->internal_type == AST_Type_u8) variable = 1;\
			else if(tdef->internal_type == AST_Type_u16) variable = 2;\
			else if(tdef->internal_type == AST_Type_u32) variable = 4;\
			else if(tdef->internal_type == AST_Type_u64) variable = 8;\
			else if(tdef->internal_type == AST_Type_char) variable = 1;\
			else if(tdef->internal_type == AST_Type_float) variable = 8;/*idk*/\
			else if(tdef->internal_type == AST_Type_pointer) variable = 4;\
			else if(tdef->internal_type == AST_Type_c_call) variable = 4;\
			else variable = 1;\
		} else if(type->kind == AST_TYPE_POINTER || type->kind == AST_TYPE_STRUCT || type->kind == AST_TYPE_ARRAY) {\
			variable = interpret->type_pointer->size;\
		} else { \
			assert(false);\
		}\
	}

void* test(void* a1, void* a2, void* a3, void* a4) {
	return (void*)0;
}

AST_Type* BytecodeRunner::get_type(AST_Type* type) {
	if (type->kind == AST_TYPE_POINTER) {
		auto point = (AST_Pointer*)type;
		return get_type(point->point_type);
	}

	return type;
}

int BytecodeRunner::run_expression(int address) {

	auto bc = get_bytecode(address);

	switch (bc->instruction) {
		case BYTECODE_NOOP: {
			return 0;
		}
		case BYTECODE_INSTRICT_ASSERT: {
			//auto x = this->registers[bc->index_r]._pointer;
			assert(this->registers[bc->index_r]._s64 != 0);
			//WNDCLASSEXA* example = (WNDCLASSEXA*)this->registers[bc->index_r]._pointer;
			return 0;
		}
		case BYTECODE_INSTRICT_PRINT: {
			auto print = this->registers[bc->index_r];
			auto type = get_type(this->types[bc->index_r]);

			if (type->kind == AST_TYPE_DEFINITION) {
				auto tdef = (AST_Type_Definition*)type;

				if (tdef->internal_type == AST_Type_s8) {
					printf("%d", this->registers[bc->index_r]._s8);
				}
				else if (tdef->internal_type == AST_Type_s16) {
					printf("%d", this->registers[bc->index_r]._s16);
				}
				else if (tdef->internal_type == AST_Type_s32) {
					printf("%d", this->registers[bc->index_r]._s32);
				}
				else if (tdef->internal_type == AST_Type_s64) {
					//auto x = *(int64_t*)(((int8_t*)this->registers[bc->index_r]._pointer)); //this is good but why is not in s64
					printf("%lld", this->registers[bc->index_r]._s64);
					//printf("%lld", x);
				}
				else if (tdef->internal_type == AST_Type_u8) {
					printf("%d", this->registers[bc->index_r]._u8);
				}
				else if (tdef->internal_type == AST_Type_u16) {
					printf("%d", this->registers[bc->index_r]._u16);
				}
				else if (tdef->internal_type == AST_Type_u32) {
					printf("%d", this->registers[bc->index_r]._u32);
				}
				else if (tdef->internal_type == AST_Type_u64) {
					printf("%lld", this->registers[bc->index_r]._u64);
				}
				else if (tdef->internal_type == AST_Type_string) {
					auto str = ((String*)this->registers[bc->index_r]._pointer);
					printf("%s", ((String*)this->registers[bc->index_r]._pointer)->data);
				}
				else if (tdef->internal_type == AST_Type_pointer) {
					printf("%p", &this->registers[bc->index_r]._pointer);
				}
			}
			else if(type->kind == AST_TYPE_ENUM) {
				printf("%d", this->registers[bc->index_r]._s32);
			}
			else {
				auto point = this->registers[bc->index_r]._pointer;
				printf("%p", &this->registers[bc->index_r]._pointer);
			}
			//auto type = (AST_Type_Definition*)types[bc->index_r];
			//auto c = (New_String*)this->registers[bc->index_r]._pointer;
			//auto ch = (char*)this->registers[bc->index_r]._pointer;
			//printf("\n%lld", this->registers[bc->index_r]._s64);
			//printf("\n%c", (char)this->registers[bc->index_r]._u8);
			return 0;
		}
		case BYTECODE_C_CALL_FROM_PROCEDURE: {
			auto reg = this->registers[bc->index_r];
			auto proc = (AST_Procedure*)bc->big_constant._pointer;

			/*if (1 == 1) {
				FunRegister* freg = new FunRegister(proc);
				/*freg->_function = [proc](void* arg1, void* arg2, void* arg3, void* arg4) {
					//return (void*)proc->bytecode_address; 
					return (void*)0;
				};* /
				//freg->_function = &test;

				this->registers[bc->index_r]._pointer = (void*)freg;//&
				auto regd = this->registers[bc->index_r];
				auto cxcx = 41;
			}		*/	
			
			//auto fr = ((FunRegister*)(this->registers[bc->index_r]._pointer));
			//void* xxx = (void*)fr;
			//auto xx = (*fr)((void*)125);
			/*
			WNDPROCFUN f1 = NULL;
			if (1 == 1) {
				auto b = [&](void* hwnd, void* uMsg, void* wParam, void* lParam, void* arg5, void* arg6, void* arg7, void* arg8, void* arg9, void* arg10, void* arg11, void* arg12) {
					if ((UINT)uMsg == WM_PAINT) {
						PAINTSTRUCT ps;
						HDC hdc = BeginPaint((HWND)hwnd, &ps);
						auto color = 0 | 0 << 8 | 150 << 16;
						FillRect(hdc, &ps.rcPaint, CreateSolidBrush(color));
						EndPaint((HWND)hwnd, &ps);
						return (void*)0;
					}
					return (void*)DefWindowProc((HWND)hwnd, (UINT)uMsg, (WPARAM)wParam, (LPARAM)lParam);
				};
				FunRegister* freg = new FunRegister(proc);
				f1 = (WNDPROCFUN)Lambda::ptr<void*>(b);
			}*/
			
			//FunRegister* freg = new FunRegister(proc);
			auto b = [&](void* hwnd, void* uMsg, void* wParam, void* lParam, void* arg5, void* arg6, void* arg7, void* arg8, void* arg9, void* arg10, void* arg11, void* arg12) {
				if ((UINT)uMsg == WM_PAINT) {
					PAINTSTRUCT ps;
					HDC hdc = BeginPaint((HWND)hwnd, &ps);
					auto color = 0 | 0 << 8 | 150 << 16;
					FillRect(hdc, &ps.rcPaint, CreateSolidBrush(color));
					EndPaint((HWND)hwnd, &ps);
					return (void*)0;
				}
				return (void*)DefWindowProcA((HWND)hwnd, (UINT)uMsg, (WPARAM)wParam, (LPARAM)lParam);
			};
			auto _pointer = (WNDPROCFUN)Lambda::ptr<void*>(b);

			memcpy(&this->registers[bc->index_r]._pointer, &_pointer, sizeof(_pointer));

			return bc->index_r;
		}
		case BYTECODE_CAST: {
			auto type_from = (AST_Type*)types[bc->index_a];
			auto type_to = (AST_Type*)types[bc->index_r];

			if (type_from->kind == AST_TYPE_DEFINITION) {
				auto type_def_from = static_cast<AST_Type_Definition*>(type_from);

				//Convert string
				if (type_def_from->internal_type == AST_Type_string) {
					//To pointer of
					if (type_to->kind == AST_TYPE_POINTER) {
						auto type_pointer_to = static_cast<AST_Pointer*>(type_to);
						
						if (type_pointer_to->point_to->type == AST_TYPE) {
							auto point_to_type = static_cast<AST_Type*>(type_pointer_to->point_to);

							if (point_to_type->kind == AST_TYPE_DEFINITION) {
								auto type_def_to = static_cast<AST_Type_Definition*>(point_to_type);

								//*char
								if (type_def_to->internal_type == AST_Type_char) {
									this->registers[bc->index_r]._pointer = (static_cast<String*>(this->registers[bc->index_a]._pointer))->data;
								}
							}
						}
					}
				}
				if (type_def_from->internal_type == AST_Type_s64) {
					if (type_to->kind == AST_TYPE_DEFINITION) {
						auto type_def_to = static_cast<AST_Type_Definition*>(type_to);

						if (type_def_to->internal_type == AST_Type_s32) {
							this->registers[bc->index_r]._s32 = (int32_t)this->registers[bc->index_a]._s64;
						}
						else if (type_def_to->internal_type == AST_Type_u32) {
							this->registers[bc->index_r]._u32 = (uint32_t)this->registers[bc->index_a]._s64;
						}
						else if (type_def_to->internal_type == AST_Type_s16) {
							this->registers[bc->index_r]._s16 = (int16_t)this->registers[bc->index_a]._s64;
						}
						else if (type_def_to->internal_type == AST_Type_u16) {
							this->registers[bc->index_r]._u16 = (uint16_t)this->registers[bc->index_a]._s64;
						}
						else if (type_def_to->internal_type == AST_Type_s8) {
							this->registers[bc->index_r]._s8 = (int8_t)this->registers[bc->index_a]._s64;
						}
						else if (type_def_to->internal_type == AST_Type_u8) {
							this->registers[bc->index_r]._u8 = (uint8_t)this->registers[bc->index_a]._s64;
						}
					}
				}
			}

			break;
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
			if (this->registers[bc->index_a]._s8) {
				current_address = bc->index_r - 1;
			}

			return 0;
		}
		case BYTECODE_JUMP_IF_NOT: {
			if (!this->registers[bc->index_a]._s8) {
				current_address = bc->index_r - 1;
			}

			return 0;
		}

		case BYTECODE_MOVE_A_TO_R: {
			//printf("\n  v%d <= %lld(%d)", bc->index_r, this->registers[bc->index_a]._s64, bc->index_a);
			auto type = this->types[bc->index_a];
			auto is_pointer = false;
			if (type->kind == AST_TYPE_DEFINITION) {
				auto td = (AST_Type_Definition*)type;
				if (td->internal_type == AST_Type_pointer) {
					is_pointer = true;
				}
			}
			/*else if (type->kind == AST_TYPE_POINTER) { //idk this???
				is_pointer = true; 
			}*/
			is_pointer = false;

			if (is_pointer) {
				this->registers[bc->index_r]._pointer = &this->registers[bc->index_a]._pointer;
			}
			else {
				this->registers[bc->index_r] = this->registers[bc->index_a];
			}
			return bc->index_r;
		}
		case BYTECODE_MOVE_A_BY_REFERENCE_TO_R: { //???
			this->registers[bc->index_r] = *(static_cast<Register*>(this->registers[bc->index_a]._pointer));
			return bc->index_r;
		}
		case BYTECODE_MOVE_A_BY_REFERENCE_PLUS_OFFSET_TO_R: {			
			/*
			auto type_a = (AST_Type*)types[bc->index_a];
			auto type_r = (AST_Type*)types[bc->index_r];
			auto offset = this->registers[bc->index_r]._s64;
			auto pointer = this->registers[bc->index_a]._pointer;
			*/
			auto type_r = (AST_Type*)types[bc->index_a];
			auto tope = get_type((AST_Type*)types[bc->index_a]);
			auto yyy = this->registers[bc->index_a]._pointer;
			auto xxx = this->registers[bc->index_b]._s64;
			void* posy = (int8_t*)this->registers[bc->index_a]._pointer + (this->registers[bc->index_b]._s64);
			void* posa = (int8_t*)this->registers[bc->index_a]._pointer;
			//auto sada = *(int64_t*)posy;

			/*
			* void* pos = (int8_t*)this->registers[pointer]._pointer + (offset);
			auto type = get_type((AST_Type*)types[index]); \
		void* pos = (int8_t*)this->registers[pointer]._pointer + (offset);\
			*/

			ASSIGN_TO_REGISTER_BY_TYPE_WITH_OFFSET(bc->index_r, bc->index_a, this->registers[bc->index_b]._s64);

			/*
			void* pos = (int8_t*)this->registers[bc->index_a]._pointer + (this->registers[bc->index_b]._s64);
			auto xoxox3 = this->registers[bc->index_a]._s32;
			auto xoxoxo = this->registers[bc->index_r]._s64;
			auto xoxox2 = this->registers[bc->index_r]._u64;
			int64_t i64 = *(int64_t*)pos;
			auto ycyxc = 0;
			*/
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
			GET_SIZE_OF_TYPE(size_of, bc->index_r);
			void* pos = (int8_t*)this->registers[bc->index_r]._pointer + bc->index_b;
			
			void* val = NULL;
			auto type = static_cast<AST_Type*>(this->types[bc->index_r]);
			if (type->kind == AST_TYPE_DEFINITION) {
				//auto olo = this->registers[bc->index_a];
				GET_VALUE_FROM_REGISTER(value, bc->index_a, bc->index_r);
				val = value;
			}
			else if (type->kind == AST_TYPE_STRUCT || type->kind == AST_TYPE_ARRAY || type->kind == AST_TYPE_POINTER) {
				val = this->registers[bc->index_a]._pointer;
			}
			else {
				val = &this->registers[bc->index_a];
			}

			memcpy(pos, &val, size_of);
			return bc->index_r;
		}
		case BYTECODE_MOVE_A_TO_R_PLUS_OFFSET_REG: {	//mov6 *v24(r) + v39(b) = v36(a)
														//malloc v24 >> 36
			bool reverse = true; // bc->options == 1;
			int moving_register = bc->index_a;
			int saving_register = bc->index_r;
			if (bc->options == 1) {
				//moving_register = bc->index_r;
				//saving_register = bc->index_a;
				//switch a to r
				//auto save = bc->index_r;
				//bc->index_r = bc->index_a;
				//bc->index_a = save;
				reverse = false;
			}

			auto x = static_cast<AST_Type_Definition*>(this->types[moving_register]);
			GET_SIZE_OF_TYPE(size_of, moving_register); //moving_register ??? když vlevo je u32 a vpravo u64 tak by to bylo bad!
			void* pos = (int8_t*)this->registers[bc->index_r]._pointer;

			WNDCLASSEXA* example = (WNDCLASSEXA*)this->registers[bc->index_r]._pointer;

			void* val = NULL;
			auto type = static_cast<AST_Type*>(this->types[moving_register]);
			if (type->kind == AST_TYPE_DEFINITION) {
				GET_VALUE_FROM_REGISTER(value, moving_register, moving_register);
				val = value;
			}
			else if (type->kind == AST_TYPE_STRUCT || type->kind == AST_TYPE_ARRAY || type->kind == AST_TYPE_POINTER) {
				val = this->registers[moving_register]._pointer;
			}
			else {
				val = &this->registers[moving_register];
			}

			auto type_out = static_cast<AST_Type*>(this->types[saving_register]);
			auto output = pos;
			if (type_out->kind == AST_TYPE_POINTER) {
				output = *((void**)pos); //get the pointer inside the pointer
			}

			output = (int8_t*)output + this->registers[bc->index_b]._s64;

			if (type->kind == AST_TYPE_DEFINITION) {
				auto tdef = (AST_Type_Definition*)type;
				if (tdef->size == 8) {
					if(tdef->internal_type == AST_Type_s64)
						smemcpy(output, &this->registers[moving_register]._s64, size_of, reverse);
					else if (tdef->internal_type == AST_Type_u64)
						smemcpy(output, &this->registers[moving_register]._u64, size_of, reverse);
					else {
						interpret->report_error("Unkown 8bit size of register supported is only s64 and u64");
						assert(false);
					}
				}
				else if (tdef->internal_type == AST_Type_pointer) {
					smemcpy(output, &val, size_of, reverse); //??? idk maybe size_of constant 4 or remove because down &val?
				}
				else {
					smemcpy(output, &val, size_of, reverse);
				}
			}
			else {
				smemcpy(output, &val, size_of, reverse);
			}

			if(reverse)
				return bc->index_r;

			return bc->index_a;

			//printf("\nmov *v%d + %lld = %lld", bc->index_r, this->registers[bc->index_b]._s64, this->registers[bc->index_a]._s64);
			/////void* pos = (int8_t*)this->registers[bc->index_r]._pointer + this->registers[bc->index_b]._s64;
			//void* pos = &this->registers[bc->index_r]._pointer + this->registers[bc->index_b]._s64;
			/////memcpy(pos, &this->registers[bc->index_a]._s64, sizeof(this->registers[bc->index_a]._s64));	

			//auto xx = &this->registers[bc->index_a]._s64;
			//auto xy = this->registers[bc->index_a]._s64;
			//auto yx = 0;
			//auto a = [](int a) { return 0; };
			//memcpy(pos, &a, 4);

			/////return bc->index_r;
		}
		case BYTECODE_MOVE_A_PLUS_OFFSET_TO_R: {
			void* pos = (int8_t*)this->registers[bc->index_a]._pointer + bc->index_b;
			this->registers[bc->index_r] = *(static_cast<Register*>(pos));
			return bc->index_r;
		}
		case BYTECODE_RESERVE_MEMORY_TO_R: {
			//options 0 == number
			//		  1 == reg

			auto size = 0;
			if (bc->options == 0) {
				size = bc->index_a;
			}
			else {
				//size = this->registers[bc->index_a]._s64;
				GET_VALUE_FROM_REGISTER(size_val, bc->index_a, bc->index_a)
				size = (int)size_val;
			}

			this->registers[bc->index_r]._pointer = malloc(size);
			memset(this->registers[bc->index_r]._pointer, 0, size);//fill it with NULL
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
				while (arg->substitution != NULL) arg = arg->substitution;

				if (arg->type == AST_TYPE) {
					AST_Type* type = (AST_Type*)arg;
					if (type->kind == AST_TYPE_POINTER) {
						AST_Pointer* ptr = (AST_Pointer*)type;
						arg = ptr->point_to;
					}
				}//????

				if (arg->type == AST_IDENT) {
					AST_Ident* ident = (AST_Ident*)arg;
					arg = ident->type_declaration;
				}

				if (arg->type == AST_DECLARATION) {
					AST_Declaration* decl = (AST_Declaration*)arg;
					if (decl->inferred_type->kind == AST_TYPE_STRUCT) {
						
						void* posy = (int8_t*)reg._pointer;
						int64_t t64 = *(int32_t*)posy;

						AST_Struct* strct = (AST_Struct*)decl->inferred_type;
						Register copy_reg = Register();
						copy_reg._pointer = malloc(strct->size);
						assert(copy_reg._pointer != NULL);
						memcpy(copy_reg._pointer, reg._pointer, strct->size);
						reg = copy_reg;
						
						void* posy2 = (int8_t*)reg._pointer;
						int64_t t642 = *(int32_t*)posy2;
						auto xdsf = 4;

						//Register copy_reg = Register();
						//copy_reg._pointer = reg._pointer;
						//reg = copy_reg;
						//Copy the struct so wee don't give to the function reference but only copy of it!
					}
					else if (decl->inferred_type->kind == AST_TYPE_POINTER) {
						AST_Pointer* point = (AST_Pointer*)decl->inferred_type;
						//reg._pointer = &point->point_to; //wtf???
					}
				}

				//auto xx = this->registers[arg->bytecode_address]._pointer;
				regs.push_back(reg);
				types.push_back(this->types[arg->bytecode_address]); //bytecode_index ???
			}
			
			if (procedure->procedure->flags & AST_PROCEDURE_FLAG_FOREIGN) {
				auto lit = static_cast<AST_Literal*>(procedure->procedure->foreign_library_expression);

				void* result = CallDllFunction(get_hmodule(lit->string_value.data), procedure->name.data, regs, types);			

				auto xoxox = GetLastError();				

				if (procedure->procedure->returnType != NULL) {
					auto type = static_cast<AST_Type*>(this->types[procedure->return_register]);
					if (type->kind == AST_TYPE_DEFINITION) {
						auto type_def = static_cast<AST_Type_Definition*>(type);
						if (type_def->internal_type == AST_Type_pointer) {
							registers[procedure->return_register]._pointer = result;
						}
						else if (type_def->internal_type == AST_Type_string) {
							auto str = String((const char*)result);
							registers[procedure->return_register]._pointer = &str;
						}
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
					//auto xxxx = srec->registers[i];
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
			auto type = static_cast<AST_Type*>(this->types[bc->index_a]);
			if (type->kind == AST_TYPE_DEFINITION) {
				GET_VALUE_FROM_REGISTER(value, bc->index_a, bc->index_a);
				this->registers[bc->index_r]._pointer = &value;
			}
			else if (type->kind == AST_TYPE_STRUCT || type->kind == AST_TYPE_ARRAY || type->kind == AST_TYPE_POINTER) {
				this->registers[bc->index_r]._pointer = &(this->registers[bc->index_a]._pointer);
			}
			else {
				this->registers[bc->index_r]._pointer = &(this->registers[bc->index_a]);
			}
			
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
			//auto xo = this->registers[bc->index_r]._pointer;
			stack.pop_back();
			return 0;
		}		
	}

	if (is_binop(bc->instruction)) {
		return run_binop(address);
	}

	return 0;
}

void* BytecodeRunner::smemcpy(void* dest, void* src, size_t size, bool reverse) {
	if (!reverse)
		return memcpy(src, dest, size);
	return memcpy(dest, src, size);
}

int BytecodeRunner::run_binop(int address) {
	auto bc = get_bytecode(address);

	//this->registers[bc->index_r]._u64 = 0; // WTF IS THIS OMG

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