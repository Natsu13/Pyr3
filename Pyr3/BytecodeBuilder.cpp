#pragma once
#include "BytecodeBuilder.h"

BytecodeBuilder::BytecodeBuilder(Interpret* interpret, TypeResolver* typeResolver) {
	this->interpret = interpret;
	this->typeResolver = typeResolver;
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
	bc->serial = serial_counter++;

	return bc;
}

int BytecodeBuilder::get_current_bytecode_address(int offset) {
	if (current_bytecode != NULL)
		return current_bytecode->index_instruction + offset;
	return 0;
}

vector<ByteCode*> BytecodeBuilder::get_instructions() {
	return bytecodes;
}

vector<AST_Type*> BytecodeBuilder::get_types() {
	return bytecode_types;
}

int BytecodeBuilder::get_output_register_size() {
	return output_registers_index;
}

int BytecodeBuilder::get_output_stack_size() {
	return output_registers_end;
}

int BytecodeBuilder::allocate_output_register(AST_Type* type) {
	int save_index = output_registers_index;
	int array_size = typeResolver->calculate_array_size(type);

	if (bytecode_types.size() <= save_index) {
		int resize = (save_index * 2 < 30) ? save_index * 2 : bytecode_types.size() + 30;
		bytecode_types.resize(resize);
	}
	if (bytecode_types.size() == save_index) {
		bytecode_types.push_back(type);
	}
	else {
		bytecode_types[save_index] = type;
	}

	if (type->kind == AST_TYPE_DEFINITION) {
		AST_Type_Definition* typdef = static_cast<AST_Type_Definition*>(type);
				
		int save_end = output_registers_end;
		output_registers_end += typdef->size * array_size;
		output_registers.push_back(new output_register(save_end, output_registers_end));
	}

	output_registers_index += 1;

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

int BytecodeBuilder::build_expression(AST_Expression* expression) {
	if (expression->substitution != NULL) {
		expression = expression->substitution;
	}

	switch (expression->type) {
	case AST_IDENT: {
		AST_Ident* ident = static_cast<AST_Ident*>(expression);

		auto type_def = typeResolver->find_declaration(ident, ident->scope);
		if (type_def != NULL && type_def->value != NULL && type_def->value->type == AST_PROCEDURE) {
			AST_Procedure* proc = (AST_Procedure*)type_def->value;
			if (proc->flags & AST_PROCEDURE_FLAG_C_CALL) {
				int reg_c = allocate_output_register(interpret->type_c_call);
				auto inst_c = Instruction(BYTECODE_C_CALL_FROM_PROCEDURE, -1, -1, reg_c);
				inst_c->big_constant._pointer = proc;
				return reg_c;
			}
		}

		/*
		if (member->value != NULL && member->value->type == AST_PROCEDURE) {
		AST_Procedure* proc = (AST_Procedure*)member->value;
		if (proc->flags & AST_PROCEDURE_FLAG_C_CALL) {
			int reg_c = allocate_output_register(interpret->type_c_call);
			auto inst_c = Instruction(BYTECODE_C_CALL_FROM_PROCEDURE, -1, -1, reg_c);
			inst_c->big_constant._pointer = proc;

			auto inst = Instruction(BYTECODE_MOVE_A_PLUS_OFFSET_TO_R, addrStruct, member->offset, reg);

			return reg;
		}
	}
		*/
		/*
		if (ident->flags & AST_IDENT_FLAG_C_CALL) {
			auto reg = allocate_output_register(interpret->type_pointer);
			auto decl = typeResolver->find_declaration(ident, ident->scope);

			assert(decl->value->type == AST_PROCEDURE);

			auto instr = Instruction(BYTECODE_C_CALL_FROM_PROCEDURE, -1, -1, reg);
			instr->big_constant._pointer = decl;

			return reg;
		}
		*/
		return ident->type_declaration->register_index;
	}break;
	case AST_BLOCK: {
		AST_Block* _block = static_cast<AST_Block*>(expression);
		build(_block);
		return -1;//maybe return something?
	}break;
	case AST_CONDITION: {
		AST_Condition* condition = static_cast<AST_Condition*>(expression);
		return build_condition(condition);
	}break;
	case AST_DECLARATION: {
		AST_Declaration* declaration = static_cast<AST_Declaration*>(expression);
		return build_declaration(declaration);
	}break;
	case AST_TYPE: {
		AST_Type* type = static_cast<AST_Type*>(expression);
		return build_type(type);
	}break;
	case AST_LITERAL: {
		AST_Literal* literal = static_cast<AST_Literal*>(expression);

		//There is two literal resolve fix me!
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
		else if (literal->value_type == LITERAL_STRING) {
			int output_register = allocate_output_register(interpret->type_string);
			auto instr = Instruction(BYTECODE_ASSING_TO_BIG_CONSTANT, -1, -1, output_register);

			instr->big_constant._pointer = new New_String{ literal->string_value.data, literal->string_value.size };

			return output_register;
		}
		break;
	}
	case AST_BINARY: {
		AST_Binary* binary = static_cast<AST_Binary*>(expression);
		return build_binary(binary);
	}
	case AST_PROCEDURE: {
		AST_Procedure* procedure = static_cast<AST_Procedure*>(expression);
		return build_procedure(procedure);
	}
	case AST_PARAMLIST: {
		break;
	}
	case AST_UNARYOP: {
		AST_UnaryOp* unary = static_cast<AST_UnaryOp*>(expression);
		return build_unary(unary);
	}
	case AST_RETURN: {
		AST_Return* retu = static_cast<AST_Return*>(expression);
		return build_return(retu);
	}
	case AST_DIRECTIVE: {
		AST_Directive* directive = static_cast<AST_Directive*>(expression);
		break;
	}
	case AST_POINTER: {
		AST_Pointer* pointer = static_cast<AST_Pointer*>(expression);
		return build_pointer(pointer);
	}
	case AST_CAST: {
		AST_Cast* cast = static_cast<AST_Cast*>(expression);

		if (cast->flags & CAST_NOCHECK) {
			//need to create new value and return the new value
			return build_expression(cast->cast_expression);
		}

		AST_Type* type = static_cast<AST_Type*>(cast->cast_to);
		int result_register = allocate_output_register(type);
		int from_register = build_expression(cast->cast_expression);

		auto inst = Instruction(BYTECODE_CAST, from_register, -1, result_register);

		if(!(type->kind == AST_TYPE_DEFINITION || type->kind == AST_TYPE_STRUCT || type->kind == AST_TYPE_POINTER)) {
			assert(false);
		}

		return result_register;
	}
	case AST_WHILE: {
		AST_While* whl = static_cast<AST_While*>(expression);
		return build_while(whl);
	}
	default:
		assert(false);
		break;
	}

	assert(false);
	return -1;
}

int BytecodeBuilder::build_while(AST_While* whl) {
	int pre_condition_address = get_current_bytecode_address(0);
	int condition = build_expression(whl->condition);
	auto instr_while = Instruction(BYTECODE_JUMP_IF_NOT, condition, -1, -1);

	build_expression(whl->block);

	Instruction(BYTECODE_JUMP, -1, -1, pre_condition_address);
	instr_while->index_r = get_current_bytecode_address(0);

	return instr_while->index_r;
}

int BytecodeBuilder::build_procedure(AST_Procedure* procedure) {
	if (procedure->flags & AST_PROCEDURE_FLAG_INTRINSIC) return 0;

	int result_register = allocate_output_register(interpret->type_pointer);

	procedure->bytecode_address = get_current_bytecode_address();
	procedure->bytecode_index = output_registers_index;

	//build(procedure->header);
	if (!(procedure->flags & AST_PROCEDURE_FLAG_FOREIGN)) {
		auto size = procedure->header->expressions.size();
		for (int i = size - 1; i >= 0; i--) {
			auto addr = build_expression(procedure->header->expressions[i]);
			Instruction(BYTECODE_POP_FROM_STACK, -1, -1, addr);
		}

		if (procedure->body != NULL) {
			build(procedure->body);
		}

		if (procedure->returnType == NULL) {
			Instruction(BYTECODE_RETURN, -1, -1, -1);
		}
	}

	return result_register;
}

void BytecodeBuilder::build_array(int register_index, AST_Array* _array) {
	assert(typeResolver->is_static(_array->size));
	int size = typeResolver->calculate_array_size(_array);
	assert(size > 0);
	Instruction(BYTECODE_RESERVE_MEMORY_TO_R, size, -1, register_index);
}

int BytecodeBuilder::build_declaration(AST_Declaration* declaration) {
	if (declaration->value != NULL && declaration->value->type == AST_TYPE) {
		auto type = static_cast<AST_Type*>(declaration->value);
		if (type->kind != AST_TYPE_ARRAY) {
			return -1;
		}
	}

	AST_Type* type = static_cast<AST_Type*>(declaration->assigmet_type);

	if (declaration->register_index == -1) {
		int output_register = allocate_output_register(static_cast<AST_Type_Definition*>(declaration->assigmet_type));
		declaration->register_index = output_register;	
	}	

	if (!(declaration->flags & DECLARATION_IN_HEAD) && declaration->value == NULL) {
		if (type->kind == AST_TYPE_STRUCT) {
			auto _struct = static_cast<AST_Struct*>(type);
			Instruction(BYTECODE_RESERVE_MEMORY_TO_R, _struct->size, -1, declaration->register_index);
		}

		if (type->kind == AST_TYPE_ARRAY) {
			build_array(declaration->register_index, (AST_Array*)type);
		}
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
			}else if (literal->value_type == LITERAL_STRING) {
				auto instr = Instruction(BYTECODE_ASSING_TO_BIG_CONSTANT, -1, -1, declaration->register_index);
				instr->big_constant._pointer = new New_String{ literal->string_value.data, literal->string_value.size };
			}

			break;
		}
		case AST_BLOCK: {
			if (type->kind == AST_TYPE_ARRAY) {
				auto arr = (AST_Array*)type;
				build_array(declaration->register_index, arr);
				
				auto block = (AST_Block*)declaration->value;
				auto typeSize = typeResolver->find_typeof(arr->point_to);
				int size = 0;
				if (typeSize->kind == AST_TYPE_DEFINITION) {
					auto typeDef = (AST_Type_Definition*)typeSize;
					size = typeDef->size;
				}

				assert(size != 0);

				for (int i = 0; i < block->expressions.size(); i++) {
					auto in = build_expression(block->expressions[i]);
					Instruction(BYTECODE_MOVE_A_TO_R_PLUS_OFFSET, in, size * i, declaration->register_index);
				}
			}
			else if (type->kind == AST_TYPE_STRUCT) {
				auto _struct = static_cast<AST_Struct*>(type);
				Instruction(BYTECODE_RESERVE_MEMORY_TO_R, _struct->size, -1, declaration->register_index);

				auto block = (AST_Block*)declaration->value;

				int size = 0;
				for (int i = 0; i < _struct->members->expressions.size(); i++) {
					auto expr = _struct->members->expressions[i];										
					auto in = build_expression(block->expressions[i]);
					Instruction(BYTECODE_MOVE_A_TO_R_PLUS_OFFSET, in, size, declaration->register_index);
					size += typeResolver->get_size_of(expr);
				}
			}
			break;
		} 
		default: {
			/*
			auto instr_initialize = Instruction(BYTECODE_ASSING_TO_BIG_CONSTANT, -1, -1, declaration->register_index);
			instr_initialize->big_constant._s64 = 0;
			*/
			auto index = build_expression(declaration->value);
			if (index != -1) {
				auto instr = Instruction(BYTECODE_MOVE_A_TO_R, index, -1, declaration->register_index);
			}
			break;
		}
		}
	}

	return declaration->register_index;
}

