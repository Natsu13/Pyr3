#include "Copier.h"

Copier::Copier(Interpret* interpret) {
	this->interpret = interpret;
}

Token* Copier::copy_token(Token* token) {
	auto news = new Token();
	news->file_name = token->file_name;
	news->column = token->column;
	news->row = token->row;
	news->value = token->value;
	return news;
}

#define AST_COPY(type, expression) (type*)copy_expression(expression, AST_NEW_EMPTY(type));

AST_Expression* Copier::copy_expression(AST_Expression* expression, AST_Expression* expression_copy) {
	expression_copy->flags = expression->flags;	

	if(expression->substitution != NULL)
		expression_copy->substitution = copy(expression->substitution);

	//if (expression->scope != NULL) //we need to replace the scope to the new copy scope!
	//	expression_copy->scope = expression->scope; //no copy!
	if(current_scope == NULL)
		expression_copy->scope = expression->scope; //idk this is about return in procedure
	else
		expression_copy->scope = current_scope;

	expression_copy->line_number = expression->line_number;
	expression_copy->character_number = expression->character_number;
	expression_copy->file_name = expression->file_name;
	expression_copy->serial = interpret->counter++;

	return expression_copy;
}

AST_Expression* Copier::copy(AST_Expression* expression) {
	if (expression->type == AST_PROCEDURE) {
		return copy_procedure((AST_Procedure*)expression);
	}
	else if (expression->type == AST_BLOCK) {
		return copy_block((AST_Block*)expression);
	}
	else if (expression->type == AST_FOR) {
		return copy_for((AST_For*)expression);
	}
	else if (expression->type == AST_IDENT) {
		return copy_ident((AST_Ident*)expression);
	}
	else if (expression->type == AST_DECLARATION) {
		return copy_declaration((AST_Declaration*)expression);
	}
	else if (expression->type == AST_TYPE) {
		return copy_type((AST_Type*)expression);
	}
	else if (expression->type == AST_RETURN) {
		return copy_return((AST_Return*)expression);
	}
	else if (expression->type == AST_LITERAL) {
		return copy_literal((AST_Literal*)expression);
	}
	else if (expression->type == AST_BINARY) {
		return copy_binary((AST_Binary*)expression);
	}

	assert(false && "This expression can't be copy");
}

AST_Binary* Copier::copy_binary(AST_Binary* binary) {
	AST_Binary* binary_copy = AST_COPY(AST_Binary, binary);
	
	binary_copy->operation = binary->operation;
	binary_copy->left = copy(binary->left);
	binary_copy->right = copy(binary->right);

	return binary_copy;
}

AST_Literal* Copier::copy_literal(AST_Literal* ast_literal) {
	AST_Literal* ast_literal_copy = AST_COPY(AST_Literal, ast_literal);

	ast_literal_copy->value_type = ast_literal->value_type;
	ast_literal_copy->number_flags = ast_literal->number_flags;

	if (ast_literal->value_type == LITERAL_NUMBER) {
		ast_literal_copy->integer_value = ast_literal->integer_value;
	}
	else if (ast_literal->value_type == LITERAL_FLOAT) {
		ast_literal_copy->float_value = ast_literal->float_value;
	}
	else if (ast_literal->value_type == LITERAL_STRING) {
		ast_literal_copy->string_value = String(ast_literal_copy->string_value);
	}
	else {
		assert("Unkown literal type" && false);
	}	

	return ast_literal_copy;
}

AST_Return* Copier::copy_return(AST_Return* ast_return) {
	AST_Return* ast_return_copy = AST_COPY(AST_Return, ast_return);

	if (ast_return->value != NULL)
		ast_return_copy->value = copy(ast_return->value);

	return ast_return_copy;
}

AST_Procedure* Copier::copy_procedure(AST_Procedure* procedure) {
	AST_Procedure* procedure_copy = AST_COPY(AST_Procedure, procedure);

	procedure_copy->header = copy_block(procedure->header);
	procedure_copy->header->belongs_to_procedure = procedure_copy; //@todo: idk??
	
	if (procedure->body != NULL) {
		procedure_copy->body = copy_block(procedure->body);
		procedure_copy->body->belongs_to_procedure = procedure_copy;
	}

	procedure_copy->token = copy_token(procedure->token);

	if (procedure->name != NULL)
		procedure_copy->name = copy_token(procedure->name);

	if (procedure->returnType != NULL) {
		procedure_copy->returnType = copy(procedure->returnType);
		procedure_copy->returnType->scope = procedure_copy->header;
	}

	if (procedure->foreign_library_expression != NULL)
		procedure_copy->foreign_library_expression = copy(procedure->foreign_library_expression);

	return procedure_copy;
}

AST_Type* Copier::copy_type(AST_Type* type) {
	if (type->kind == AST_TYPE_DEFINITION) {
		return type;
	}
	if (type->kind == AST_TYPE_GENERIC) {
		return NULL; //don't copy this
	}

	assert(false);
}

AST_Ident* Copier::copy_ident(AST_Ident* ident) {
	AST_Ident* ident_copy = AST_COPY(AST_Ident, ident);

	ident_copy->name = copy_token(ident->name);

	if(ident->type_declaration != NULL)
		ident_copy->type_declaration = copy_declaration(ident->type_declaration);

	return ident_copy;
}

AST_Declaration* Copier::copy_declaration(AST_Declaration* declaration) {
	AST_Declaration* declaration_copy = AST_COPY(AST_Declaration, declaration);

	declaration_copy->ident = copy_ident(declaration->ident);
	declaration_copy->assigmet_type = copy(declaration->assigmet_type);

	if(declaration->inferred_type != NULL)
		declaration_copy->inferred_type = copy_type(declaration->inferred_type);

	if(declaration->value != NULL)
		declaration_copy->value = copy(declaration->value);

	return declaration_copy;
}

AST_For* Copier::copy_for(AST_For* ast_for) {
	AST_For* ast_for_copy = AST_COPY(AST_For, ast_for);

	if (ast_for->key != NULL)
		ast_for_copy->key = copy_declaration(ast_for->key);

	if (ast_for->value != NULL)
		ast_for_copy->value = copy_declaration(ast_for->value);

	ast_for_copy->each = copy(ast_for->each);

	ast_for_copy->header = copy_block(ast_for->header);
	ast_for_copy->header->belongs_to_for = ast_for_copy;

	ast_for_copy->block = copy_block(ast_for->block);
	ast_for_copy->block->belongs_to_for = ast_for_copy;

	return ast_for_copy;
}

AST_Block* Copier::copy_block(AST_Block* block) {
	AST_Block* block_copy = AST_COPY(AST_Block, block);

	auto old_scope = current_scope;
	current_scope = block_copy;

	block_copy->belongs_to = block->belongs_to;
	block_copy->belongs_to_procedure = block->belongs_to_procedure;
	block_copy->belongs_to_for = block->belongs_to_for;

	For(block->expressions) {
		auto c = copy(it);

		if(c != NULL)
			block_copy->expressions.push_back(c);
	}

	current_scope = old_scope;

	return block_copy;
}