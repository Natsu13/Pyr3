#pragma once

#include "Parser.h"
#include "Headers.h"
#include "Utils.h"
#include "Lexer.h"
#include "Interpret.h"

Parser::Parser(Interpret* interpret, Lexer* lexer) {
	this->interpret = interpret;
	this->lexer = lexer;
}

AST_Block* Parser::parse() {
	current_scope = AST_NEW(AST_Block);
	current_scope->flags |= AST_BLOCK_FLAG_MAIN_BLOCK;	

	return parse_block();
}

AST_Block* Parser::parse_block() {
	AST_Block* scope = current_scope;
	while (lexer->peek_next_token()->type != TOKEN_EOF && lexer->peek_next_token()->type != '}') {
		if (lexer->peek_next_token()->type == ';') {
			lexer->eat_token();
			continue;
		}

		AST_Expression* expression = parse_primary();
		if (expression == NULL) return scope;

		scope->expressions.push_back(expression);
	}
	return scope;
}

AST_Expression* Parser::parse_primary() {
	AST_Expression* left = parse_expression();

	if (left == NULL)
		return NULL;

	Token* token = lexer->peek_next_token();
	if (token->type == ';' || token->type == '}') {
		if (left->type == AST_FUNCTION && token->type == '}') {
			lexer->eat_token();
		}
		return left;
	}

	return parse_binop(0, left);
}

AST_Expression* Parser::parse_expression() {
	Token* token = lexer->peek_next_token();

	if (token->type == TOKEN_IDENT) {
		return parse_ident();
	}
	else if (token->type == TOKEN_STRING) {
		return parse_string();
	}
	else if (token->type == TOKEN_NUMBER) {
		return parse_number();
	}
	else if (token->type == TOKEN_DIRECTIVE) {
		return parse_directive();
	}
	else if (token->type == TOKEN_KEYWORD_IF) {
		return parse_condition();
	}
	else if (token->type == TOKEN_KEYWORD_TRUE || token->type == TOKEN_KEYWORD_FALSE) {
		this->lexer->eat_token();

		AST_Number* num = AST_NEW(AST_Number);
		num->flags |= TOKEN_NUMBER_FLAG_INT;
		num->value = token->type == TOKEN_KEYWORD_TRUE? 1: 0;
		return num;
	}
	else if(token->type == '(') {
		lexer->eat_token();
		/*
		AST_Expression* expression = parse_expression();

		token = lexer->peek_next_token();
		if (token->type != ')') {
			interpret->report_error(token, "Expecting ')' after end of expression");
		}
		lexer->eat_token();
		*/
		return parse_param_or_function();
	}

	interpret->report_error(token, "Unexpected token type '%s'", token_to_string(token->type));
	return NULL;
}

const char* Parser::token_to_string(int type) {

	switch (type) {
	case TOKEN_KEYWORD_IDENT:
		return "ident";
	case TOKEN_KEYWORD_TRUE:
		return "true";
	case TOKEN_KEYWORD_FALSE:
		return "false";
	case TOKEN_KEYWORD_IF:
		return "if";
	case TOKEN_KEYWORD_ELSE:
		return "else";
	case TOKEN_KEYWORD_FOR:
		return "for";
	case TOKEN_KEYWORD_INT:
		return "int";
	case TOKEN_KEYWORD_STRING:
		return "string";
	case TOKEN_KEYWORD_NEW:
		return "new";
	case TOKEN_KEYWORD_FLOAT:
		return "float";
	case TOKEN_KEYWORD_LONG:
		return "long";
	case TOKEN_KEYWORD_RETURN:
		return "return";
	case TOKEN_KEYWORD_ENUM:
		return "enum";
	case TOKEN_KEYWORD_STRUCT:
		return "struct";
	case TOKEN_KEYWORD_DEFER:
		return "defer";
	case TOKEN_KEYWORD_CONSTRUCTOR:
		return "constructor";
	case TOKEN_KEYWORD_DESCRUCTOR:
		return "destructor";
	case TOKEN_KEYWORD_S16:
		return "s16";
	case TOKEN_KEYWORD_S32:
		return "s32";
	case TOKEN_KEYWORD_S64:
		return "s64";
	}

	auto char_type = std::to_string(type);
	return char_type.c_str();
}

