#pragma once
#include "BytecodeBuilder.h"

BytecodeBuilder::BytecodeBuilder(Interpret* interpet):interpet(interpet) {

}

ByteCode* BytecodeBuilder::instruction(Bytecode_Instruction instruction, int a, int b, int result, int line_number) {
	ByteCode* bc = new ByteCode();

	bc->instruction = instruction;
	bc->index_a = a;
	bc->index_b = b;
	bc->index_r = result;
	bc->flags = 0;

	bc->line_number = line_number;

	return bc;
}

vector<ByteCode*> BytecodeBuilder::get_instructions() {
	return this->bytecodes;
}

int BytecodeBuilder::get_output_register_size() {
	return this->output_registers_index;
}

int BytecodeBuilder::allocate_output_register(AST_Type_Definition* type_def) {
	int save_index = output_registers_index;
	output_registers_index += 1;
	int save_end = output_registers_end;
	output_registers_end += type_def->size;
	output_registers.push_back(new output_register(save_end, output_registers_end));

	return save_index;
}

void BytecodeBuilder::build(AST_Block* block) {
	//First we build everything except functions
	vector<AST_Expression*> build_after;

	for (auto index = 0; index < block->expressions.size(); index++) {
		auto it = block->expressions[index];
		if (it->type == AST_DECLARATION) {
			auto declaration = static_cast<AST_Declaration*>(it);
			if (declaration->value->type == AST_PROCEDURE) {
				int output_register = allocate_output_register(interpet->type_def_s64);
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

int BytecodeBuilder::build_expression(AST_Expression* expression) {
	switch (expression->type) {
	case AST_IDENT: {
		AST_Ident* ident = static_cast<AST_Ident*>(expression);		
		return ident->type_declaration->register_index;
	}break;
	case AST_BLOCK: {
		AST_Block* _block = static_cast<AST_Block*>(expression);

	}break;
	case AST_DECLARATION: {
		AST_Declaration* declaration = static_cast<AST_Declaration*>(expression);

		if (declaration->register_index == -1) {
			int output_register = allocate_output_register(interpet->type_def_s64);
			declaration->register_index = output_register;

			//auto instr = Instruction(BYTECODE_ASSING_TO_BIG_CONSTANT, -1, -1, -1);
			//instr->big_constant._s64 = ident->type_declaration->register_index;			
		}

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
				auto instr_initialize = Instruction(BYTECODE_ASSING_TO_BIG_CONSTANT, -1, -1, declaration->register_index);
				instr_initialize->big_constant._s64 = 0;

				auto index = build_expression(declaration->value);
				auto instr = Instruction(BYTECODE_MOVE_A_TO_R, index, -1, declaration->register_index);
			}break;
		}

		return declaration->register_index;
	}break;
	case AST_TYPE_DEFINITION: {

	}break;
	case AST_LITERAL: {
		AST_Literal* literal = static_cast<AST_Literal*>(expression);

		if (literal->value_type == LITERAL_NUMBER) {
			int output_register = allocate_output_register(interpet->type_def_s64);
			auto instr = Instruction(BYTECODE_ASSING_TO_BIG_CONSTANT, -1, -1, output_register);

			if (literal->number_flags & NUMBER_FLAG_FLOAT) 
				instr->big_constant._float = literal->float_value;
			else if(literal->number_flags & NUMBER_FLAG_SIGNED)
				instr->big_constant._s64 = literal->integer_value;
			else
				instr->big_constant._u64 = literal->integer_value;

			return output_register;
		}
	}break;
	case AST_BINARYOP: {
		AST_BinaryOp* binary = static_cast<AST_BinaryOp*>(expression);
		return build_binary(binary);
	}break;
	case AST_PROCEDURE: {

	}break;
	case AST_PARAMLIST: {

	}break;
	case AST_UNARYOP: {

	}break;
	case AST_DIRECTIVE: {
		AST_Directive* directive = static_cast<AST_Directive*>(expression);

	}break;
	default:
		assert(false);
		break;
	}

	return -1;
}

int BytecodeBuilder::build_binary(AST_BinaryOp* binop) {
	// ident + exp
	if (binop->left->type == AST_IDENT) {
		auto ident = static_cast<AST_Ident*>(binop->left);
		int output_register = allocate_output_register(interpet->type_def_s64);

		// ident + number
		if (binop->right->type == AST_LITERAL) {
			auto literal = static_cast<AST_Literal*>(binop->right);
			auto instr = Instruction(BYTECODE_INTEGER_ADD_TO_CONSTANT, ident->type_declaration->register_index, -1, output_register);
			if (literal->number_flags & NUMBER_FLAG_FLOAT)
				instr->big_constant._float = literal->float_value;
			else if (literal->number_flags & NUMBER_FLAG_SIGNED)
				instr->big_constant._s64 = literal->integer_value;
			else
				instr->big_constant._u64 = literal->integer_value;
			return output_register;
		}
	}
	else {
		Bytecode_Instruction op = BYTECODE_UNINITIALIZED;
		char operation = binop->operation;
		if (operation == '+') {
			op = BYTECODE_BINOP_PLUS;
		} else if (operation == '-') {
			op = BYTECODE_BINOP_MINUS;
		} else if (operation == '*') {
			op = BYTECODE_BINOP_TIMES;
		} else if (operation == '/') {
			op = BYTECODE_BINOP_DIV;
		} else if (operation == '%') {
			op = BYTECODE_BINOP_MOD;
		} else if (operation == BINOP_ISEQUAL) {
			op = BYTECODE_BINOP_ISEQUAL;
		} else if (operation == BINOP_ISNOTEQUAL) {
			op = BYTECODE_BINOP_ISNOTEQUAL;
		} else if (operation == '>') {
			op = BYTECODE_BINOP_GREATER;
		} else if (operation == BINOP_GREATEREQUAL) {
			op = BYTECODE_BINOP_GREATEREQUAL;
		} else if (operation == '<') {
			op = BYTECODE_BINOP_LESS;
		} else if (operation == BINOP_LESSEQUAL) {
			op = BYTECODE_BINOP_LESSEQUAL;
		} else if (operation == BINOP_LOGIC_AND) {
			op = BYTECODE_BINOP_LOGIC_AND;
		} else if (operation == BINOP_LOGIC_OR) {
			op = BYTECODE_BINOP_LOGIC_OR;
		} else if (operation == BINOP_BITWISE_AND) {
			op = BYTECODE_BINOP_BITWISE_AND;
		} else if (operation == BINOP_BITWISE_OR) {
			op = BYTECODE_BINOP_BITWISE_OR;
		}

		int left_register = build_expression(binop->left);
		int right_register = build_expression(binop->right);
		int output_register = allocate_output_register(interpet->type_def_bit);

		auto instr = Instruction(op, left_register, right_register, output_register);

		return output_register;
	}

	return -1;
}