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
	int find_internal_typeinition(Token* value);
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
	AST_Type* resolveBinary(AST_BinaryOp* binop);
	AST_Type* resolveUnary(AST_UnaryOp* unary);
	AST_Type* resolveType(AST_Type* type);
	void resolveDirective(AST_Directive* directive);

	AST_Type* find_typedefinition(AST_Ident* ident, AST_Block* scope);
	AST_Type_Definition* find_typedefinition_from_type(AST_Type* type);
	AST_Declaration* find_declaration(AST_Ident* ident, AST_Block* scope);

	AST_Literal* make_string_literal(String value);
	AST_Literal* make_number_literal(long long value);
	AST_Literal* make_number_literal(float value);

	bool is_pointer(AST_Expression* expression);
	bool is_number(AST_Expression* expression);

	AST_Type* get_inferred_type(AST_Expression* expression);
};
