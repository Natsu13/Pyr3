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
	AST_Type_Definition* find_typedefinition(AST_Ident* ident, AST_Block* block);
	int find_internal_typeinition(Token* value);
	void resolveOther();	
public:
	TypeResolver(Interpret* interpret);
	void resolve_main(AST_Block* block);
	void resolve(AST_Block* block);
	AST_Type_Definition* resolveExpression(AST_Expression* expression);
	AST_Type_Definition* resolveIdent(AST_Ident* ident);
	AST_Type_Definition* resolveDeclaration(AST_Declaration* declaration);
	AST_Type_Definition* resolveLiteral(AST_Literal* literal);
	AST_Type_Definition* resolveBinary(AST_BinaryOp* binop);
	AST_Type_Definition* resolveUnary(AST_UnaryOp* unary);
	AST_Type_Definition* resolveType(AST_Type* type);	
	void resolveDirective(AST_Directive* directive);

	AST_Literal* make_string_literal(String value);
	AST_Literal* make_number_literal(long long value);
	AST_Literal* make_number_literal(float value);
};