int BytecodeBuilder::build_return(AST_Return* ret) {
	//ret->scope->

	int output = allocate_output_register(static_cast<AST_Type_Definition*>(interpret->type_u64));
	int addr = build_expression(ret->value);
	//auto address_reg = Instruction(BYTECODE_MOVE_A_TO_R, addr, -1, output);
	auto stackPush = Instruction(BYTECODE_PUSH_TO_STACK, -1, -1, addr);

	Instruction(BYTECODE_RETURN, -1, -1, -1);

	return -1;
}

int BytecodeBuilder::find_address_of_type(AST_Expression* expression) {
	if (expression->type == AST_DECLARATION) {
		auto declaration = static_cast<AST_Declaration*>(expression);
		if (declaration->assigmet_type->type == AST_TYPE) {
			auto type = static_cast<AST_Type*>(declaration->assigmet_type);
			if (type->kind == AST_TYPE_POINTER) {
				auto pointer = static_cast<AST_Pointer*>(type);
				return find_address_of_type(pointer->point_to);
			}
		}
		return declaration->register_index;
	}

	//assert(false);
	return -1;
}

int BytecodeBuilder::find_address_of(AST_Expression* expression) {
	if (expression->type == AST_IDENT) {
		auto ident = static_cast<AST_Ident*>(expression);
		if (ident->type_declaration != NULL) {
			auto re = find_address_of_type(ident->type_declaration);
			if (re >= 0) {
				return re;
			}
		}

		auto declaration = typeResolver->find_declaration(ident, ident->scope);
		return declaration->register_index;
	}
	else if (expression->type == AST_UNARYOP) {
		auto unary = static_cast<AST_UnaryOp*>(expression);
		return find_address_of(unary->left);
	}
	else if (expression->type == AST_BINARY) {
		auto binary = static_cast<AST_Binary*>(expression);
		
		return find_address_of(binary->left);
	}
	else if (expression->type == AST_TYPE) {
		auto type = static_cast<AST_Type*>(expression);
		if (type->kind == AST_TYPE_ARRAY) {
			auto arr = static_cast<AST_Array*>(type);
			return find_address_of(arr->point_to);
		}
	}

	assert(false);
	return 0;
}

