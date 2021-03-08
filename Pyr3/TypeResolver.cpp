#pragma once

#include "Headers.h"
#include "TypeResolver.h"

TypeResolver::TypeResolver(Interpret* interpret) {
	this->interpret = interpret;
}

AST_Literal* TypeResolver::make_string_literal(CString value) {
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
	for (int i = 0; i < to_be_resolved.size(); i++) {
		AST_Expression* it = to_be_resolved[i];

		resolveExpression(it, NULL);
	}
}

void TypeResolver::resolve(AST_Block* block) {
	for (auto index = 0; index < block->expressions.size(); index++) {
		auto it = block->expressions[index];

		resolveExpression(it, block);
	}
}

int TypeResolver::resolveExpression(AST_Expression* expression, AST_Block* block) {
	switch (expression->type) {
	case AST_IDENT: {
		AST_Ident* ident = static_cast<AST_Ident*>(expression);
		if (block != NULL) ident->scope = block;

		return resolveIdent(ident);
	}
	case AST_BLOCK: {
		AST_Block* _block = static_cast<AST_Block*>(expression);
		if (block != NULL) _block->scope = block;
		resolve(_block);

		return NULL;
	}
	case AST_DECLARATION: {
		AST_Declaration* declaration = static_cast<AST_Declaration*>(expression);
		if (block != NULL) declaration->scope = block;

		return resolveDeclaration(declaration);
	}
	case AST_TYPE_DEFINITION:
		break;
	case AST_LITERAL: {
		AST_Literal* literal = static_cast<AST_Literal*>(expression);
		return resolveLiteral(literal);
	}
	case AST_BINARYOP: {
		AST_BinaryOp* binop = static_cast<AST_BinaryOp*>(expression);
		if (binop != NULL) binop->scope = block;

		return resolveBinop(binop);
	}
	case AST_PROCEDURE:
		break;
	case AST_PARAMLIST:
		break;
	case AST_UNARYOP:
		break;
	case AST_DIRECTIVE: {
		AST_Directive* directive = static_cast<AST_Directive*>(expression);
		if (block != NULL) directive->scope = block;

		resolveDirective(directive);
		return NULL;
	}
	default:
		assert(false);
		break;
	}

	return NULL;
}

int TypeResolver::resolveBinop(AST_BinaryOp* binop) {
	resolveExpression(binop->left, binop->scope);
	resolveExpression(binop->right, binop->scope);

	return NULL;
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

int TypeResolver::resolveLiteral(AST_Literal* literal) {
	if (literal->value_type == LITERAL_STRING)
		return TYPE_DEFINITION_STRING;
	return TYPE_DEFINITION_NUMBER;
}

int TypeResolver::resolveIdent(AST_Ident* ident) {
	int type = find_type_definition(ident, ident->scope);

	if(type == NULL) //We don't have this type jet maybe it's defined after this expression
		addToResolve(ident);

	return type;
}

int TypeResolver::resolveDeclaration(AST_Declaration* declaration) {
	if (declaration->type_definition != NULL)
		return declaration->type_definition;

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

		if (declaration->assigmet_type->type == AST_IDENT) {
			declaration->type_definition = TYPE_DEFINITION_IDENT;
		}
		if (declaration->type_definition != NULL) {
			return declaration->type_definition;
		}
	}

	if (declaration->value != NULL) {
		declaration->type_definition = resolveExpression(declaration->value, declaration->scope);
		return declaration->type_definition;
	}

	return -1;
}

int TypeResolver::find_type_definition(AST_Ident* ident, AST_Block* block) {
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

				if (declaration->type_definition != NULL) {
					return declaration->type_definition;
				}
			}
		}
		else if (it->type == AST_IDENT) {
			AST_Ident* ident = static_cast<AST_Ident*>(it);
			if (ident->name->value == name) {
				int definition = find_internal_type_definition(ident->name);
				if (definition != NULL) {
					return definition;
				}
			}
		}
	}

	if (block->flags & AST_BLOCK_FLAG_MAIN_BLOCK) 
		return NULL;
	
	if (block->scope != NULL)
		return find_type_definition(ident, block->scope);

	return NULL;
}

int TypeResolver::find_internal_type_definition(Token* value) {
	auto name = value->value.data;

	if (COMPARE(name, "int")) {
		return TYPE_DEFINITION_NUMBER;
	}
	if (COMPARE(name, "char")) {
		return TYPE_DEFINITION_STRING;
	}

	return NULL;
}