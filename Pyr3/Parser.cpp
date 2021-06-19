#pragma once

#include "Parser.h"
#include "Headers.h"
#include "Utils.h"
#include "Lexer.h"
#include "Interpret.h"

Parser::Parser(Interpret* interpret, Lexer* lexer, TypeResolver* type_resolver) {
	this->interpret = interpret;
	this->lexer = lexer;
	this->type_resolver = type_resolver;
}

AST_Block* Parser::parse() {
	current_scope = AST_NEW(AST_Block);
	current_scope->flags |= AST_BLOCK_FLAG_MAIN_BLOCK;	

	return parse_block();
}

AST_Block* Parser::parse_block(bool new_scope) {
	AST_Block* scope = current_scope;
	if (new_scope) {
		current_scope = AST_NEW(AST_Block);
	}
	AST_Block* save_scope = current_scope;

	while (lexer->peek_next_token()->type != TOKEN_EOF && lexer->peek_next_token()->type != '}') {
		if (lexer->peek_next_token()->type == ';') {
			lexer->eat_token();
			continue;
		}

		AST_Expression* expression = parse_primary();
		if (expression == NULL) return save_scope;

		save_scope->expressions.push_back(expression);
	}
	if (new_scope) {
		current_scope = scope;
		return save_scope;
	}
	return scope;
}

AST_Expression* Parser::parse_primary() {
	return parse_primary(0);
}

AST_Expression* Parser::parse_primary(int prec) {
	AST_Expression* left = parse_expression();

	if (left == NULL)
		return NULL;	

	Token* token = lexer->peek_next_token();

	if (token->type == ';' || token->type == '}' || token->type == ')') {
		if ((left->type == AST_PROCEDURE && token->type == '}')) {
			lexer->eat_token();
		}
		return left;
	}

	return parse_binop(prec, left);
}