bool Parser::is_typedef_keyword() {
	Token* token = lexer->peek_next_token();
	int type = token->type;
	if (type == TOKEN_KEYWORD_INT || type == TOKEN_KEYWORD_FLOAT || type == TOKEN_KEYWORD_LONG
		|| type == TOKEN_KEYWORD_S16 || type == TOKEN_KEYWORD_S32 || type == TOKEN_KEYWORD_S64
		|| type == TOKEN_KEYWORD_STRING)
		return true;

	return false;
}

AST_Type_Definition* Parser::parse_type_definition() {
	Token* token = lexer->peek_next_token();
	int type = token->type;

	this->lexer->eat_token();

	switch (type) {
	case TOKEN_KEYWORD_INT: {
			return interpret->type_def_int;
		}break;
	case TOKEN_KEYWORD_FLOAT: {
		return interpret->type_def_float;
		}break;
	case TOKEN_KEYWORD_LONG: {
			return interpret->type_def_long;
		}break;
	case TOKEN_KEYWORD_STRING: {
			return interpret->type_def_char;
		}break;
	case TOKEN_KEYWORD_S16: {
			return interpret->type_def_s16;
		}break;
	case TOKEN_KEYWORD_S32: {
			return interpret->type_def_s32;
		}break;
	case TOKEN_KEYWORD_S64: {
			return interpret->type_def_s64;
		}break;
	}

	assert(false);
	return NULL;
}

AST_Condition* Parser::parse_condition() {
	lexer->eat_token();

	AST_Condition* condition = AST_NEW(AST_Condition);

	condition->condition = parse_primary();
	condition->body_pass = parse_block_or_expression();

	Token* token = lexer->peek_next_token();
	if (token->type == TOKEN_KEYWORD_ELSE) {
		lexer->eat_token();

		condition->body_fail = parse_block_or_expression();
	}

	return condition;
}

AST_Expression* Parser::parse_block_or_expression() {
	if (lexer->peek_next_token()->type == '{') {
		lexer->eat_token();
		AST_Block* block = parse_block();
		lexer->eat_token();
		return block;
	}

	return parse_expression();
}

void Parser::parse_directives(vector<AST_Directive*>& directives) {
	while (this->lexer->peek_next_token()->type == TOKEN_DIRECTIVE) {
		directives.push_back(parse_directive());
	}
}

AST_Directive* Parser::parse_directive() {
	this->lexer->eat_token();

	AST_Directive* directive = AST_NEW(AST_Directive);	
	directive->name = parse_string();

	Token* token = NULL;
	auto name = directive->name->value.c_str();

	if (COMPARE(name, L"import")) {
		directive->directive_type = D_IMPORT;

		token = lexer->peek_next_token();
		if (token->type != TOKEN_STRING) {
			interpret->report_error(token, "import procedure need name of library or pyr file as string");
			return NULL;
		}

		directive->value0 = parse_string();
	}
	else {
		interpret->report_error(token, "unkown directive");
	}

	return directive;
}

bool Parser::parse_arguments(AST_Block* block) {
	Token* token = lexer->peek_next_token();

	if (token->type == '\0') return false;

	bool isProcDeclaration = true;

	while (token->type != ')') {
		AST_Expression* exp = parse_expression();
		if (exp == NULL) {
			return true;
		}
		block->expressions.push_back(exp);

		if (isProcDeclaration && exp->type != AST_DECLARATION) {
			isProcDeclaration = false;
		}

		token = lexer->peek_next_token();
		if (token->type == ',') {
			lexer->eat_token();
			token = lexer->peek_next_token();
			if (token->type == ')') {
				interpret->report_error(token, "Missing expression after comma in parameters");
				break;
			}
		}
	}
	
	assert(token->type == ')');

	lexer->eat_token();		

	return isProcDeclaration;
}

