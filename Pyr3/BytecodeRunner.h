#pragma once
#include "Interpret.h"
#include "Bytecode.h"
#include "Array.h"

class BytecodeRunner
{
private:
	Interpret* interpret;
	vector<ByteCode*> bytecodes;
	//vector<Register> registers;
	vector<Register> stack; 
	vector<Register> addressStack;
	Array<Register> registers;

	int current_address;

	ByteCode* get_bytecode(int address);
	bool is_binop(Bytecode_Instruction bc_inst);
public:
	BytecodeRunner(Interpret* interpret, vector<ByteCode*> bytecodes, int register_size, int memory_size);
	void run(int address);
	void loop();
	int run_expression(int address);
	int run_binop(int address);
	int get_address_of_procedure(String procedure_name, AST_Block* block);
	void set_current_address(int address);
};

