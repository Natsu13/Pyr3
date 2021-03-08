#pragma once

#include <cstddef>
#include "Headers.h"
#include "Token.h"
#include "Lexer.h"
#include "TypeResolver.h"

class Parser
{
private:
	TypeResolver* type_resolver;
	Interpret* interpret;
	Lexer* lexer;	
	AST_Block* current_scope = NULL;	

	AST_Block* parse_block();
	AST_Expression* parse_primary();
	AST_Expression* parse_expression();
	AST_Expression* parse_ident();
	AST_BinaryOp* parse_assigment(AST_Ident* left);
	AST_Literal* parse_string();
	AST_Literal* parse_number();
	AST_Condition* parse_condition();
	AST_Directive* parse_directive();
	void parse_directives(vector<AST_Directive*> &directives);
	AST_Expression* parse_binop(int prec, AST_Expression* left);
	AST_Expression* parse_param_or_function();
	AST_Expression* parse_block_or_expression();
	AST_Type_Definition* parse_type_definition();
	bool parse_arguments(AST_Block* block);
	bool is_typedef_keyword();
	const char* token_to_string(int type);

	AST_Ident* crate_ident_from_current_token();

	int get_current_token_prec();
public:
	Parser(Interpret* interpret, Lexer* lexer, TypeResolver* type_resolver);
	AST_Block* parse();
};