int BytecodeBuilder::build_unary(AST_UnaryOp* unary) {
	if (unary->operation == UNOP_CALL) {
		return build_procedure_call(unary);
	}
	else if (unary->operation == UNOP_DEF) { //*
		int output = allocate_output_register(static_cast<AST_Type_Definition*>(interpret->type_pointer));
		int addr = build_expression(unary->left);
		auto address_reg = Instruction(BYTECODE_MOVE_A_BY_REFERENCE_TO_R, addr, -1, output);
		return output;
	}
	else if (unary->operation == UNOP_REF) { //&
		int addr = build_expression(unary->left);
		int output = allocate_output_register(static_cast<AST_Type_Definition*>(interpret->type_address));
		auto address_reg = Instruction(BYTECODE_ADDRESS_OF, addr, -1, output);
		return output;
	}
	else if (unary->operation == UNOP_INCREMENT) {
		int addr = build_expression(unary->left);

		AST_Literal* literal = AST_NEW_EMPTY(AST_Literal);
		literal->value_type = LITERAL_NUMBER;
		literal->integer_value = 1;
		int plus1 = build_expression(literal);		

		if (unary->isPreppend) {
			auto address_reg = Instruction(BYTECODE_BINOP_PLUS, addr, plus1, addr);
			return addr;
		}

		int output = allocate_output_register(typeResolver->find_typeof(unary->left));
		auto address_reg = Instruction(BYTECODE_MOVE_A_TO_R, addr, -1, output);
		auto address_reg2 = Instruction(BYTECODE_BINOP_PLUS, addr, plus1, addr);
		return output;
	}
	else if (unary->operation == UNOP_DECREMENT) {
		int addr = build_expression(unary->left);

		AST_Literal* literal = AST_NEW_EMPTY(AST_Literal);
		literal->value_type = LITERAL_NUMBER;
		literal->integer_value = 1;
		int plus1 = build_expression(literal);		

		if (unary->isPreppend) {
			//int output = allocate_output_register(typeResolver->find_typeof(unary->left));
			auto address_reg = Instruction(BYTECODE_BINOP_MINUS, addr, plus1, addr);
			return addr;
		}

		int output = allocate_output_register(typeResolver->find_typeof(unary->left));
		auto address_reg = Instruction(BYTECODE_MOVE_A_TO_R, addr, -1, output);
		auto address_reg2 = Instruction(BYTECODE_BINOP_MINUS, addr, plus1, addr);
		return output;
	}

	assert(false);
	return -1;
}

