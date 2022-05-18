#pragma once
#include "Interpret.h"
#include "Bytecode.h"
#include "TypeResolver.h"
#include "Array.h"
#include "Utils.h"

struct output_register {
	int start = -1;
	int end = -1;

	output_register(int s, int e):start(s), end(e) {

	}
};

class BytecodeBuilder
{
private:
	int instruction_index = 0;
	int output_registers_index = 0;
	int output_registers_end = 0;
	int stack_size = 0;
	int serial_counter = 0;
	Interpret* interpret = nullptr;
	TypeResolver* typeResolver = nullptr;
	ByteCode* current_bytecode = nullptr;

	vector<ByteCode*> bytecodes;	
	vector<AST_Type*> bytecode_types;
	vector<output_register*> output_registers;
	int allocate_output_register(AST_Type* type);

	int build_expression(AST_Expression* expression);
	int build_binary(AST_Binary* binop);
	int build_condition(AST_Condition* condition);
	int build_unary(AST_UnaryOp* unary);
	int build_assigment(AST_Binary* binop);
	int build_type(AST_Type* type);
	int build_pointer(AST_Pointer* pointer);
	int build_return(AST_Return* ret);
	int build_procedure_call(AST_UnaryOp* unary);
	int build_intrinsic_procedure_call(Token* name, vector<AST_Expression*> arguments);
	int build_declaration(AST_Declaration* declaration);
	int build_procedure(AST_Procedure* procedure);
	int build_struct_dereference(AST_Binary* binary);
	int build_reference(AST_Binary* binary);
	int build_while(AST_While* whl);
	int build_enum_dereference(AST_Binary* binary);
	int build_for(AST_For* ast_for);

	void build_array(int register_index, AST_Array* _array);
	int build_procedure_call(AST_Procedure* procedure, vector<AST_Expression*> arguments);

	int find_address_of(AST_Expression* expression);
	int find_address_of_type(AST_Expression* expression);
	int find_offset_of(AST_Expression* expression, AST_Block* scope);
	AST_Declaration* find_member_of(AST_Expression* expression, AST_Block* scope);
	AST_Expression* find_last_member(AST_Expression* expression, AST_Block* scope);
	//AST_Declaration* find_declaration(AST_Ident* ident, AST_Block* scope);
	int get_current_bytecode_address(int offset = 0);	

	int build_array_offset(AST_Array* arr);
	int build_struct_offset(AST_Expression* expr, int* input_register, int* result_register, bool* is_pointer = NULL);

	ByteCode* instruction(Bytecode_Instruction instruction, int a, int b, int result, int line_number);
public:
	BytecodeBuilder(Interpret* interpret, TypeResolver* typeResolver);
	void build(AST_Block* block);
	vector<ByteCode*> get_instructions();
	vector<AST_Type*> get_types();
	int get_output_register_size();
	int get_output_stack_size();
};

#define Instruction(instr, index_a, index_b, index_r) current_bytecode = insert_and_return(bytecodes, instruction(instr, index_a, index_b, index_r, __LINE__));