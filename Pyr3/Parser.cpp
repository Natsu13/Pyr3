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

	main_scope = current_scope;

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

		if (expression->type == AST_OPERATOR) { //all operators go to main scope!
			main_scope->expressions.push_back(expression);
		}
		else {
			save_scope->expressions.push_back(expression);
		}
	}
	if (new_scope) {
		current_scope = scope;
		return save_scope;
	}
	return scope;
}

AST_Expression* Parser::parse_primary_without_paramlist() {
	auto saved_disable_paramlist = disable_paramlist;
	disable_paramlist = true;
	auto primary = parse_primary();
	disable_paramlist = saved_disable_paramlist;
	return primary;
}

AST_Expression* Parser::parse_primary() {
	return parse_primary(0);
}

AST_Expression* Parser::parse_primary(int prec) {
	AST_Expression* left = NULL;
	auto token = lexer->peek_next_token();

	while (left == NULL || token->type == ',') {
		if (token->type == ',') lexer->eat_token();

		AST_Expression* expr = parse_expression();

		if (expr == NULL)
			return NULL;

		token = lexer->peek_next_token();

		if (token->type == ';' || token->type == '}' || token->type == ')') {
			if ((expr->type == AST_PROCEDURE && token->type == '}')) {
				lexer->eat_token();
			}

			if(left == NULL || disable_paramlist)
				return expr;

			assign_to_ident_or_paramlist(left, expr);
			return left;
		}

		expr = parse_binop(prec, expr);

		if (disable_paramlist)
			return expr;

		assign_to_ident_or_paramlist(left, expr);

		token = lexer->peek_next_token();
	}

	return left;
}

