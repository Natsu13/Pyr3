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

	AST_Block* parse_block(bool new_scope = false);
	AST_Expression* parse_primary();
	AST_Expression* parse_primary(int prec);
	AST_Expression* parse_expression();
	AST_Expression* parse_ident();
	AST_Binary* parse_assigment(AST_Expression* left);
	AST_Literal* parse_string();
	AST_Literal* parse_number();
	AST_Condition* parse_condition();
	AST_Directive* parse_directive();
	AST_Expression* parse_dereference(AST_Ident* ident = NULL);
	AST_Struct* parse_struct();
	void parse_member_struct(AST_Struct* _struct);
	AST_Return* parse_return();
	void parse_directives(vector<AST_Directive*> &directives);
	AST_Expression* parse_binop(int prec, AST_Expression* left);
	AST_Expression* parse_param_or_function();
	AST_Expression* parse_block_or_expression();
	AST_Type* parse_type();
	AST_Expression* parse_type_array(AST_Expression* type);
	AST_Type* parse_typedefinition();
	AST_Expression* parse_typedefinition_or_ident();
	bool parse_arguments(AST_Block* block);
	bool is_typedef_keyword();
	const char* token_to_string(int type);

	AST_Ident* create_ident_from_current_token();

	int get_current_token_prec();
public:
	Parser(Interpret* interpret, Lexer* lexer, TypeResolver* type_resolver);
	AST_Block* parse();
};