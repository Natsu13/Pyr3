#pragma once

#include "Headers.h"
#include "Interpret.h"

class TypeResolver
{
private:
	Interpret* interpret;
	vector<AST_Expression*> to_be_resolved;
	int phase = 0;

	void addToResolve(AST_Expression* expression);	
	AST_Declaration* find_expression_declaration(AST_Expression* expression);	
	void resolveOther();

public:
	TypeResolver(Interpret* interpret);
	void resolve_main(AST_Block* block);
	void resolve(AST_Block* block);
	AST_Type* resolveExpression(AST_Expression* expression);
	AST_Type* resolveIdent(AST_Ident* ident);
	AST_Type* resolveDeclaration(AST_Declaration* declaration);
	AST_Type* resolveLiteral(AST_Literal* literal);
	AST_Type* resolveBinary(AST_Binary* binop);
	AST_Type* resolveUnary(AST_UnaryOp* unary);
	AST_Type* resolveType(AST_Type* type, bool as_declaration = false, AST_Expression* value = NULL);
	AST_Type* resolveStructDereference(AST_Struct* _struct, AST_Expression* expression);
	AST_Type* resolveArray(AST_Array * arr);
	void resolveDirective(AST_Directive* directive);

	AST_Type* find_typeof(AST_Expression* expression);
	AST_Type* find_typedefinition(AST_Ident* ident, AST_Block* scope);
	AST_Type_Definition* find_typedefinition_from_type(AST_Type* type);
	AST_Declaration* find_declaration(AST_Ident* ident, AST_Block* scope);

	AST_Literal* make_string_literal(String value);
	AST_Literal* make_number_literal(int value);
	AST_Literal* make_number_literal(long long value);
	AST_Literal* make_number_literal(float value);

	int get_size_of(AST_Expression* expr);
	void calculate_struct_size(AST_Struct* _struct, int offset = 0);
	bool is_static(AST_Expression* expression);
	int do_int_operation(int left, int right, int op);
	int calculate_size_of_static_expression(AST_Expression* expression);
	int calculate_array_size(AST_Type* type);
	String get_string_from_literal(AST_Expression* expression);

	bool is_pointer(AST_Expression* expression);
	bool is_number(AST_Expression* expression);
	bool is_type_integer(AST_Type* type);

	AST_Type* get_inferred_type(AST_Expression* expression);
	void copy_token(AST_Expression* old, AST_Expression* news);

	String expressionTypeToString(AST_Expression* type);
	String typeToString(AST_Type* type);
};
