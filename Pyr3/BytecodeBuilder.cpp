#pragma once
#include "BytecodeBuilder.h"

BytecodeBuilder::BytecodeBuilder(Interpret* interpret):interpret(interpret) {

}

ByteCode* BytecodeBuilder::instruction(Bytecode_Instruction instruction, int a, int b, int result, int line_number) {
	ByteCode* bc = new ByteCode();

	bc->instruction = instruction;
	bc->index_a = a;
	bc->index_b = b;
	bc->index_r = result;
	bc->flags = 0;
	bc->index_instruction = ++instruction_index;

	bc->line_number = line_number;

	return bc;
}

vector<ByteCode*> BytecodeBuilder::get_instructions() {
	return bytecodes;
}

int BytecodeBuilder::get_output_register_size() {
	return output_registers_index;
}

int BytecodeBuilder::get_output_stack_size() {
	return output_registers_end;
}

int BytecodeBuilder::allocate_output_register(AST_Type* type) {
	int save_index = output_registers_index;
	int array_size = calculate_array_size(type);

	if (type->kind == AST_TYPE_DEFINITION) {
		AST_Type_Definition* typdef = static_cast<AST_Type_Definition*>(type);
		
		output_registers_index += 1;
		int save_end = output_registers_end;
		output_registers_end += typdef->size * array_size;
		output_registers.push_back(new output_register(save_end, output_registers_end));
	}

	return save_index;
}

void BytecodeBuilder::build(AST_Block* block) {
	//First we build everything except functions
	vector<AST_Expression*> build_after;

	for (auto index = 0; index < block->expressions.size(); index++) {
		auto it = block->expressions[index];
		if (it->type == AST_DECLARATION) {
			auto declaration = static_cast<AST_Declaration*>(it);
			if (declaration->value != NULL && declaration->value->type == AST_PROCEDURE) {
				int output_register = allocate_output_register(interpret->type_s64);
				declaration->register_index = output_register;

				build_after.push_back(it);
				continue;
			}
		}
		build_expression(it);
	}	

	for (auto index = 0; index < build_after.size(); index++) {		
		build_expression(build_after[index]);
	}
}

int BytecodeBuilder::calculate_array_size(AST_Type* type) {
	uint64_t resolveSize = 1;

	for (int i = 0; i < type->array_size.size(); i++) {
		auto size = type->array_size[i];
		if (size->type == AST_LITERAL) {
			auto lit = static_cast<AST_Literal*>(size);
			if (lit->value_type == LITERAL_NUMBER) {
				resolveSize *= lit->integer_value;
			}
			else {
				assert(false);
			}
		}
		else {
			assert(false && "constant parse");
		}
	}

	return resolveSize;
}

int BytecodeBuilder::build_expression(AST_Expression* expression) {
	switch (expression->type) {
	case AST_IDENT: {
		AST_Ident* ident = static_cast<AST_Ident*>(expression);		
		return ident->type_declaration->register_index;
	}break;
	case AST_BLOCK: {
		AST_Block* _block = static_cast<AST_Block*>(expression);

	}break;
	case AST_CONDITION: {
		AST_Condition* condition = static_cast<AST_Condition*>(expression);
		return build_condition(condition);
	}break;
	case AST_DECLARATION: {
		AST_Declaration* declaration = static_cast<AST_Declaration*>(expression);

		if (declaration->value != NULL && declaration->value->type == AST_TYPE) {
			return -1;
		}

		AST_Type* type = static_cast<AST_Type*>(declaration->assigmet_type);

		if (declaration->register_index == -1) {
			int output_register = allocate_output_register(static_cast<AST_Type_Definition*>(declaration->assigmet_type));
			declaration->register_index = output_register;

			//auto instr = Instruction(BYTECODE_ASSING_TO_BIG_CONSTANT, -1, -1, -1);
			//instr->big_constant._s64 = ident->type_declaration->register_index;			
		}		

		if (declaration->value != NULL) {
			switch (declaration->value->type) {
				case AST_LITERAL: {
					auto literal = static_cast<AST_Literal*>(declaration->value);

					if (literal->value_type == LITERAL_NUMBER) {
						auto instr = Instruction(BYTECODE_ASSING_TO_BIG_CONSTANT, -1, -1, declaration->register_index);

						if (literal->number_flags & NUMBER_FLAG_FLOAT)
							instr->big_constant._float = literal->float_value;
						else if (literal->number_flags & NUMBER_FLAG_SIGNED)
							instr->big_constant._s64 = literal->integer_value;
						else
							instr->big_constant._u64 = literal->integer_value;
					}
				}break;
				default: {
					/*
					auto instr_initialize = Instruction(BYTECODE_ASSING_TO_BIG_CONSTANT, -1, -1, declaration->register_index);
					instr_initialize->big_constant._s64 = 0;
					*/
					auto index = build_expression(declaration->value);
					auto instr = Instruction(BYTECODE_MOVE_A_TO_R, index, -1, declaration->register_index);
				}break;
			}
		}

		return declaration->register_index;
	}break;
	case AST_TYPE: {

	}break;
	case AST_LITERAL: {
		AST_Literal* literal = static_cast<AST_Literal*>(expression);

		if (literal->value_type == LITERAL_NUMBER) {
			int output_register = allocate_output_register(interpret->type_s64);
			auto instr = Instruction(BYTECODE_ASSING_TO_BIG_CONSTANT, -1, -1, output_register);

			if (literal->number_flags & NUMBER_FLAG_FLOAT) 
				instr->big_constant._float = literal->float_value;
			else if(literal->number_flags & NUMBER_FLAG_SIGNED)
				instr->big_constant._s64 = literal->integer_value;
			else
				instr->big_constant._u64 = literal->integer_value;

			return output_register;
		}
		break;
	}
	case AST_BINARYOP: {
		AST_BinaryOp* binary = static_cast<AST_BinaryOp*>(expression);
		return build_binary(binary);
	}
	case AST_PROCEDURE: {		
		AST_Procedure* procedure = static_cast<AST_Procedure*>(expression);
		int result_register = allocate_output_register(interpret->type_pointer);
		
		build(procedure->body);

		return result_register;
	}
	case AST_PARAMLIST: {

	}break;
	case AST_UNARYOP: {
		AST_UnaryOp* unary = static_cast<AST_UnaryOp*>(expression);
		return build_unary(unary);
	}
	case AST_RETURN: {

	}break;
	case AST_DIRECTIVE: {
		AST_Directive* directive = static_cast<AST_Directive*>(expression);
		break;
	}
	default:
		assert(false);
		break;
	}

	assert(false);
	return -1;
}

