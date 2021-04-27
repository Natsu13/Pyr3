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

AST_Type_Definition* TypeResolver::resolveExpression(AST_Expression* expression) {
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
			AST_BinaryOp* binop = static_cast<AST_BinaryOp*>(expression);
			return resolveBinary(binop);
		}
		case AST_PROCEDURE: {
			AST_Procedure* procedure = static_cast<AST_Procedure*>(expression);
			resolve(procedure->body);
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

AST_Type_Definition* TypeResolver::resolveType(AST_Type* type) {
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

	assert(false);
	return NULL;
}

AST_Type_Definition* TypeResolver::resolveBinary(AST_BinaryOp* binop) {
	auto type = resolveExpression(binop->left);
	resolveExpression(binop->right);

	return type;
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

AST_Type_Definition* TypeResolver::resolveLiteral(AST_Literal* literal) {
	if (literal->value_type == LITERAL_STRING)
		return interpret->type_char;
	return interpret->type_u64;
}

AST_Type_Definition* TypeResolver::resolveIdent(AST_Ident* ident) {
	auto type = find_typedefinition(ident, ident->scope);

	if (type == NULL) {
		if (phase == 1)
			addToResolve(ident); //We don't have this type jet maybe it's defined after this expression
		else if (phase == 2)
			interpret->report_error(ident->name, "Unkown ident '%s'", ident->name->value);
	}	 

	return type;
}

AST_Type_Definition* TypeResolver::resolveUnary(AST_UnaryOp* unary) {
	if (unary->operation == UNOP_DEF) { //*
		return resolveExpression(unary->left);
	}
	else if (unary->operation == UNOP_REF) { //&
		return resolveExpression(unary->left);
	}

	assert(false && "this type of unarry not handled");
}

AST_Type_Definition* TypeResolver::resolveDeclaration(AST_Declaration* declaration) {
	if (declaration->value != NULL) {
		resolveExpression(declaration->value);
	}

	if (declaration->assigmet_type != NULL && declaration->assigmet_type->type == AST_TYPE) {		
		return resolveType(static_cast<AST_Type*>(declaration->assigmet_type));
	}

	if (declaration->assigmet_type != NULL) {
		AST_Ident* assing_type = NULL;
		if (declaration->assigmet_type->type == AST_IDENT) {
			assing_type = static_cast<AST_Ident*>(declaration->assigmet_type);
		}

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
		return type_def;
	}

	if (declaration->value != NULL) {
		declaration->assigmet_type = resolveExpression(declaration->value);

		if (declaration->value->type == AST_IDENT) {
			auto ident = static_cast<AST_Ident*>(declaration->value);
			if (ident->type_declaration->type == AST_DECLARATION) {
				auto type = find_typedefinition(ident, declaration->scope);
				declaration->value = type;
			}
		}		
		
		if(declaration->assigmet_type->type == AST_TYPE)
			return static_cast<AST_Type_Definition*>(declaration->assigmet_type);
	}

	return NULL;
}

AST_Type_Definition* TypeResolver::find_typedefinition(AST_Ident* ident, AST_Block* block) {
	auto name = ident->name->value;

	for (auto i = 0; i < block->expressions.size(); i++) {
		AST_Expression* it = block->expressions[i];

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
				int definition = find_internal_typeinition(ident->name);
				if (definition != NULL) {
					return NULL;
				}
			}
		}
	}

	if (block->flags & AST_BLOCK_FLAG_MAIN_BLOCK) 
		return NULL;
	
	if (block->scope != NULL)
		return find_typedefinition(ident, block->scope);

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