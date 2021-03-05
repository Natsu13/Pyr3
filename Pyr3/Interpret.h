#pragma once

#include "Headers.h"
#include "Token.h"

enum AST_Types {
	AST_EXPRESSION = 0x0,
	AST_IDENT = 0x1,
	AST_BLOCK = 0x2,
	AST_DECLARATION = 0x3,
	AST_TYPE_DEFINITION = 0x4,
	AST_STRING = 0x5,
	AST_NUMBER = 0x6,
	AST_BINARYOP = 0x7,
	AST_FUNCTION = 0x8,
	AST_PARAMLIST = 0x9,
	AST_UNARYOP = 0x10,
	AST_DIRECTIVE = 0x11,
	AST_CONDITION = 0x12,
	AST_LITERAL = 0x13
};

const int D_COMPILER = 0x1;
const int D_FOREIGN = 0x2;
const int D_IMPORT = 0x3;
const int D_INTERNAL = 0x4;
const int D_NATIVE = 0x5;

using namespace std;

struct AST_Declaration;
struct AST_Block;
struct AST_Type_Definition;
struct AST_Ident;

struct AST_Expression {
	AST_Expression() {}

	int type = AST_EXPRESSION;
};

const int AST_BLOCK_FLAG_MAIN_BLOCK = 0x1;
struct AST_Block : public AST_Expression {
	AST_Block() { type = AST_BLOCK; }

	int flags = 0x0;
	AST_Block* scope = NULL;
	vector<AST_Expression*> expressions;
};

const int AST_Type_unitialized = 0x0;
const int AST_Type_s8 = 0x1;
const int AST_Type_s16 = 0x2;
const int AST_Type_s32 = 0x3;
const int AST_Type_s64 = 0x4;
const int AST_Type_int = 0x5;
const int AST_Type_char = 0x6;
const int AST_Type_struct = 0x7;
const int AST_Type_enum = 0x8;
const int AST_Type_float = 0x9;
const int AST_Type_long = 0x10;
const int AST_Type_bit = 0x11;

struct AST_Type_Definition : public AST_Expression {
	AST_Type_Definition() { type = AST_TYPE_DEFINITION; }

	int size = 1; //Size in bits
	int internal_type = 0;
};

const int AST_IDENT_FLAG_CONSTANT = 0x1;
struct AST_Ident : public AST_Expression {
	AST_Ident() { type = AST_IDENT; }

	Token* name = NULL;

	AST_Block* scope = NULL;
	AST_Expression* type_definition = NULL;
	AST_Declaration* type_declaration = NULL;

	int flags = 0x0;
};

const int AST_DECLARATION_FLAG_CONSTANT = 0x1;

const int TYPE_DEFINITION_STRING = 0x1;
const int TYPE_DEFINITION_NUMBER = 0x2;
const int TYPE_DEFINITION_STRUCT = 0x3;
const int TYPE_DEFINITION_IDENT  = 0x4;

struct AST_Declaration : public AST_Expression {
	AST_Declaration() { type = AST_DECLARATION; }

	AST_Ident* ident = NULL;
	AST_Expression* assigmet_type = NULL;
	AST_Expression* value = NULL;

	AST_Block* scope = NULL;
	int type_definition = NULL;

	int register_index = -1;

	int flags = 0x0;
};

struct AST_Literal : public AST_Expression {
	AST_Literal() { type = AST_LITERAL; }
};

struct AST_String : public AST_Expression {
	AST_String() { type = AST_STRING; }

	wstring value;
	Token* name = NULL;
};
struct AST_Number : public AST_Expression {
	AST_Number() { type = AST_NUMBER; }

	int flags = 0x0;

	int64_t value = 0;
};

struct AST_UnaryOp : public AST_Expression {
	AST_UnaryOp() { type = AST_UNARYOP; }

	AST_Expression* left = NULL;
	AST_Block* arguments = NULL;
	Token* operation = NULL;
	bool isPreppend = false;
};

enum BinaryOp {
	BINOP_ASIGN			= '=', //61
	BINOP_PLUS			= '+', //43
	BINOP_MINUS			= '-', //45
	BINOP_TIMES			= '*', //42
	BINOP_DIV			= '/', //47
	BINOP_MOD			= '%', //37
	BINOP_ISEQUAL		= 5,   //==
	BINOP_ISNOTEQUAL	= 6,   //!=
	BINOP_GREATER		= '>', //62
	BINOP_GREATEREQUAL	= 8,   //>=
	BINOP_LESS			= '<', //60
	BINOP_LESSEQUAL		= 10,  // <=
	BINOP_LOGIC_AND		= 11,  // &&
	BINOP_LOGIC_OR		= 12,  // ||
	BINOP_BITWISE_AND	= '&', //38
	BINOP_BITWISE_OR	= '|', //124
};
struct AST_BinaryOp : public AST_Expression {
	AST_BinaryOp() { type = AST_BINARYOP; }

	AST_Expression* left = NULL;
	AST_Expression* right = NULL;
	int operation = 0;

	AST_Block* scope = NULL;
};

const int AST_DIRECTIVE_FLAG_INITIALIZING = 0x1;
const int AST_DIRECTIVE_FLAG_INITIALIZED = 0x2;
const int AST_DIRECTIVE_FLAG_CANT_INITIALIZE = 0x4;
struct AST_Directive : public AST_Expression {
	AST_Directive() { type = AST_DIRECTIVE; }

	AST_String* name = NULL;
	int directive_type = 0;
	int flags = 0;

	AST_Expression* value0 = NULL;

	AST_Block* scope = NULL;

};


const int AST_FUNCTION_FLAG_COMPILER = 0x1;
const int AST_FUNCTION_FLAG_INTERNAL = 0x2;
const int AST_FUNCTION_FLAG_NATIVE = 0x4;
const int AST_FUNCTION_FLAG_FOREIGN = 0x8;
struct AST_Function : public AST_Expression {
	AST_Function() { type = AST_FUNCTION; }

	AST_Expression* returnType = NULL;
	AST_Block* header = NULL;
	AST_Block* body = NULL;

	AST_String* foreign_library = NULL;
	AST_Expression* foreign_library_expression = NULL;

	int flags = 0;
};

struct AST_Condition : public AST_Expression {
	AST_Condition() { type = AST_CONDITION; }

	AST_Expression* condition = NULL;
	AST_Expression* body_pass = NULL;
	AST_Expression* body_fail = NULL;
};

class Interpret {
private:
	bool errorOccured = false;

	void initialize();
public:
	Interpret();

	void report_error(wstring message, va_list argptr);
	void report_error(wstring message, const char* file, int row, int column, va_list argptr);
	void report_warning(wstring message, va_list argptr);
	void report_warning(wstring message, const char* file, int row, int column, va_list argptr);

	void report_error(Token* token, const char* message, ...);
	void report_error(Token* token, wstring message, ...);
	void report_warning(Token* token, const char* message, ...);
	void report_warning(Token* token, wstring message, ...);

	bool isError();		

	AST_Type_Definition* type_def_bit = NULL;

	AST_Type_Definition* type_def_int = NULL;
	AST_Type_Definition* type_def_float = NULL;
	AST_Type_Definition* type_def_long = NULL;
	AST_Type_Definition* type_def_char = NULL;

	AST_Type_Definition* type_def_s8 = NULL;
	AST_Type_Definition* type_def_s16 = NULL;
	AST_Type_Definition* type_def_s32 = NULL;
	AST_Type_Definition* type_def_s64 = NULL;
};

#define AST_NEW(type) new type();

template<class T>
T insert_and_return(vector<T>& _vector, T object) {
	_vector.push_back(object);
	return object;
}