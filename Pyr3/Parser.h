#pragma once

#include <cstddef>
#include "Headers.h"
#include "Token.h"
#include "Lexer.h"
#include "TypeResolver.h"

class Parser
{
private:
	Interpret* interpret;
	Lexer* lexer;	
	TypeResolver* type_resolver;
	AST_Block* current_scope = NULL;	
	AST_Block* main_scope = NULL;

	AST_Block* parse_block(bool new_scope = false);
	AST_Expression* parse_primary();
	AST_Expression* parse_primary(int prec, bool possibly_paramlist = true);
	AST_Expression* parse_expression();
	AST_Expression* parse_ident();
	AST_Binary* parse_assigment(AST_Expression* left);
	AST_Literal* parse_string();
	AST_Literal* parse_number();
	AST_While* parse_while();
	AST_For* parse_for();
	AST_Block* parse_list();
	AST_Condition* parse_condition();
	AST_Directive* parse_directive();
	AST_Expression* parse_dereference(AST_Expression* ident = NULL);
	AST_TypeSizeOf* parse_typesizeof();

	bool expect_and_eat_bracket(bool simple = true);
	void assign_to_ident_or_paramlist(AST_Expression* &ident_or_paramlist, AST_Expression* expression);
	
	AST_Struct* parse_struct();	
	void parse_member_struct(AST_Struct* _struct);
	
	AST_Enum* parse_enum();
	void parse_member_enum(AST_Enum* _enum);

	AST_Return* parse_return();
	AST_Cast* parse_cast();
	AST_Operator* parse_operator();
	void parse_directives(vector<AST_Directive*> &directives);
	AST_Expression* parse_binop(int prec, AST_Expression* left);
	AST_Expression* parse_param_or_function();
	AST_Expression* parse_block_or_expression();
	AST_Type* parse_type();
	AST_Expression* parse_type_array(AST_Expression* type);
	AST_Expression* parse_ident_array(AST_Expression* ident);
	AST_Type* parse_typedefinition();
	AST_Expression* parse_typedefinition_or_ident();
	AST_Expression* parse_type_or_paramlist();
	bool parse_arguments(AST_Block* block);
	bool is_typedef_keyword();

	AST_Ident* create_ident_from_current_token();

	int get_current_token_prec();
public:
	Parser(Interpret* interpret, Lexer* lexer, TypeResolver* type_resolver);
	AST_Block* parse();
};