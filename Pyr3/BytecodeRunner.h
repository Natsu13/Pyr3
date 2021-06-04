#pragma once
#include "Interpret.h"
#include "Bytecode.h"

class BytecodeRunner
{
private:
	Interpret* interpret;
	vector<ByteCode*> bytecodes;
	vector<Register> registers;
	vector<Register> stack;

	int current_address;

	ByteCode* get_bytecode(int address);
	bool is_binop(Bytecode_Instruction bc_inst);
public:
	BytecodeRunner(Interpret* interpret, vector<ByteCode*> bytecodes, int register_size, int memory_size);
	void run(int address);
	void loop();
	int run_expression(int address);
	int run_binop(int address);
};

