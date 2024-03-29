#pragma once
#include "Interpret.h"
#include "Bytecode.h"
#include "Array.h"
#include "Utils.h"
#include "TypeResolver.h"

struct ForeignLibrary {
	const char* name;
	HMODULE mod;
};


class BytecodeRunner
{
private:
	Interpret* interpret;
	TypeResolver* typeResolver;
	vector<ByteCode*> bytecodes;
	vector<AST_Type*> types;
	//vector<Register> registers;
	vector<Register> stack; 
	vector<Register> addressStack;
	vector<ForeignLibrary> foreignLibrary;
	vector<Register> registers;

	int current_address;

	ByteCode* get_bytecode(int address);
	bool is_binop(Bytecode_Instruction bc_inst);
	HMODULE get_hmodule(const char* name);

	AST_Type* get_type(AST_Type* type);
	AST_Type* get_type_base(AST_Type* type);
	void* smemcpy(void* dest, void* src, size_t size, bool reverse = false);
public:
	BytecodeRunner(Interpret* interpret, TypeResolver* typeResolver, vector<ByteCode*> bytecodes, vector<AST_Type*> types, int register_size, int memory_size);
	void run(int address);
	void loop();
	int run_expression(int address);
	int run_binop(int address);
	int get_address_of_procedure(String procedure_name, AST_Block* block);
	void set_current_address(int address);
};