int BytecodeBuilder::build_procedure_call(AST_UnaryOp* unary) {	
	auto call = new Call_Record();

	assert(unary->left->type == AST_IDENT);
	auto literar = static_cast<AST_Ident*>(unary->left);	

	auto declaration = typeResolver->find_declaration(literar, literar->scope);
	assert(declaration != NULL);
	assert(declaration->value->type == AST_PROCEDURE);

	call->name = declaration->ident->name->value;
	call->procedure = static_cast<AST_Procedure*>(declaration->value);
	call->offset = output_registers_index;// get_current_bytecode_address();

	if (call->procedure->flags & AST_PROCEDURE_FLAG_INTRINSIC) {
		delete call;

		if (COMPARE(literar->name->value, "print")) {
			int arg1 = build_expression(unary->arguments->expressions[0]);
			Instruction(BYTECODE_INSTRICT_PRINT, -1, -1, arg1);
			return 0;
		}

		if (COMPARE(literar->name->value, "assert")) {
			int arg1 = build_expression(unary->arguments->expressions[0]);
			Instruction(BYTECODE_INSTRICT_ASSERT, -1, -1, arg1);
			return 0;
		}

		if (COMPARE(literar->name->value, "malloc")) {
			int arg1 = build_expression(unary->arguments->expressions[0]);
			int return_register = allocate_output_register(interpret->type_u8);

			auto opt = Instruction(BYTECODE_RESERVE_MEMORY_TO_R, arg1, -1, return_register);
			opt->options = 1;

			return return_register;
		}

		return 0;
	}
	
	call->arguments.clear();

	for (int i = 0; i < unary->arguments->expressions.size(); i++) {
		auto expr = unary->arguments->expressions[i];
		expr->bytecode_address = build_expression(expr);
		call->arguments.push_back(expr);
	}

	int return_register = -1;
	if (call->procedure->returnType != NULL) {
		AST_Type* return_type = typeResolver->get_inferred_type(call->procedure->returnType);
		return_register = allocate_output_register(return_type);
		call->return_register = return_register;
	}

	auto call_instr = Instruction(BYTECODE_CALL_PROCEDURE, -1, -1, -1);
	call_instr->big_constant._pointer = call;

	if (call->procedure->returnType != NULL) {
		//Curently we support only one return value
		Instruction(BYTECODE_POP_FROM_STACK, -1, -1, return_register);

		return return_register;
	}
	return 0;
}

