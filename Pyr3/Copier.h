#pragma once
#include "Headers.h"
#include "Interpret.h"

class Copier
{
private:
	Interpret* interpret;
	AST_Block* current_scope = NULL;
public:
	Copier(Interpret* interpret);
	AST_Expression* copy_expression(AST_Expression* expression, AST_Expression* expression_copy);
	AST_Expression* copy(AST_Expression* expression);
	AST_Procedure* copy_procedure(AST_Procedure* procedure);
	AST_For* copy_for(AST_For* ast_for);
	AST_Block* copy_block(AST_Block* block);
	AST_Declaration* copy_declaration(AST_Declaration* declaration);
	AST_Ident* copy_ident(AST_Ident* ident);
	AST_Type* copy_type(AST_Type* type);
	AST_Return* copy_return(AST_Return* ast_return);
	AST_Literal* copy_literal(AST_Literal* ast_literal);
	AST_Binary* copy_binary(AST_Binary* binary);

	Token* copy_token(Token* token);
};