AST_Expression* Parser::parse_param_or_function() {
	Token* token = lexer->peek_next_token();
	AST_Block* header = AST_NEW(AST_Block);

	if (!parse_arguments(header)) {
		return header->expressions[0];
	}
	
	//it must be declaration of procedue
	AST_Function* function = AST_NEW(AST_Function);
	function->header = header;

	token = lexer->peek_next_token();

	if (token->type == TOKEN_RETURNTYPE) {
		lexer->eat_token();
		token = lexer->peek_next_token();

		if (token->type == '{' || token->type == ';') {
			interpret->report_error(token, "You need to defined return type after '->'");
		}
		else {
			function->returnType = parse_ident();
		}
	}
	token = lexer->peek_next_token();

	vector<AST_Directive*> directive;
	while (token->type == TOKEN_DIRECTIVE) {
		
		lexer->eat_token();
		token = lexer->peek_next_token();
		auto name = token->value.c_str();

		if (COMPARE(name, L"compiler"))
			function->flags |= AST_FUNCTION_FLAG_COMPILER;
		else if (COMPARE(name, L"internal"))
			function->flags |= AST_FUNCTION_FLAG_INTERNAL;
		else if (COMPARE(name, L"native"))
			function->flags |= AST_FUNCTION_FLAG_NATIVE;
		else if (COMPARE(name, L"foreign")) {
			function->flags |= AST_FUNCTION_FLAG_FOREIGN;

			lexer->eat_token();
			token = lexer->peek_next_token();
			if (token->type != TOKEN_STRING) {
				function->foreign_library = parse_string();
			}
			else if (token->type != TOKEN_IDENT) {
				function->foreign_library_expression = parse_expression();
			}
			else {
				interpret->report_error(token, "foreign procedure need name of DLL");
			}			
		}
		else {
			interpret->report_error(token, "unkown directive in procedure");			
		}

		lexer->eat_token();
		token = lexer->peek_next_token();
	}

	AST_Block* block = NULL;
	if (token->type == '{') {
		block = AST_NEW(AST_Block);

		lexer->eat_token();
		token = lexer->peek_next_token();

		AST_Block* scope = current_scope;
		current_scope = block;
		parse_block();
		current_scope = scope;
	}
		
	function->body = block;
	return function;
}

AST_Expression* Parser::parse_binop(int prec, AST_Expression* left) {
	while (true) {
		int tokprec = get_current_token_prec();

		if (tokprec < prec) {
			return left;
		}

		Token* binop_token = lexer->peek_next_token();
		lexer->eat_token();
		char binop = 0;
		auto vbop = binop_token->type;

		if (vbop == TOKEN_EQUAL) {
			binop = BINOP_ISEQUAL;
		}else if(vbop == TOKEN_NOTEQUAL) {
			binop = BINOP_ISNOTEQUAL;
		} else if (vbop == TOKEN_PLUS) {
			binop = BINOP_PLUS;
		} else if (vbop == TOKEN_MINUS) {
			binop = BINOP_MINUS;
		} else if (vbop == TOKEN_DIV) {
			binop = BINOP_DIV;
		} else if (vbop == TOKEN_MUL) {
			binop = BINOP_TIMES;
		} else if (vbop == TOKEN_MOD) {
			binop = BINOP_MOD;
		} else if (vbop == TOKEN_MORE) {
			binop = BINOP_GREATER;
		} else if (vbop == TOKEN_MOREEQUAL) {
			binop = BINOP_GREATEREQUAL;
		} else if (vbop == TOKEN_LESS) {
			binop = BINOP_LESS;
		} else if (vbop == TOKEN_LESSEQUAL) {
			binop = BINOP_LESSEQUAL;
		} else if (vbop == TOKEN_AND) {
			binop = BINOP_LOGIC_AND;
		} else if (vbop == TOKEN_OR) {
			binop = BINOP_LOGIC_OR;
		} else if (vbop == TOKEN_BAND) {
			binop = BINOP_BITWISE_AND;
		} else if (vbop == TOKEN_BOR) {
			binop = BINOP_BITWISE_OR;
		} else {
			interpret->report_error(binop_token, "Unkown binary operation");
		}

		AST_Expression* right = parse_primary();
		if (right == NULL) {
			interpret->report_error(lexer->peek_next_token(), "Binary operation need right expression");
		}

		AST_BinaryOp* result = AST_NEW(AST_BinaryOp);
		result->left = left;
		result->right = right;
		result->operation = binop;

		left = result;
	}
}