AST_Expression* Parser::parse_expression() {
	Token* token = lexer->peek_next_token();

	if (token->type == TOKEN_MUL || token->type == TOKEN_BAND || token->type == TOKEN_IDENT
		|| token->type == TOKEN_INCREMENT || token->type == TOKEN_DECREMENT) { // *test (pointer to) &test (address of)
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
	else if (token->type == TOKEN_KEYWORD_UNION) {
		return parse_union();
	}
	else if (token->type == TOKEN_KEYWORD_ENUM) {
		return parse_enum();
	}
	else if (token->type == TOKEN_KEYWORD_CAST) {
		return parse_cast();
	}
	else if (token->type == TOKEN_KEYWORD_TYPEOF || token->type == TOKEN_KEYWORD_SIZEOF) {
		return parse_typesizeof();
	}
	else if (token->type == TOKEN_KEYWORD_WHILE) {
		return parse_while();
	}
	else if (token->type == TOKEN_KEYWORD_OPERATOR) {
		return parse_operator();
	}
	else if (token->type == TOKEN_KEYWORD_FOR) {
		return parse_for();
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
	else if (is_typedef_keyword()) {
		return parse_type();
	}

	interpret->report_error(token, "Unexpected token type '%s'", token_to_string(token->type));
	return NULL;
}

bool Parser::expect_and_eat_bracket(bool simple) {
	auto token = lexer->peek_next_token();

	if (simple) { // ()
		if (token->type != '(' && token->type != ')') {
			interpret->report_error(token, "You need to put the expression inside bracket (expr)");
			return false;
		}
	}
	else { // {}
		if (token->type != '{' && token->type != '}') {
			interpret->report_error(token, "You need to put the expressions inside bracket {expr}");
			return false;
		}
	}

	lexer->eat_token();
	return true;
}

AST_TypeSizeOf* Parser::parse_typesizeof() {
	auto token = lexer->peek_next_token();
	lexer->eat_token(); //eat typeof,sizeof

	AST_TypeSizeOf* ast = AST_NEW(AST_TypeSizeOf);
	if (token->type == TOKEN_KEYWORD_TYPEOF) {
		ast->flags |= AST_TYPESIZEOF_IS_TYPEOF;
	}
	else if (token->type == TOKEN_KEYWORD_SIZEOF) {
		ast->flags |= AST_TYPESIZEOF_IS_SIZEOF;
	}

	if (ast->flags == 0) {
		delete ast;
		interpret->report_error(token, "Unexpected token type '%s' for sizeof, typeof", token_to_string(token->type));
		assert(false && "Unkown token");
	}

	expect_and_eat_bracket();

	ast->of = parse_expression();

	expect_and_eat_bracket();

	return ast;
}

AST_Operator* Parser::parse_operator() {
	lexer->eat_token(); //eat operator

	AST_Operator* op = AST_NEW(AST_Operator);
	if (lexer->peek_next_token()->type == TOKEN_LBRACKET) { //handle [] operator
		op->op = lexer->peek_next_token();
		op->op->type = TOKEN_EMPTY_INDEX;

		lexer->eat_token();
		assert(lexer->peek_next_token()->type == TOKEN_RBRACKET);		
	}
	else {
		op->op = lexer->peek_next_token();
	}

	lexer->eat_token();

	assert(lexer->peek_next_token()->type == ':');
	lexer->eat_token();
	assert(lexer->peek_next_token()->type == ':');
	lexer->eat_token();

	assert(lexer->peek_next_token()->type == '(');
	lexer->eat_token();

	op->procedure = parse_param_or_function();

	return op;
}

AST_For* Parser::parse_for() {
	lexer->eat_token(); //eat for

	AST_For* ast_for = AST_NEW(AST_For);

	/*  for value, key: 0..5 {
	 *  for value: 0..5 {
	 *  for 0..5 {
	 */

	int i = 0;
	bool comma = false;
	bool colon = false;
	Token* token = lexer->peek_token(i);
	while (token != NULL && token->type != '{' && token->type != TOKEN_EOF) {
		if (token->type == TOKEN_COMMA) comma = true;
		if (token->type == TOKEN_COLON)	colon = true;

		i++;
		token = lexer->peek_token(i);
	}

	AST_Ident* value = NULL;
	AST_Ident* key = NULL;
	if (comma || colon) {
		value = create_ident_from_current_token();

		token = lexer->peek_next_token();
		if (comma && token->type == ',') {
			lexer->eat_token(); //eat ,
			key = create_ident_from_current_token();
		}
		else if(comma) {
			interpret->report_error(token, "We expected comma here");
		}
	}

	if (value != NULL) {
		auto decl_from = AST_NEW(AST_Declaration);
		decl_from->ident = (AST_Ident*)value;
		decl_from->assigmet_type = interpret->type_s64; //todo: just for now!
		ast_for->value = decl_from;
	}
	if (key != NULL) {
		auto decl_to = AST_NEW(AST_Declaration);
		decl_to->ident = (AST_Ident*)key;
		decl_to->assigmet_type = interpret->type_s64; //todo: just for now!
		ast_for->key = decl_to;
	}

	token = lexer->peek_next_token();
	if (token->type == TOKEN_COLON) { // :
		lexer->eat_token();
	}

	ast_for->each = parse_primary();

	token = lexer->peek_next_token();
	if (token->type == TOKEN_RANGE) { // ..
		auto range = AST_NEW(AST_Range);

		range->from = ast_for->each;
		lexer->eat_token();
		range->to = parse_primary();

		ast_for->each = range;
	}

	auto exp = parse_block_or_expression();

	if (exp->type == AST_BLOCK) {
		ast_for->block = (AST_Block*)exp;
	}
	else {
		ast_for->block = AST_NEW(AST_Block);
		ast_for->block->expressions.push_back(exp);
	}

	if (ast_for->block != NULL) {
		ast_for->block->belongs_to = AST_BLOCK_BELONGS_TO_FOR;
		ast_for->block->belongs_to_for = ast_for;
	}

	ast_for->header = AST_NEW(AST_Block);

	return ast_for;
}

AST_While* Parser::parse_while() {
	lexer->eat_token(); //eat while

	AST_While* ast_while = AST_NEW(AST_While);

	ast_while->condition = parse_expression();
	auto exp = parse_block_or_expression();

	if (exp->type == AST_BLOCK) {
		ast_while->block = (AST_Block*)exp;
	}
	else {
		ast_while->block = AST_NEW(AST_Block);
		ast_while->block->expressions.push_back(exp);
	}

	return ast_while;
}

AST_Cast* Parser::parse_cast() {
	lexer->eat_token(); //eat cast

	AST_Cast* cst = AST_NEW(AST_Cast);

	Token* token = lexer->peek_next_token();
	while (token->type == ',') {
		lexer->eat_token();
		token = lexer->peek_next_token();

		if (token->type == TOKEN_KEYWORD_STATIC) {
			cst->flags |= CAST_STATIC;
		}
		else if (token->type == TOKEN_KEYWORD_NOCHECK) {
			cst->flags |= CAST_NOCHECK;
		}
		else {
			interpret->report_error(token, "Unexpected cast operator '%s'", token_to_string(token->type));
			return NULL;
		}

		lexer->eat_token();
		token = lexer->peek_next_token();
	}

	assert(token->type == '(');
	lexer->eat_token();
	cst->cast_to = parse_typedefinition_or_ident();
	assert(lexer->peek_next_token()->type == ')');
	lexer->eat_token();

	cst->cast_expression = parse_primary();

	return cst;
}

void Parser::parse_member_enum(AST_Enum* _enum) {
	lexer->eat_token(); //eat {
	Token* token = lexer->peek_next_token();

	_enum->members = AST_NEW(AST_Block);
	_enum->index = 0;

	while (token->type != '}' && token->type != TOKEN_EOF) {
		if (token->type == '#') {

		}
		else {
			auto primary = parse_expression();
			AST_Declaration* declaration = NULL;
			bool report_error = false;
			if (primary->type == AST_IDENT) {
				declaration = AST_NEW(AST_Declaration);
				declaration->ident = (AST_Ident*)primary;
				declaration->flags |= AST_DECLARATION_FLAG_CONSTANT;				
			}
			else if (primary->type == AST_DECLARATION) {
				declaration = (AST_Declaration*)primary;

				if (!(declaration->flags & AST_DECLARATION_FLAG_CONSTANT)) {
					report_error = true;
				}
			}
			else {
				report_error = true;
			}

			if (report_error) {
				interpret->report_error(primary->token, "Enum can by only ident or static declaration");
			}
			else {
				_enum->members->expressions.push_back(declaration);
			}
		}

		lexer->eat_token();
		token = lexer->peek_next_token();

		if (token->type == ';') {
			lexer->eat_token();
			token = lexer->peek_next_token();
		}
	}

	assert(token->type == '}');
}

AST_Enum* Parser::parse_enum() {
	lexer->eat_token();
	Token* token = lexer->peek_next_token();

	AST_Expression* type = NULL;
	if (token->type != '{') {
		type = parse_typedefinition_or_ident();
		token = lexer->peek_next_token();
	}

	if (token->type != '{') {
		interpret->report_error(token, "Enum members must be inside block");
		return NULL;
	}

	AST_Enum* _enum = AST_NEW(AST_Enum);
	_enum->serial = interpret->typeCounter++;
	_enum->enum_type = type;

	parse_member_enum(_enum);

	token = lexer->peek_next_token();
	assert(token->type == '}');
	lexer->eat_token();

	return _enum;
}

void Parser::parse_member_union(AST_Union* _union){
	lexer->eat_token(); //eat {
	Token* token = lexer->peek_next_token();

	_union->members = AST_NEW(AST_Block);

	while (token->type != '}' && token->type != TOKEN_EOF) {
		if (token->type == '#') {

		}
		else {
			auto primary = parse_expression();
			_union->members->expressions.push_back(primary);
		}

		lexer->eat_token();
		token = lexer->peek_next_token();

		if (token->type == ';') {
			lexer->eat_token();
			token = lexer->peek_next_token();
		}
	}

	assert(token->type == '}');
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
		
		//lexer->eat_token();
		token = lexer->peek_next_token();

		if (token->type == ';') {
			lexer->eat_token();
			token = lexer->peek_next_token();
		}
	}

	assert(token->type == '}');
}

AST_Union* Parser::parse_union() {
	lexer->eat_token();
	Token* token = lexer->peek_next_token();

	if (token->type != '{') {
		interpret->report_error(token, "Union members must be inside block");
		return NULL;
	}

	AST_Union* _union = AST_NEW(AST_Union);

	parse_member_union(_union);

	token = lexer->peek_next_token();
	assert(token->type == '}');
	lexer->eat_token();

	return _union;
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
		directive->directive_type = AST_DIRECTIVE_TYPE_IMPORT;

		token = lexer->peek_next_token();
		if (token->type != TOKEN_STRING) {
			interpret->report_error(token, "import procedure need name of library or pyr file as string"); //this will be handled with type resolver soo we can pass constant too
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
		AST_Expression* exp = parse_primary_without_paramlist();
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
			function->returnType = parse_type_or_paramlist();// parse_typedefinition_or_ident();
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
		else if (COMPARE(name, "c_call")) {
			function->flags |= AST_PROCEDURE_FLAG_C_CALL;
		}
		else if (COMPARE(name, "foreign")) {
			function->flags |= AST_PROCEDURE_FLAG_FOREIGN;

			lexer->eat_token();
			token = lexer->peek_next_token();
			function->foreign_library_expression = parse_expression();		
		}
		else {
			interpret->report_error(token, "unkown directive '%s' in procedure", name);
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
		if(function->returnType != NULL) 
			function->returnType->scope = block; //put the return type to scope of the procedure declaration soo it can see the Generic types
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

AST_Type* Parser::parse_type() {
	auto type = parse_typedefinition();

	return type;
}

AST_Expression* Parser::parse_type_or_paramlist() {
	Token* token = lexer->peek_next_token();
	auto paramlist = AST_NEW(AST_ParamList);

	if(token->type == '(') expect_and_eat_bracket();

	AST_Expression* type = NULL;
	while (token->type == ',' || paramlist->expressions.size() == 0) {
		type = parse_typedefinition_or_ident();
		token = lexer->peek_next_token();
		if (token->type == ',') {
			paramlist->expressions.push_back(type);
			lexer->eat_token();
		}
		else if(paramlist->expressions.size() == 0){
			delete paramlist;
			return type;
		}
	}

	paramlist->expressions.push_back(type);
	if (token->type == ')') expect_and_eat_bracket();
	
	return paramlist;
}

AST_Expression* Parser::parse_typedefinition_or_ident() {
	auto type = parse_typedefinition();
	if(type == NULL)
		return create_ident_from_current_token();
	return type;
}

AST_Type* Parser::parse_typedefinition() {
	Token* token = lexer->peek_next_token();

	AST_Type* _type = NULL;
	AST_Pointer* _pointer = NULL;

	//token = lexer->peek_next_token();

	while (token->type == TOKEN_MUL) {		
		AST_Pointer* pointer = AST_NEW(AST_Pointer);
		if (_pointer != NULL && _pointer->kind == AST_POINTER) {
			pointer->point_to = _pointer;
		}		
		_pointer = pointer;
		
		this->lexer->eat_token();
		token = lexer->peek_next_token();		
	}

	switch (token->type) {
		case TOKEN_KEYWORD_BOOL: {
			_type = interpret->type_bool;
			break;
		}
		case TOKEN_KEYWORD_CHAR: {
			_type = interpret->type_char;
			break;
		}
		case TOKEN_KEYWORD_FLOAT: {
			_type = interpret->type_float;
			break;
		}
		case TOKEN_KEYWORD_LONG: {
			_type = interpret->type_long;
			break;
		}
		case TOKEN_KEYWORD_STRING: {
			_type = interpret->type_string;
			break;
		}
		case TOKEN_KEYWORD_S8: {
			_type = interpret->type_s8;
			break;
		}
		case TOKEN_KEYWORD_S16: {
			_type = interpret->type_s16;
			break;
		}
		case TOKEN_KEYWORD_S32: {
			_type = interpret->type_s32;
			break;
		}
		case TOKEN_KEYWORD_S64: {
			_type = interpret->type_s64;
			break;
		}
		case TOKEN_KEYWORD_U8: {
			_type = interpret->type_u8;
			break;
		}
		case TOKEN_KEYWORD_U16: {
			_type = interpret->type_u16;
			break;
		}
		case TOKEN_KEYWORD_U32: {
			_type = interpret->type_u32;
			break;
		}
		case TOKEN_KEYWORD_U64: {
			_type = interpret->type_u64;
			break;
		}
		case TOKEN_KEYWORD_POINTER: {
			_type = interpret->type_pointer;
			break;
		}
		case TOKEN_KEYWORD_INT: {
			_type = interpret->type_s64; // int
		}
	}

	if (_pointer != NULL) {
		this->lexer->eat_token();
		_pointer->point_to = _type;
		return _pointer;
	}

	if (_type != NULL) {
		this->lexer->eat_token();
	}

	return _type;
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

AST_Expression* Parser::parse_type_array(AST_Expression* type) {
	auto token = lexer->peek_next_token();
	while (token->type == TOKEN_LBRACKET) {
		lexer->eat_token();
		token = lexer->peek_next_token();

		AST_Array* arr = AST_NEW(AST_Array);
		arr->token = lexer->peek_next_token();

		if (token->type == TOKEN_RBRACKET) {
			lexer->eat_token();
			arr->flags = ARRAY_AUTO_SIZE;
			arr->point_to = type;
			type = arr;
			continue;
		}

		token = lexer->peek_next_token();
		if (token->type == TOKEN_RANGE) {
			arr->flags |= ARRAY_DYNAMIC;
		}
		else {
			arr->size = parse_primary();
		}
		arr->point_to = type;
		type = arr;

		token = lexer->peek_next_token();
		assert(token->type == TOKEN_RBRACKET);
		lexer->eat_token();
		token = lexer->peek_next_token();
	}

	return type;
}

AST_Expression* Parser::parse_ident_array(AST_Expression* ident) {
	auto token = lexer->peek_next_token();
	while (token->type == TOKEN_LBRACKET) {
		lexer->eat_token();

		AST_Array* arr = AST_NEW(AST_Array);
		arr->flags |= ARRAY_IDENT;
		arr->token = lexer->peek_next_token();
		token = lexer->peek_next_token();

		arr->size = parse_primary();
		arr->point_to = ident;
		ident = arr;

		token = lexer->peek_next_token();
		assert(token->type == TOKEN_RBRACKET);
		lexer->eat_token();
		token = lexer->peek_next_token();
	}

	return ident;
}

AST_Expression* Parser::parse_dereference(AST_Expression* ident) {
	if (ident == NULL) {
		ident = create_ident_from_current_token();
	}

	assert(ident->type == AST_IDENT);

	auto token = lexer->peek_next_token();
	if (token->type == '.' || token->type == '?') {
		AST_Binary* binary = AST_NEW(AST_Binary);
		binary->operation = BINOP_DOT;
		binary->left = ident;

		lexer->eat_token();

		if (token->type == '?') {
			binary->flags |= BINOP_FLAG_NOTHARD;
			lexer->eat_token();
		}

		token = lexer->peek_next_token();
		if (binary->flags & BINOP_FLAG_NOTHARD && token->type != '.') {
			interpret->report_error(token, "Dereference operator ? must continue with dot operator");
		}

		binary->right = parse_dereference();

		return binary;
	}

	if (token->type == '[') { // a.b[10]
		return parse_ident_array(ident);
	}
	
	return ident;	
}

void Parser::assign_to_ident_or_paramlist(AST_Expression* &ident_or_paramlist, AST_Expression* expression) {
	if (ident_or_paramlist == NULL) {
		ident_or_paramlist = expression;
	}
	else if (ident_or_paramlist->type == AST_IDENT) {
		auto param_list = AST_NEW(AST_ParamList);
		param_list->expressions.push_back(ident_or_paramlist);
		param_list->expressions.push_back(expression);
		ident_or_paramlist = param_list;
	}
	else {
		assert(ident_or_paramlist->type == AST_PARAMLIST);
		auto param_list = (AST_ParamList*)ident_or_paramlist;
		param_list->expressions.push_back(expression);
	}
}

AST_Expression* Parser::parse_ident() {
	auto token = lexer->peek_next_token();

	if (token->type == TOKEN_MUL) { //Pointer to *test
		lexer->eat_token();

		AST_Unary* unary = AST_NEW(AST_Unary);
		unary->operation = UNOP_DEF;
		unary->left = parse_expression();
		return unary;
	}
	if (token->type == TOKEN_BAND) { //Address of &test
		lexer->eat_token();

		AST_Unary* unary = AST_NEW(AST_Unary);
		unary->operation = UNOP_REF;
		unary->left = parse_expression();
		return unary;
	}
	if (token->type == TOKEN_INCREMENT) { //++i
		lexer->eat_token();
		AST_Unary* unary = AST_NEW(AST_Unary);
		unary->operation = UNOP_INCREMENT;
		unary->left = parse_expression();
		return unary;
	}
	if (token->type == TOKEN_DECREMENT) { //--i
		lexer->eat_token();
		AST_Unary* unary = AST_NEW(AST_Unary);
		unary->operation = UNOP_DECREMENT;
		unary->left = parse_expression();
		return unary;
	}

	AST_Expression* ident = NULL;

	while (ident == NULL || token->type == ',') {
		if (token->type == ',') {
			if (lexer->peek_token()->type == TOKEN_IDENT && !disable_paramlist) {
				lexer->eat_token();
			}
			else {
				return ident;
			}
		}

		AST_Expression* _ident = create_ident_from_current_token();
		token = lexer->peek_next_token();
		if (token->type == '[') { //Array
			_ident = parse_ident_array(_ident);
			token = lexer->peek_next_token();
		}

		if (token->type == '.') { // a.b.c
			_ident = parse_dereference(_ident);
			token = lexer->peek_next_token();
		}

		assign_to_ident_or_paramlist(ident, _ident);
		token = lexer->peek_next_token();
	}

	if (ident->type == AST_IDENT) { //not for paramlist
		if (token->type == TOKEN_INCREMENT) { //i++
			lexer->eat_token();
			AST_Unary* unary = AST_NEW(AST_Unary);
			unary->operation = UNOP_INCREMENT;
			unary->left = ident;
			unary->isPreppend = false;
			return unary;
		}
		if (token->type == TOKEN_DECREMENT) { //i--
			lexer->eat_token();
			AST_Unary* unary = AST_NEW(AST_Unary);
			unary->operation = UNOP_DECREMENT;
			unary->left = ident;
			unary->isPreppend = false;
			return unary;
		}
	}

	if (token->type == ':') { //Create variable
		AST_Declaration* declaration = AST_NEW(AST_Declaration);
		if (ident->type == AST_IDENT) {
			declaration->ident = (AST_Ident*)ident;
		}
		else {
			declaration->param_list = (AST_ParamList*)ident;
			declaration->param_list->flags |= AST_PARAMLIST_IS_DECLARATION;

			declaration->flags |= DECLARATION_IS_FROM_PARAMLIST;			
		}

		lexer->eat_token();
		token = lexer->peek_next_token();

		if (declaration->param_list != NULL && (token->type != '=' && token->type != ':')) {
			interpret->report_error(token, "Param list declaration can't have specifed types by declaration please use := auto decide.");
		}

		if (token->type != ':') {
			if (token->type == '$') { //parse generic types 'value: $T'
				lexer->eat_token();
				declaration->flags |= TYPE_DEFINITION_GENERIC;
				token = lexer->peek_next_token();
			}

			bool is_typedef = is_typedef_keyword();
			if (is_typedef) {
				declaration->assigmet_type = parse_type();
				token = lexer->peek_next_token();
				if (token->type == TOKEN_LBRACKET) {
					declaration->assigmet_type = parse_type_array(declaration->assigmet_type);
				}
			}
			else if ((declaration->flags & TYPE_DEFINITION_GENERIC) == TYPE_DEFINITION_GENERIC) {
				declaration->assigmet_type = create_ident_from_current_token();
				token = lexer->peek_next_token();
				if (token->type == TOKEN_LBRACKET) {
					declaration->assigmet_type = parse_type_array(declaration->assigmet_type);
				}
			}
			else {
				declaration->assigmet_type = parse_primary();
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

		token = lexer->peek_next_token();
		if (token->type == '{') {
			declaration->value = parse_list();
			assert(lexer->peek_next_token()->type == ';');
			lexer->eat_token();
		}
		else {
			declaration->value = parse_primary();
		}

		if (declaration->value->type == AST_PROCEDURE) {
			auto procedure = (AST_Procedure*)declaration->value;
			declaration->assigmet_type = interpret->type_pointer;
			procedure->name = declaration->ident->name;
		}

		return declaration;
	}
	else if (token->type == '=' || token->type == TOKEN_INCREMENT_ASIGN || token->type == TOKEN_DECREMENT_ASIGN 
		|| token->type == TOKEN_MUL_ASING || token->type == TOKEN_DIV_ASING) { //Assign to variable		
		return parse_assigment(ident);
	}
	else if (token->type == '(') { //Call function
		lexer->eat_token();

		AST_Unary* call = AST_NEW(AST_Unary);
		call->left = ident;
		call->arguments = AST_NEW(AST_Block);
		call->operation = UNOP_CALL;
		parse_arguments(call->arguments);
		return call;
	}	

	return ident;
}

AST_Block* Parser::parse_list() {
	lexer->eat_token(); // eat {

	AST_Block* block = AST_NEW(AST_Block);

	while (lexer->peek_next_token()->type != '}') {
		block->expressions.push_back(parse_primary());

		if (lexer->peek_next_token()->type == ',') {
			lexer->eat_token();
		}
	}

	lexer->eat_token(); // eat }

	return block;
}

AST_Binary* Parser::parse_assigment(AST_Expression* left) {
	Token* operation = lexer->peek_next_token();
	lexer->eat_token();

	int op = BINOP_ASIGN;

	if (operation->type == TOKEN_INCREMENT_ASIGN)
		op = BINOP_PLUS_ASIGN;
	else if (operation->type == TOKEN_DECREMENT_ASIGN)
		op = BINOP_MINUS_ASIGN;
	else if (operation->type == TOKEN_MUL_ASING)
		op = BINOP_TIMES_ASIGN; 
	else if (operation->type == TOKEN_DIV_ASING)
		op = BINOP_DIV_ASIGN;

	AST_Binary* assigment = AST_NEW(AST_Binary);
	assigment->left = left;
	assigment->operation = op;
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
		return type_resolver->make_number_literal(token->number_value_f);
	}
	
	return type_resolver->make_number_literal(value);
}

bool Parser::is_typedef_keyword() {
	Token* token = lexer->peek_next_token();
	int type = token->type;
	if (type == TOKEN_KEYWORD_FLOAT || type == TOKEN_KEYWORD_LONG || type == TOKEN_KEYWORD_CHAR
		|| type == TOKEN_KEYWORD_S8 || type == TOKEN_KEYWORD_S16 || type == TOKEN_KEYWORD_S32 || type == TOKEN_KEYWORD_S64
		|| type == TOKEN_KEYWORD_U8 || type == TOKEN_KEYWORD_U16 || type == TOKEN_KEYWORD_U32 || type == TOKEN_KEYWORD_U64
		|| type == TOKEN_KEYWORD_STRING || type == TOKEN_MUL || type == TOKEN_BAND || type == TOKEN_KEYWORD_POINTER || type == '*' 
		|| type == TOKEN_KEYWORD_BOOL || type == TOKEN_KEYWORD_INT)
		return true;

	return false;
}