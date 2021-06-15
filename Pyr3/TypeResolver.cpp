#pragma once

#include "Headers.h"
#include "TypeResolver.h"

TypeResolver::TypeResolver(Interpret* interpret) {
	this->interpret = interpret;
}

AST_Literal* TypeResolver::make_string_literal(String value) {
	AST_Literal* literal = AST_NEW_EMPTY(AST_Literal);
	literal->value_type = LITERAL_STRING;
	literal->string_value = value;

	return literal;
}

AST_Literal* TypeResolver::make_number_literal(long long value) {
	AST_Literal* literal = AST_NEW_EMPTY(AST_Literal);
	literal->value_type = LITERAL_NUMBER;
	literal->integer_value = value;

	if (value < 0) literal->number_flags |= NUMBER_FLAG_SIGNED;

	return literal;
}

void TypeResolver::resolve_main(AST_Block* block) {
	phase = 1;
	resolve(block);
	resolveOther();
}

AST_Literal* TypeResolver::make_number_literal(float value) {
	AST_Literal* literal = AST_NEW_EMPTY(AST_Literal);
	literal->value_type = LITERAL_NUMBER;
	literal->float_value = value;
	literal->number_flags |= NUMBER_FLAG_FLOAT;

	return literal;
}

AST_Type* TypeResolver::get_inferred_type(AST_Expression* expression) {
	if (expression->type == AST_IDENT) {
		auto ident = static_cast<AST_Ident*>(expression);
		return ident->type_declaration->inferred_type;
	}
	if (expression->type == AST_TYPE) {
		return static_cast<AST_Type*>(expression);
	}

	assert(false);
	return NULL;
}

void TypeResolver::addToResolve(AST_Expression* expression) {
	to_be_resolved.push_back(expression);
}

void TypeResolver::resolveOther() {
	phase = 2;

	for (int i = 0; i < to_be_resolved.size(); i++) {
		AST_Expression* it = to_be_resolved[i];
		resolveExpression(it);
	}
}

void TypeResolver::resolve(AST_Block* block) {
	for (auto index = 0; index < block->expressions.size(); index++) {
		auto it = block->expressions[index];
		resolveExpression(it);
	}	
}

void TypeResolver::calculate_struct_size(AST_Struct* _struct, int offset) {
	int size = 0;
	for (int i = 0; i < _struct->members->expressions.size(); i++) {
		auto it = _struct->members->expressions[i];

		if (it->type == AST_DECLARATION) {
			auto declaration = static_cast<AST_Declaration*>(it);
			if (declaration->inferred_type->kind == AST_TYPE_DEFINITION) {
				auto type_def = static_cast<AST_Type_Definition*>(declaration->inferred_type);
				declaration->offset = offset + size;

				size+= type_def->size;
				continue;
			}			
			assert(false);
		}
		else if (it->type == AST_TYPE) {
			auto type = static_cast<AST_Type*>(it);
			if (type->kind == AST_STRUCT) {
				auto _type_struct = static_cast<AST_Struct*>(type);
				calculate_struct_size(_type_struct, size);
				size += _type_struct->size;
				continue;
			}
		}

		assert(false);
	}

	_struct->size = size;
}

AST_Type* TypeResolver::resolveExpression(AST_Expression* expression) {
	switch (expression->type) {
		case AST_IDENT: {
			AST_Ident* ident = static_cast<AST_Ident*>(expression);
			return resolveIdent(ident);
		}
		case AST_CONDITION: {
			AST_Condition* condition = static_cast<AST_Condition*>(expression);

			resolveExpression(condition->condition);
			resolveExpression(condition->body_pass);
			if (condition->body_fail != NULL)
				resolveExpression(condition->body_fail);

			return NULL;
		}
		case AST_BLOCK: {
			AST_Block* _block = static_cast<AST_Block*>(expression);
			resolve(_block);

			return NULL;
		}
		case AST_DECLARATION: {
			AST_Declaration* declaration = static_cast<AST_Declaration*>(expression);
			return resolveDeclaration(declaration);
		}
		case AST_TYPE: {
			AST_Type* type = static_cast<AST_Type*>(expression);
			return resolveType(type);
		}
		case AST_LITERAL: {
			AST_Literal* literal = static_cast<AST_Literal*>(expression);
			return resolveLiteral(literal);
		}
		case AST_BINARYOP: {
			AST_Binary* binop = static_cast<AST_Binary*>(expression);
			return resolveBinary(binop);
		}
		case AST_PROCEDURE: {
			AST_Procedure* procedure = static_cast<AST_Procedure*>(expression);
			resolve(procedure->body);
			resolve(procedure->header);
			if(procedure->returnType != NULL)
				resolveExpression(procedure->returnType);

			break;
		}
		case AST_PARAMLIST:
			break;
		case AST_UNARYOP: {
			AST_UnaryOp* unary = static_cast<AST_UnaryOp*>(expression);
			return resolveUnary(unary);
		}
		case AST_RETURN: {
			AST_Return* ast_return = static_cast<AST_Return*>(expression);
			resolveExpression(ast_return->value);
			break;
		}
		case AST_DIRECTIVE: {
			AST_Directive* directive = static_cast<AST_Directive*>(expression);
			resolveDirective(directive);
			return NULL;
		}
		default:
			assert(false);
			break;
	}

	return NULL;
}