int BytecodeBuilder::build_pointer(AST_Pointer* type) {
	auto index = build_expression(type->point_to);

	if (type->point_to->type == AST_DECLARATION) {
		auto decla = static_cast<AST_Declaration*>(type->point_to);
		return decla->register_index;
	}
	
	return index;

	//assert(false);
	//return 0;
}

int BytecodeBuilder::build_type(AST_Type* type) {
	if (type->kind == AST_TYPE_POINTER) {
		return build_pointer(static_cast<AST_Pointer*>(type));
	}
	if (type->kind == AST_TYPE_ARRAY) {
		auto arrAddr = find_address_of(type);
		auto offset = build_array_offset(static_cast<AST_Array*>(type));
		int output = allocate_output_register(interpret->type_u64);
		auto inst = Instruction(BYTECODE_MOVE_A_BY_REFERENCE_PLUS_OFFSET_TO_R, arrAddr, offset, output);
		return output;
	}

	assert(false);
	return 0;
}

int BytecodeBuilder::build_condition(AST_Condition* condition) {
	int condition_register = build_expression(condition->condition);
	auto instr_if = Instruction(BYTECODE_JUMP_IF_NOT, condition_register, -1, -1);

	build_expression(condition->body_pass);	

	instr_if->index_r = get_current_bytecode_address(0);

	if (condition->body_fail != NULL) {
		auto instr_go_after_else = Instruction(BYTECODE_JUMP, -1, -1, -1);
		instr_if->index_r += 1;

		build_expression(condition->body_fail);

		instr_go_after_else->index_r = get_current_bytecode_address(0);
	}

	return 0;
}

int BytecodeBuilder::find_offset_of(AST_Expression* expression, AST_Block* scope) {
	if (expression->type == AST_IDENT) {
		auto ident = static_cast<AST_Ident*>(expression);
		auto decl = typeResolver->find_declaration(ident, ident->scope);
		return decl->offset;
	}
	else if (expression->type == AST_BINARY) {
		auto binop = static_cast<AST_Binary*>(expression);
		return find_offset_of(binop->right, binop->left->scope);
	}

	assert(false);
}

AST_Declaration* BytecodeBuilder::find_member_of(AST_Expression* expression, AST_Block* scope) {
	if (expression->type == AST_IDENT) {
		auto ident = static_cast<AST_Ident*>(expression);
		auto decl = typeResolver->find_declaration(ident, ident->scope);
		return decl;
	}
	else if (expression->type == AST_BINARY) {
		auto binop = static_cast<AST_Binary*>(expression);
		return find_member_of(binop->right, binop->left->scope);
	}

	assert(false);
}

