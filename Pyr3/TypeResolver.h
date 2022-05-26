#pragma once

#include "Headers.h"
#include "Interpret.h"
#include "Table.h"

struct OperatorKey {
	int Operator;
	AST_Type* Type1;
	AST_Type* Type2;

	bool operator == (const OperatorKey& opr) {
		if (Operator != opr.Operator) { return false; }
		if (Type1->kind != opr.Type1->kind) { return false; }
		else if (Type1->kind == AST_TYPE_STRUCT && Type1->serial != opr.Type1->serial) {
			return false;
			/*
			auto identL = (AST_Ident*)opl.Type1->expression;
			auto identR = (AST_Ident*)opr.Type1->expression;
			if (!COMPARE(identL->name->value, identR->name->value)) {
				return false;
			}*/
		}
		if (Type2 != NULL && Type2->kind != opr.Type2->kind) { return false; }
		else if (Type2 != NULL && Type2->kind == opr.Type2->kind && Type2->kind == AST_TYPE_STRUCT && Type2->serial != opr.Type2->serial) {
			return false;
			/*
			auto identL = (AST_Ident*)opl.Type2->expression;
			auto identR = (AST_Ident*)opr.Type2->expression;
			if (!COMPARE(identL->name->value, identR->name->value)) {
				return false;
			}
			*/
		}

		return true;
	}
};

class TypeResolver
{
private:
	Interpret* interpret;
	Table<OperatorKey, AST_Operator*> operatorTable;
	vector<AST_Expression*> to_be_resolved;
	int phase = 0;

	void addToResolve(AST_Expression* expression);	
	AST_Declaration* find_expression_declaration(AST_Expression* expression);	
	void resolveOther();

public:
	TypeResolver(Interpret* interpret);
	void resolve_main(AST_Block* block);
	void resolve(AST_Block* block);

	void resolveThis(AST_Expression* expression);

	AST_Type* resolveExpression(AST_Expression* expression);
	AST_Type* resolveIdent(AST_Ident* ident);
	AST_Type* resolveDeclaration(AST_Declaration* declaration);
	AST_Type* resolveLiteral(AST_Literal* literal);
	AST_Type* resolveBinary(AST_Binary* binop);
	AST_Type* resolveUnary(AST_UnaryOp* unary);
	AST_Type* resolveType(AST_Type* type, bool as_declaration = false, AST_Expression* value = NULL);
	AST_Type* resolveStructDereference(AST_Struct* _struct, AST_Expression* expression);
	AST_Type* resolveArray(AST_Array * arr);
	AST_Type* resolveOperator(AST_Operator* op);
	void resolveDirective(AST_Directive* directive);

	AST_Operator* findOperator(int Operator, AST_Type* type1, AST_Type* type2 = NULL);

	bool ensure_struct_elements_is_resolved(AST_Struct* _struct);

	AST_Type* find_typeof(AST_Expression* expression, bool deep = true);
	AST_Type* find_typedefinition(AST_Ident* ident, AST_Block* scope);
	AST_Type_Definition* find_typedefinition_from_type(AST_Type* type);

	AST_Declaration* find_declaration(AST_Ident* ident, AST_Block* scope);
	vector<AST_Declaration*> find_declarations(AST_Ident* ident, AST_Block* scope);

	AST_Procedure* find_procedure(AST_Expression* expression, AST_Block* arguments);

	AST_Literal* make_string_literal(String value);
	AST_Literal* make_number_literal(int value);
	AST_Literal* make_number_literal(long long value);
	AST_Literal* make_number_literal(float value);

	void print_procedure(AST_Procedure* procedure);
	bool check_procedure_arguments(AST_Block* header, AST_Block* arguments, int* generic_definition);
	bool compare_type(AST_Expression* left, AST_Expression* right);

	int get_size_of(AST_Expression* expr);
	void calculate_struct_size(AST_Struct* _struct, int offset = 0);
	void calcaulate_index_of_enum(AST_Enum* _enum);
	bool is_static(AST_Expression* expression);
	int do_int_operation(int left, int right, int op);
	int calculate_size_of_static_expression(AST_Expression* expression);
	int calculate_array_size(AST_Type* type, bool first = true);
	String get_string_from_literal(AST_Expression* expression);

	bool is_pointer(AST_Expression* expression);
	bool is_number(AST_Expression* expression);
	bool is_type_integer(AST_Type* type);

	AST_Type* get_inferred_type(AST_Expression* expression);
	void copy_token(AST_Expression* old, AST_Expression* news);

	String expressionTypeToString(AST_Expression* type);
	String typeToString(AST_Type* type);
};