AST_Type* TypeResolver::resolveType(AST_Type* type) {
	if (type->kind == AST_TYPE_DEFINITION) {
		AST_Type_Definition* def = static_cast<AST_Type_Definition*>(type);
		return def;
	}
	else if (type->kind == AST_TYPE_POINTER) {
		AST_Pointer* pointer = static_cast<AST_Pointer*>(type);
		auto type = resolveExpression(pointer->point_to);
		pointer->point_type = type;
		return type;
	}
	else if (type->kind == AST_TYPE_STRUCT) {
		AST_Struct* _struct = static_cast<AST_Struct*>(type);
		resolve(_struct->members);
		calculate_struct_size(_struct);

		return type;
	}

	assert(false);
	return NULL;
}

bool TypeResolver::is_pointer(AST_Expression* expression) {
	if (expression->type == AST_UNARYOP) {
		auto unary = static_cast<AST_UnaryOp*>(expression);
		if (unary->operation == UNOP_REF) {
			return true;
		}
	}

	return false;
}

bool TypeResolver::is_number(AST_Expression* expression) {
	if (expression->type == AST_LITERAL) {
		auto literar = static_cast<AST_Literal*>(expression);
		if (literar->value_type == LITERAL_NUMBER) {
			return true;
		}
	}

	return false;
}

AST_Type* TypeResolver::resolveStructDereference(AST_Struct* _struct, AST_Expression* expression) {
	if (expression->type == AST_IDENT) { // struct.member
		auto ident = static_cast<AST_Ident*>(expression);
		auto type = find_declaration(ident, _struct->members);
		ident->type_declaration = type;

		resolveExpression(type);
		resolveExpression(ident);
		return type->inferred_type;
	}

	auto binary = static_cast<AST_Binary*>(expression);
	auto ident = static_cast<AST_Ident*>(binary->left);
	auto type = find_declaration(ident, _struct->members);
	ident->type_declaration = type;

	if (type->assigmet_type->type == AST_TYPE) {
		auto struct_type = static_cast<AST_Type*>(type->assigmet_type);
		assert(struct_type->kind == AST_TYPE_STRUCT);

		return resolveStructDereference(static_cast<AST_Struct*>(struct_type), binary->right);
	}
}

AST_Type* TypeResolver::resolveBinary(AST_Binary* binop) {
	auto type = resolveExpression(binop->left);
	
	if (binop->operation == BINOP_DOT) { // a.b
		auto ident = static_cast<AST_Ident*>(binop->left);
		auto type = find_declaration(ident, binop->scope); //We know left can be only ident
		if (type != NULL && type->assigmet_type->type == AST_TYPE) {
			auto _type = static_cast<AST_Type*>(type->assigmet_type);
			ident->type_declaration = type;
			auto _struct = static_cast<AST_Struct*>(_type);
			binop->right->scope = _struct->members;

			if (_type->kind == AST_TYPE_STRUCT) {
				return resolveStructDereference(_struct, binop->right);
			}
			else {
				assert(false && "Just now we can dereference only struct");
			}
		}
	}

	resolveExpression(binop->right);

	bool left_ispointer = is_pointer(binop->left);
	bool right_ispointer = is_pointer(binop->right);

	if (!left_ispointer && !right_ispointer) {
		return type;
	}

	if (left_ispointer && right_ispointer) {
		interpret->report_error(binop->left->token, "Can't sum two pointers");
		return NULL;
	}

	if (!left_ispointer && right_ispointer) {
		auto temp = binop->left;
		binop->left = binop->right;
		binop->right = temp;
	}

	auto unary = static_cast<AST_UnaryOp*>(binop->left);
	auto pointer = static_cast<AST_Pointer*>(resolveExpression(unary));

	auto size = find_typedefinition_from_type(pointer->point_type)->aligment;

	AST_Literal* literar = AST_NEW_EMPTY(AST_Literal);
	literar->value_type = LITERAL_NUMBER;
	literar->integer_value = size;

	AST_Binary* count_binop = AST_NEW_EMPTY(AST_Binary);
	count_binop->left = binop->right;
	count_binop->operation = BINOP_TIMES;
	count_binop->right = literar;

	binop->right = count_binop;

	return type;
}