AST_Expression* Parser::parse_expression() {
	Token* token = lexer->peek_next_token();

	if (token->type == TOKEN_MUL || token->type == TOKEN_BAND || token->type == TOKEN_IDENT) { // *test (pointer to) &test (address of)
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
	else if (token->type == TOKEN_KEYWORD_RETURN) {
		return parse_return();
	}
	else if (token->type == TOKEN_KEYWORD_STRUCT) {
		return parse_struct();
	}
	else if (token->type == TOKEN_KEYWORD_TRUE || token->type == TOKEN_KEYWORD_FALSE) {
		this->lexer->eat_token();

		AST_Literal* num = AST_NEW(AST_Literal);
		num->flags |= TOKEN_NUMBER_FLAG_INT;
		num->value_type = LITERAL_NUMBER;
		num->integer_value = token->type == TOKEN_KEYWORD_TRUE? 1: 0;
		return num;
	}
	else if(token->type == '(') {
		lexer->eat_token();
		return parse_param_or_function();
	}

	interpret->report_error(token, "Unexpected token type '%s'", token_to_string(token->type));
	return NULL;
}

void Parser::parse_member_struct(AST_Struct* _struct) {
	lexer->eat_token(); //eat {
	Token* token = lexer->peek_next_token();

	_struct->members = AST_NEW(AST_Block);

	while (token->type != '}' && token->type != TOKEN_EOF) {
		if (token->type == '#') {

		}
		else {
			auto primary = parse_expression();
			_struct->members->expressions.push_back(primary);
		}
		
		lexer->eat_token();
		token = lexer->peek_next_token();
	}

	assert(token->type == '}');
}

AST_Struct* Parser::parse_struct() {
	lexer->eat_token();
	Token* token = lexer->peek_next_token();

	if (token->type != '{') {
		interpret->report_error(token, "Struct members must be inside block");
		return NULL;
	}

	AST_Struct* _struct = AST_NEW(AST_Struct);

	parse_member_struct(_struct);

	token = lexer->peek_next_token();
	assert(token->type == '}');
	lexer->eat_token();

	return _struct;
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

AST_Return* Parser::parse_return() {
	lexer->eat_token();

	AST_Return* ast_return = AST_NEW(AST_Return);
	ast_return->value = parse_primary();

	return ast_return;
}

AST_Expression* Parser::parse_block_or_expression() {
	if (lexer->peek_next_token()->type == '{') {
		lexer->eat_token();
		AST_Block* block = parse_block(true);
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
	auto name = directive->name->string_value.data;

	if (COMPARE(name, "import")) {
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
		AST_Expression* exp = parse_primary();
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
	AST_Procedure* function = AST_NEW(AST_Procedure);
	function->header = header;

	token = lexer->peek_next_token();

	if (token->type == TOKEN_RETURNTYPE) {
		lexer->eat_token();
		token = lexer->peek_next_token();

		if (token->type == '{' || token->type == ';') {
			interpret->report_error(token, "You need to defined return type after '->'");
		}
		else {
			function->returnType = parse_typedefinition_or_ident();
		}
	}
	token = lexer->peek_next_token();

	vector<AST_Directive*> directive;
	while (token->type == TOKEN_DIRECTIVE) {
		
		lexer->eat_token();
		token = lexer->peek_next_token();
		auto name = token->value.data;

		if (COMPARE(name, "compiler"))
			function->flags |= AST_PROCEDURE_FLAG_COMPILER;
		else if (COMPARE(name, "internal"))
			function->flags |= AST_PROCEDURE_FLAG_INTERNAL;
		else if (COMPARE(name, "intrinsic"))
			function->flags |= AST_PROCEDURE_FLAG_INTRINSIC;
		else if (COMPARE(name, "foreign")) {
			function->flags |= AST_PROCEDURE_FLAG_FOREIGN;

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
	
	if (block != NULL) {
		block->belongs_to = AST_BLOCK_BELONGS_TO_PROCEDURE;
		block->belongs_to_procedure = function;
	}

	function->body = block;

	return function;
}

AST_Expression* Parser::parse_binop(int prec, AST_Expression* left) {
	while (true) {
		int tokprec = get_current_token_prec();

		if (tokprec == 0 || tokprec <= prec) {
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


		AST_Expression* right = parse_primary(tokprec);
		if (right == NULL) {
			interpret->report_error(lexer->peek_next_token(), "Binary operation need right expression");
		}

		AST_Binary* result = AST_NEW(AST_Binary);
		result->left = left;
		result->right = right;
		result->operation = binop;

		left = result;
	}
}

AST_Type* Parser::parse_type_array(AST_Type* type) {
	auto token = lexer->peek_next_token();	
	while (token->type == TOKEN_LBRACKET) {
		lexer->eat_token();

		token = lexer->peek_next_token();
		if (token->type == TOKEN_NUMBER) {
			type->array_size.push_back(parse_number());
		}
		else {
			interpret->report_error("Currently si supported only numbers");
		}

		token = lexer->peek_next_token();
		assert(token->type == TOKEN_RBRACKET);
		lexer->eat_token();
		token = lexer->peek_next_token();
	}

	return type;
}

AST_Type* Parser::parse_type() {
	auto type = parse_typedefinition();
		
	auto token = lexer->peek_next_token();
	if (token->type == TOKEN_LBRACKET) {
		return parse_type_array(type);
	}

	return type;
}

AST_Expression* Parser::parse_typedefinition_or_ident() {
	auto type = parse_typedefinition();
	if(type == NULL)
		return create_ident_from_current_token();
	return type;
}

AST_Type* Parser::parse_typedefinition() {
	Token* token = lexer->peek_next_token();
	int type = token->type;

	this->lexer->eat_token();

	switch (type) {
		case TOKEN_MUL: {
			AST_Pointer* pointer = AST_NEW(AST_Pointer);
			pointer->point_to = parse_typedefinition();
			return pointer;
		}
		case TOKEN_BAND: {
			AST_Addressof* addressof = AST_NEW(AST_Addressof);
			addressof->address_of = parse_typedefinition();
			return addressof;
		}
		case TOKEN_KEYWORD_FLOAT: {
			return interpret->type_float;
		}
		case TOKEN_KEYWORD_LONG: {
			return interpret->type_long;
		}
		case TOKEN_KEYWORD_STRING: {
			return interpret->type_char;
		}
		case TOKEN_KEYWORD_S8: {
			return interpret->type_s8;
		}
		case TOKEN_KEYWORD_S16: {
			return interpret->type_s16;
		}
		case TOKEN_KEYWORD_S32: {
			return interpret->type_s32;
		}
		case TOKEN_KEYWORD_S64: {
			return interpret->type_s64;
		}
		case TOKEN_KEYWORD_U8: {
			return interpret->type_u8;
		}
		case TOKEN_KEYWORD_U16: {
			return interpret->type_u16;
		}
		case TOKEN_KEYWORD_U32: {
			return interpret->type_u32;
		}
		case TOKEN_KEYWORD_U64: {
			return interpret->type_u64;
		}
	}

	return NULL;
}

int Parser::get_current_token_prec() {
	Token* token = lexer->peek_next_token();

	switch (token->type) {
		case '<':
		case '>':
		case TOKEN_LESSEQUAL:
		case TOKEN_MOREEQUAL:
			return 10;
		case '+':
		case '-':
			return 20;
		case '*':
		case '/':
		case '%':
			return 30;
		case TOKEN_BAND:
		case TOKEN_BOR:
			return 40;
		case TOKEN_EQUAL:
		case TOKEN_NOTEQUAL:
			return 50;
		case TOKEN_AND:
		case TOKEN_OR:
			return 60;
	}

	return 0;
}

AST_Expression* Parser::parse_dereference(AST_Ident* ident) {
	if (ident == NULL) {
		ident = create_ident_from_current_token();
	}

	AST_Binary* binary = AST_NEW(AST_Binary);
	binary->operation = BINOP_DOT;
	binary->left = ident;

	auto token = lexer->peek_next_token();
	if (token->type == '.' || token->type == '?') {		
		if (token->type == '?') {
			binary->flags |= BINOP_FLAG_NOTHARD;
		}

		lexer->eat_token();
		binary->right = parse_dereference();

		return binary;
	}
	
	return ident;	
}

AST_Expression* Parser::parse_ident() {
	auto token = lexer->peek_next_token();

	if (token->type == TOKEN_MUL) { //Pointer to *test
		lexer->eat_token();

		AST_UnaryOp* unary = AST_NEW(AST_UnaryOp);
		unary->operation = UNOP_DEF;
		unary->left = parse_expression();

		return unary;
	}
	else if (token->type == TOKEN_BAND) { //Address of &test
		lexer->eat_token();

		AST_UnaryOp* unary = AST_NEW(AST_UnaryOp);
		unary->operation = UNOP_REF;
		unary->left = parse_expression();

		return unary;
	}

	AST_Ident* ident = create_ident_from_current_token();
	token = lexer->peek_next_token();

	if (token->type == '.') { // a.b.c
		AST_Expression* expr = parse_dereference(ident);

		token = lexer->peek_next_token();
		if (token->type == '=') {
			return parse_assigment(expr);
		}

		return expr;
	}

	if (token->type == ':') { //Create variable
		AST_Declaration* declaration = AST_NEW(AST_Declaration);
		declaration->ident = ident;

		lexer->eat_token();
		token = lexer->peek_next_token();
		
		AST_Ident* type = NULL;
		bool is_typedef = this->is_typedef_keyword();
		if (token->type == TOKEN_IDENT || is_typedef) {
			if (is_typedef) {
				declaration->assigmet_type = parse_type();
			}
			else {
				type = create_ident_from_current_token();
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

		if (value->type == AST_PROCEDURE) {
			declaration->assigmet_type = interpret->type_pointer;
		}

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
		call->operation = UNOP_CALL;
		parse_arguments(call->arguments);
		return call;
	}

	return ident;
}

AST_Binary* Parser::parse_assigment(AST_Expression* left) {
	//Token* operation = lexer->peek_next_token();
	lexer->eat_token();

	AST_Binary* assigment = AST_NEW(AST_Binary);
	assigment->left = left;
	assigment->operation = BINOP_ASIGN;
	assigment->right = parse_primary();

	return assigment;
}

AST_Ident* Parser::create_ident_from_current_token() {
	Token* name = this->lexer->peek_next_token();
	this->lexer->eat_token();

	AST_Ident* ident = AST_NEW(AST_Ident);
	ident->name = name;
	return ident;
}

AST_Literal* Parser::parse_string() {
	Token* token = this->lexer->peek_next_token();
	this->lexer->eat_token();

	return type_resolver->make_string_literal(token->value);
}

AST_Literal* Parser::parse_number() {
	auto token = this->lexer->peek_next_token();
	this->lexer->eat_token();

	long long value = 0;
	if (token->number_flags & TOKEN_NUMBER_FLAG_INT) {
		if (token->number_flags & TOKEN_NUMBER_FLAG_HEX) {
			value = token->number_value_l;
		}
		else {
			value = token->number_value_i;
		}
	}
	else if (token->number_flags & TOKEN_NUMBER_FLAG_LONG) {
		value = token->number_value_l;
	}
	else if (token->number_flags & TOKEN_NUMBER_FLAG_FLOAT) {
		//fvalue = floatToInteger(token->number_value_f);
		return type_resolver->make_number_literal(token->number_value_f);
	}
	
	return type_resolver->make_number_literal(value);
	/*AST_Number* num = AST_NEW(AST_Number);
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

	return num;*/
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
	case TOKEN_KEYWORD_S8:
		return "s8";
	case TOKEN_KEYWORD_S16:
		return "s16";
	case TOKEN_KEYWORD_S32:
		return "s32";
	case TOKEN_KEYWORD_S64:
		return "s64";
	case TOKEN_KEYWORD_U8:
		return "u8";
	case TOKEN_KEYWORD_U16:
		return "u16";
	case TOKEN_KEYWORD_U32:
		return "u32";
	case TOKEN_KEYWORD_U64:
		return "u64";
	}

	auto char_type = std::to_string(type);
	return char_type.c_str();
}

bool Parser::is_typedef_keyword() {
	Token* token = lexer->peek_next_token();
	int type = token->type;
	if (   type == TOKEN_KEYWORD_FLOAT	|| type == TOKEN_KEYWORD_LONG
		|| type == TOKEN_KEYWORD_S8		|| type == TOKEN_KEYWORD_S16	|| type == TOKEN_KEYWORD_S32 || type == TOKEN_KEYWORD_S64
		|| type == TOKEN_KEYWORD_U8		|| type == TOKEN_KEYWORD_U16	|| type == TOKEN_KEYWORD_U32 || type == TOKEN_KEYWORD_U64
		|| type == TOKEN_KEYWORD_STRING || type == TOKEN_MUL			|| type == TOKEN_BAND )
		return true;

	return false;
}