int BytecodeBuilder::build_array_offset(AST_Array* arr) {
	if (arr->point_to->type == AST_TYPE) {
		auto type = static_cast<AST_Type*>(arr->point_to);
		if (type->kind == AST_TYPE_ARRAY) {
			auto return_register = allocate_output_register(interpret->type_s64);
			auto left = build_expression(arr->size);
			auto inst = Instruction(BYTECODE_BINOP_PLUS, left, build_array_offset(static_cast<AST_Array*>(type)), return_register);
			return return_register;
		}
	}

	auto offset = build_expression(arr->size);
	auto basesize = typeResolver->find_typedefinition_from_type(arr)->size;
	auto m_register = allocate_output_register(interpret->type_s64);
	auto inst = Instruction(BYTECODE_ASSING_TO_BIG_CONSTANT, -1, -1, m_register);
	inst->big_constant._s64 = basesize;

	auto return_register = allocate_output_register(interpret->type_s64);
	auto inst2 = Instruction(BYTECODE_BINOP_TIMES, offset, m_register, return_register);

	return return_register;
}

int BytecodeBuilder::build_assigment(AST_Binary* binop) {
	int addr = build_expression(binop->right);
	int addrResult = find_address_of(binop->left);
	int addrResultSave = addrResult;

	auto resultType = typeResolver->find_typeof(binop->left);

	// not handling array and struct
	if (binop->operation == BINOP_PLUS_ASIGN) {
		//addrResult = allocate_output_register(resultType);
		auto plus_instr = Instruction(BYTECODE_BINOP_PLUS, addrResultSave, addr, addrResult);
	}
	else if (binop->operation == BINOP_MINUS_ASIGN) {
		//addrResult = allocate_output_register(resultType);
		auto plus_instr = Instruction(BYTECODE_BINOP_MINUS, addrResultSave, addr, addrResult);
	}
	else if (binop->operation == BINOP_TIMES_ASIGN) {
		//addrResult = allocate_output_register(resultType);
		auto plus_instr = Instruction(BYTECODE_BINOP_TIMES, addrResultSave, addr, addrResult);
	}
	else if (binop->operation == BINOP_DIV_ASIGN) {
		//addrResult = allocate_output_register(resultType);
		auto plus_instr = Instruction(BYTECODE_BINOP_DIV, addrResultSave, addr, addrResult);
	}
	else if (binop->left->type == AST_TYPE && static_cast<AST_Type*>(binop->left)->kind == AST_TYPE_ARRAY) {
		auto arr = static_cast<AST_Array*>(binop->left);
		auto offset = build_array_offset(arr);
		auto inst = Instruction(BYTECODE_MOVE_A_TO_R_PLUS_OFFSET_REG, addr, offset, addrResult);
	}
	else if (binop->left->type == AST_BINARY) {
		auto _de_binop = static_cast<AST_Binary*>(binop->left);
		int addrStruct = find_address_of(_de_binop->left);
		int offset = find_offset_of(_de_binop->right, _de_binop->left->scope);
		auto inst = Instruction(BYTECODE_MOVE_A_TO_R_PLUS_OFFSET, addr, offset, addrStruct);
	}
	else {		
		auto inst = Instruction(BYTECODE_MOVE_A_TO_R, addr, -1, addrResult);
	}

	return addr;
}

int BytecodeBuilder::build_reference(AST_Binary* binary) {	
	auto ident = static_cast<AST_Ident*>(binary->left);
	int addrStruct = find_address_of(binary->left);
	auto member = find_member_of(binary->right, binary->left->scope);
	int reg = allocate_output_register(member->inferred_type);

	auto inst = Instruction(BYTECODE_MOVE_A_PLUS_OFFSET_TO_R, addrStruct, member->offset, reg);

	return reg;
}

int BytecodeBuilder::build_binary(AST_Binary* binop) {
	Bytecode_Instruction op = BYTECODE_UNINITIALIZED;

	char operation = binop->operation;

	if (operation == '=' || operation == BINOP_PLUS_ASIGN || operation == BINOP_MINUS_ASIGN 
		|| operation == BINOP_TIMES_ASIGN || operation == BINOP_DIV_ASIGN) {
		return build_assigment(binop);
	}
	if (operation == '.') {
		return build_reference(binop);
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

	int right_register = build_expression(binop->right);
	int left_register = build_expression(binop->left);	
	int output_register = allocate_output_register(interpret->type_bit);

	auto instr = Instruction(op, left_register, right_register, output_register);

	return output_register;
}