AST_Type_Definition* TypeResolver::find_typedefinition_from_type(AST_Type* type) {
	if (type->kind == AST_TYPE_POINTER) {
		auto pointer = static_cast<AST_Pointer*>(type);
		return find_typedefinition_from_type(pointer->point_type);
	}

	auto typdef = static_cast<AST_Type_Definition*>(type);
	return typdef;
}

void TypeResolver::resolveDirective(AST_Directive* directive) {
	switch (directive->directive_type) {
	case D_IMPORT: {
			if (directive->value0->type == AST_IDENT) {
				AST_Ident* ident = static_cast<AST_Ident*>(directive->value0);
				if (directive->scope != NULL) ident->scope = directive->scope;

				resolveIdent(ident);
			}
		}
		break;
	};
}

AST_Type* TypeResolver::resolveLiteral(AST_Literal* literal) {
	if (literal->value_type == LITERAL_STRING)
		return interpret->type_char;
	return interpret->type_u64;
}

AST_Type* TypeResolver::resolveIdent(AST_Ident* ident) {
	auto type = find_typedefinition(ident, ident->scope);	

	if (type == NULL) {
		if (phase == 1)
			addToResolve(ident); //We don't have this type jet maybe it's defined after this expression
		else if (phase == 2)
			interpret->report_error(ident->name, "Unkown ident '%s'", ident->name->value);
	}

	return type;
}

AST_Type* TypeResolver::resolveUnary(AST_UnaryOp* unary) {
	if (unary->operation == UNOP_DEF) { //*
		auto type = resolveExpression(unary->left);
		/*
		AST_Pointer* pointer = AST_NEW_EMPTY(AST_Pointer);
		pointer->scope = unary->scope;
		pointer->point_to = unary->left;
		pointer->point_type = type;
		*/
		return interpret->type_s64;
	}
	else if (unary->operation == UNOP_REF) { //&
		auto type = resolveExpression(unary->left);
		auto declaration = find_expression_declaration(unary->left);

		AST_Pointer* address = AST_NEW_EMPTY(AST_Pointer);
		address->scope = unary->scope;
		address->point_to = declaration;
		address->point_type = type;

		return address;
	}
	else if (unary->operation == UNOP_CALL) { // ()
		resolveExpression(unary->left);
		resolveExpression(unary->arguments);
		auto declaration = find_expression_declaration(unary->left);	
		if (declaration->value->type == AST_PROCEDURE) {
			unary->left->expression = declaration;
		}
		else {
			interpret->report_error(unary->left->token, "You can call only procedures not '%s'", unary->left->token->value);
		}

		return interpret->type_s64; //WTF?
	}

	assert(false && "this type of unarry not handled");
}

