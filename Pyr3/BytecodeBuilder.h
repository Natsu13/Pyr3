#pragma once
#include "Interpret.h"
#include "Bytecode.h"

struct output_register {
	int start = -1;
	int end = -1;

	output_register(int s, int e):start(s), end(e) {

	}
};

class BytecodeBuilder
{
private:
	int output_registers_index = 0;
	int output_registers_end = 0;
	Interpret* interpet = nullptr;
	ByteCode* current_bytecode = nullptr;

	vector<ByteCode*> bytecodes;	
	vector<output_register*> output_registers;
	int allocate_output_register(AST_Type_Definition* type_def);
	int build_expression(AST_Expression* expression);
	int build_binary(AST_BinaryOp* binop);

	ByteCode* instruction(Bytecode_Instruction instruction, int a, int b, int result, int line_number);
public:
	BytecodeBuilder(Interpret* interpet);
	void build(AST_Block* block);
	vector<ByteCode*> get_instructions();
	int get_output_register_size();
};

#define Instruction(instr, index_a, index_b, index_r) current_bytecode = insert_and_return(bytecodes, instruction(instr, index_a, index_b, index_r, __LINE__));