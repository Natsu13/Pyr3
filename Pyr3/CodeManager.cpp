#pragma once

#include "CodeManager.h"

//This will resolver stuff like all types struct, string, int, function
CodeManager::CodeManager(Interpret* interpret, TypeResolver* typeResolver) {
	this->interpret = interpret;
	this->typeResolver = typeResolver;
}

void CodeManager::manage(AST_Block* block) {
	if (block == NULL) return;

	For(block->expressions) {
		manageExpression(it);
	}
}

void CodeManager::manageExpression(AST_Expression* expression) {
	switch (expression->type) {
		case AST_IDENT: {
			break;
		}
		case AST_CONDITION: {
			manageCondition((AST_Condition*)expression);
			break;
		}
		case AST_BLOCK: {
			manage((AST_Block*) expression);
			break;
		}
		case AST_DECLARATION: {
			manageDeclaration((AST_Declaration*)expression);
			break;
		}
		case AST_TYPE: {
			break;
		}
		case AST_LITERAL: {
			break;
		}
		case AST_BINARY: {
			manageBinary((AST_Binary*)expression);
			break;
		}
		case AST_PROCEDURE: {
			manageProcedure((AST_Procedure*)expression);
			break;
		}
		case AST_PARAMLIST: {
			break;
		}
		case AST_UNARYOP: {
			manageUnary((AST_Unary*)expression);
			break;
		}
		case AST_RETURN: {
			break;
		}
		case AST_DIRECTIVE: {
			break;
		}
		case AST_CAST: {
			break;
		}
		case AST_WHILE: {
			break;
		}
		case AST_OPERATOR: {
			break;
		}
		case AST_FOR: {
			break;
		}
		case AST_RANGE: {
			break;
		}
		case AST_TYPESIZEOF: {
			auto ast = (AST_TypeSizeOf*)expression;
			auto type = typeResolver->find_typeof(ast->of);
			ast->substitution = type;
			break;
		}
		default:
			assert(false);
			break;
	}
}

#define E(expression) if (expression != NULL) { manageExpression(expression); }
#define B(block) if (block != NULL) { manage(block); }

void CodeManager::manageCondition(AST_Condition* condition) {
	E(condition->condition);
	E(condition->body_pass);
	E(condition->body_fail);
}

void CodeManager::manageBinary(AST_Binary* binary) {
	E(binary->left);
	E(binary->right);
}

void CodeManager::manageProcedure(AST_Procedure* procedure) {
	E(procedure->header);
	B(procedure->body);
	E(procedure->returnType);
}

void CodeManager::manageDeclaration(AST_Declaration* declaration) {
	E(declaration->value);
}

void CodeManager::manageUnary(AST_Unary* unary) {
	if (unary->operation == UNOP_CALL) {
		B(unary->arguments);
		return;
	}

	return;
	//assert(false && "this type of unary not handled");
}

//lexer->current_line_number
//current_file_name
//add_directive_import AST_Directive_Import
//add_directive_load AST_Directive_Load