AST_Declaration* BytecodeBuilder::find_declaration(AST_Ident* ident, AST_Block* scope) {
	for (int i = 0; i < scope->expressions.size(); i++) {
		auto expression = scope->expressions[i];
		if (expression->type == AST_DECLARATION) {
			auto declaration = static_cast<AST_Declaration*>(expression);
			if (declaration->ident->name->value == ident->name->value) {
				return declaration;
			}
		}
	}

	if (scope->scope != NULL)
		return find_declaration(ident, scope->scope);

	return NULL;
}

int BytecodeBuilder::find_address_of(AST_Expression* expression) {
	if (expression->type == AST_IDENT) {
		auto ident = static_cast<AST_Ident*>(expression);
		auto declaration = find_declaration(ident, ident->scope);
		return declaration->register_index;
	}
	else if (expression->type == AST_UNARYOP) {
		auto unary = static_cast<AST_UnaryOp*>(expression);
		return find_address_of(unary->left);
	}

	assert(false);
	return 0;
}

int BytecodeBuilder::build_unary(AST_UnaryOp* unary) {
	if (unary->operation == UNOP_CALL) {

	}
	else if (unary->operation == UNOP_DEF) { //*
		int output = allocate_output_register(static_cast<AST_Type_Definition*>(interpret->type_pointer));
		int addr = build_expression(unary->left);
		auto address_reg = Instruction(BYTECODE_MOVE_A_REGISTER_TO_R, addr, -1, output);
		return output;
	}
	else if (unary->operation == UNOP_REF) { //&
		int output = allocate_output_register(static_cast<AST_Type_Definition*>(interpret->type_u64));
		auto address_reg = Instruction(BYTECODE_ASSING_TO_BIG_CONSTANT, -1, -1, output);
		int addr = find_address_of(unary->left);
		//int addr = build_expression(unary->left);
		address_reg->big_constant._s64 = addr;
		return output;
	}

	assert(false);
	return -1;
}

int BytecodeBuilder::build_condition(AST_Condition* condition) {
	int condition_register = build_expression(condition->condition);
	auto instr_if = Instruction(BYTECODE_JUMP_IF, condition_register, -1, instruction_index + 2);
	return 0;
}

int BytecodeBuilder::build_assigment(AST_BinaryOp* binop) {
	int addrResult = build_expression(binop->left);
	int addr = build_expression(binop->right);

	auto inst = Instruction(BYTECODE_MOVE_A_TO_R, addr, -1, addrResult);

	return addr;
}

int BytecodeBuilder::build_binary(AST_BinaryOp* binop) {
	Bytecode_Instruction op = BYTECODE_UNINITIALIZED;

	char operation = binop->operation;

	if (operation == '=') {
		return build_assigment(binop);
	}

	if (operation == '+') {
		op = BYTECODE_BINOP_PLUS;
	}
	else if (operation == '-') {
		op = BYTECODE_BINOP_MINUS;
	}
	else if (operation == '*') {
		op = BYTECODE_BINOP_TIMES;
	}
	else if (operation == '/') {
		op = BYTECODE_BINOP_DIV;
	}
	else if (operation == '%') {
		op = BYTECODE_BINOP_MOD;
	}
	else if (operation == BINOP_ISEQUAL) {
		op = BYTECODE_BINOP_ISEQUAL;
	}
	else if (operation == BINOP_ISNOTEQUAL) {
		op = BYTECODE_BINOP_ISNOTEQUAL;
	}
	else if (operation == '>') {
		op = BYTECODE_BINOP_GREATER;
	}
	else if (operation == BINOP_GREATEREQUAL) {
		op = BYTECODE_BINOP_GREATEREQUAL;
	}
	else if (operation == '<') {
		op = BYTECODE_BINOP_LESS;
	}
	else if (operation == BINOP_LESSEQUAL) {
		op = BYTECODE_BINOP_LESSEQUAL;
	}
	else if (operation == BINOP_LOGIC_AND) {
		op = BYTECODE_BINOP_LOGIC_AND;
	}
	else if (operation == BINOP_LOGIC_OR) {
		op = BYTECODE_BINOP_LOGIC_OR;
	}
	else if (operation == BINOP_BITWISE_AND) {
		op = BYTECODE_BINOP_BITWISE_AND;
	}
	else if (operation == BINOP_BITWISE_OR) {
		op = BYTECODE_BINOP_BITWISE_OR;
	}

	int left_register = build_expression(binop->left);
	int right_register = build_expression(binop->right);
	int output_register = allocate_output_register(interpret->type_bit);

	auto instr = Instruction(op, left_register, right_register, output_register);

	return output_register;
}