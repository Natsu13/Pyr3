#pragma once

#include "Headers.h"
#include "Parser.h"

class TypeResolver
{
private:
	Interpret* interpret;
	vector<AST_Expression*> to_be_resolved;

	void addToResolve(AST_Expression* expression);
	int find_type_definition(AST_Ident* ident, AST_Block* block);
	int find_internal_type_definition(Token* value);
	void resolveOther();
public:
	TypeResolver(Interpret* interpret);
	void resolve(AST_Block* block);
	int resolveExpression(AST_Expression* expression, AST_Block* block);
	int resolveIdent(AST_Ident* ident);
	int resolveDeclaration(AST_Declaration* declaration);
	int resolveNumber(AST_Number* number);
	int resolveString(AST_String* string);
	int resolveBinop(AST_BinaryOp* binop);
	void resolveDirective(AST_Directive* directive);
};
