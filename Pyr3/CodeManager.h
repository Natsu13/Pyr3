#pragma once
#include "Parser.h"
#include "Interpret.h"
#include "TypeResolver.h"

class CodeManager
{
private:
	Interpret* interpret;
	TypeResolver* typeResolver;

	void manageExpression(AST_Expression* expression);
	void manageUnary(AST_Unary* unary);
	void manageDeclaration(AST_Declaration* declaration);
	void manageProcedure(AST_Procedure* procedure);
	void manageBinary(AST_Binary* binary);
	void manageCondition(AST_Condition* condition);

public:
	CodeManager(Interpret* interpret, TypeResolver* typeResolver);
	void manage(AST_Block* block);
};