int Parser::get_current_token_prec() {
	Token* token = lexer->peek_next_token();

	if (token->type == '<')				return 10;
	if (token->type == '>')				return 10;
	if (token->type == TOKEN_LESSEQUAL)	return 10;
	if (token->type == TOKEN_MOREEQUAL)	return 10;
	if (token->type == '+')				return 20;
	if (token->type == '-')				return 20;
	if (token->type == '*')				return 40;
	if (token->type == '/')				return 40;
	if (token->type == '%')				return 40;	
	if (token->type == TOKEN_BAND)		return 60;
	if (token->type == TOKEN_BOR)		return 60;
	if (token->type == TOKEN_EQUAL)		return 80;
	if (token->type == TOKEN_NOTEQUAL)	return 80;
	if (token->type == TOKEN_AND)		return 100;
	if (token->type == TOKEN_OR)		return 100;

	return -1;
}

AST_Expression* Parser::parse_ident() {

	AST_Ident* ident = crate_ident_from_current_token();

	auto token = lexer->peek_next_token();
	if (token->type == ':') { //Create variable
		AST_Declaration* declaration = AST_NEW(AST_Declaration);
		declaration->ident = ident;

		lexer->eat_token();
		token = lexer->peek_next_token();
		
		AST_Ident* type = NULL;
		bool is_typedef = this->is_typedef_keyword();
		if (token->type == TOKEN_IDENT || is_typedef) {
			if (is_typedef) {
				declaration->assigmet_type = parse_type_definition();
			}
			else {
				type = crate_ident_from_current_token();
				declaration->assigmet_type = type;
			}
			token = lexer->peek_next_token();
		}

		if (token->type == ':') { //Constant
			declaration->flags |= AST_DECLARATION_FLAG_CONSTANT;
		}
		else if (token->type == '=') { //Instantiate variable and fill it
			
		}
		else {
			return declaration;
		}
		lexer->eat_token();		

		AST_Expression* value = parse_primary();
		declaration->value = value;

		return declaration;
	}
	else if (token->type == '=') { //Assign to variable		
		return parse_assigment(ident);
	}
	else if (token->type == '(') { //Call function
		lexer->eat_token();

		AST_UnaryOp* call = AST_NEW(AST_UnaryOp);
		call->left = ident;
		call->arguments = AST_NEW(AST_Block);
		call->operation = token;
		parse_arguments(call->arguments);
		return call;
	}

	return ident;
}

AST_BinaryOp* Parser::parse_assigment(AST_Ident* left) {
	//Token* operation = lexer->peek_next_token();
	lexer->eat_token();

	AST_BinaryOp* assigment = AST_NEW(AST_BinaryOp);
	assigment->left = left;
	assigment->operation = BINOP_ASIGN;
	assigment->right = parse_primary();

	return assigment;
}

AST_Ident* Parser::crate_ident_from_current_token() {
	Token* name = this->lexer->peek_next_token();
	this->lexer->eat_token();

	AST_Ident* ident = AST_NEW(AST_Ident);
	ident->name = name;
	return ident;
}

AST_String* Parser::parse_string() {
	Token* token = this->lexer->peek_next_token();
	this->lexer->eat_token();

	AST_String* str = AST_NEW(AST_String);
	str->value = token->value;
	str->name = token;

	return str;
}

AST_Number* Parser::parse_number() {
	auto token = this->lexer->peek_next_token();
	this->lexer->eat_token();

	AST_Number* num = AST_NEW(AST_Number);
	num->flags = token->number_flags;

	if (token->number_flags & TOKEN_NUMBER_FLAG_INT) {
		if (token->number_flags & TOKEN_NUMBER_FLAG_HEX) {
			num->value = token->number_value_l;
		}
		else {
			num->value = token->number_value_i;
		}
	}
	else if (token->number_flags & TOKEN_NUMBER_FLAG_LONG) {
		num->value = token->number_value_l;
	}
	else if (token->number_flags & TOKEN_NUMBER_FLAG_FLOAT) {
		num->value = floatToInteger(token->number_value_f);
	}

	return num;
}