AST_Type* TypeResolver::resolveDeclaration(AST_Declaration* declaration) {
	AST_Type* valueType = NULL;

	if (declaration->value != NULL) {
		valueType = resolveExpression(declaration->value);
	}

	if (declaration->assigmet_type != NULL && declaration->assigmet_type->type == AST_TYPE) {		
		auto type = resolveType(static_cast<AST_Type*>(declaration->assigmet_type));
		declaration->inferred_type = type;
		return type;
	}

	if (declaration->assigmet_type != NULL) {
		AST_Ident* assing_type = NULL;
		if (declaration->assigmet_type->type == AST_IDENT) {
			assing_type = static_cast<AST_Ident*>(declaration->assigmet_type);
		}

		assert(declaration->assigmet_type != NULL);
		if (declaration->flags & AST_DECLARATION_FLAG_CONSTANT) {
			if (declaration->assigmet_type->type == AST_IDENT) {
				assing_type->flags |= AST_IDENT_FLAG_CONSTANT;
			}
		}

		auto type_def = find_typedefinition(assing_type, declaration->scope);
		if (type_def == NULL) {
			addToResolve(declaration);
		}

		declaration->assigmet_type = type_def;
		declaration->inferred_type = type_def;
		return type_def;
	}

	if (declaration->value != NULL) {
		declaration->assigmet_type = valueType;

		if (declaration->value->type == AST_IDENT) {
			auto ident = static_cast<AST_Ident*>(declaration->value);
			if (ident->type_declaration->type == AST_DECLARATION) {
				auto type = find_typedefinition(ident, declaration->scope);
				declaration->value = type;
			}
		}		
		
		if (declaration->assigmet_type->type == AST_TYPE) {
			auto type = static_cast<AST_Type_Definition*>(declaration->assigmet_type);
			declaration->inferred_type = type;
			return type;
		}
	}

	return NULL;
}

AST_Declaration* TypeResolver::find_expression_declaration(AST_Expression* expression) {
	if (expression->type == AST_IDENT) {
		auto ident = static_cast<AST_Ident*>(expression);
		return find_declaration(ident, ident->scope);
	}
	else if (expression->type == AST_UNARYOP) {
		auto unary = static_cast<AST_UnaryOp*>(expression);
		return find_expression_declaration(unary->left);
	}
	else if (expression->type == AST_BINARYOP) {
		auto binary = static_cast<AST_Binary*>(expression);
		return find_expression_declaration(binary->left);
	}

	assert(false);
}

AST_Declaration* TypeResolver::find_declaration(AST_Ident* ident, AST_Block* scope) {
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

AST_Type* TypeResolver::find_typedefinition(AST_Ident* ident, AST_Block* scope) {
	auto name = ident->name->value;

	if (scope->belongs_to == AST_BLOCK_BELONGS_TO_PROCEDURE) {
		auto in_header = find_typedefinition(ident, scope->belongs_to_procedure->header);
		if (in_header != NULL)
			return in_header;
	}

	for (auto i = 0; i < scope->expressions.size(); i++) {
		AST_Expression* it = scope->expressions[i];

		if (it->type == AST_DECLARATION) {
			AST_Declaration* declaration = static_cast<AST_Declaration*>(it);
			if (declaration->ident->name->value == name) {
				ident->type_declaration = declaration;

				if (declaration->flags & AST_DECLARATION_FLAG_CONSTANT) {
					ident->flags |= AST_IDENT_FLAG_CONSTANT;
				}				

				if (declaration->assigmet_type != NULL && declaration->assigmet_type->type == AST_TYPE) {
					AST_Type* type = static_cast<AST_Type*>(declaration->assigmet_type);
					return resolveType(type);
				}
				else if (declaration->assigmet_type != NULL && declaration->assigmet_type->type == AST_IDENT) {
					return resolveIdent(static_cast<AST_Ident*>(declaration->assigmet_type));
				}
				else if (declaration->value != NULL && declaration->value->type == AST_TYPE_DEFINITION) {
					return static_cast<AST_Type_Definition*>(declaration->value);
				}
				else {					
					//assert(false && "only handling ast type definition");					
					return NULL;
				}
			}
		}
		else if (it->type == AST_IDENT) {
			AST_Ident* ident = static_cast<AST_Ident*>(it);
			if (ident->name->value == name) {
				auto definition = find_typedefinition(ident, ident->scope);
				if (definition != NULL) {
					return definition;
				}
			}
		}
	}

	if (scope->flags & AST_BLOCK_FLAG_MAIN_BLOCK)
		return NULL;
	
	if (scope->scope != NULL)
		return find_typedefinition(ident, scope->scope);

	return NULL;
}

int TypeResolver::find_internal_typeinition(Token* value) {
	auto name = value->value.data;

	if (COMPARE(name, "int")) {
		return TYPE_DEFINITION_NUMBER;
	}
	if (COMPARE(name, "char")) {
		return TYPE_DEFINITION_STRING;
	}

	return NULL;
}