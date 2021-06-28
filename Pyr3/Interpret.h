#pragma once

#include "Headers.h"
#include "Token.h"
#include "String.h"

enum AST_Types {
	AST_EXPRESSION		= 0x0,
	AST_IDENT			= 0x1,
	AST_BLOCK			= 0x2,
	AST_DECLARATION		= 0x3,
	//AST_STRING = 0x5,
	//AST_NUMBER = 0x6,
	AST_BINARY		= 0x7,
	AST_PROCEDURE		= 0x8,
	AST_PARAMLIST		= 0x9,
	AST_UNARYOP			= 0x10, //16
	AST_DIRECTIVE		= 0x11,
	AST_CONDITION		= 0x12,
	AST_LITERAL			= 0x13, //19
	AST_RETURN			= 0x14,
	AST_TYPE			= 0x15,
	AST_POINTER			= 0x16,
	AST_STRUCT          = 0x17,
};

const int D_COMPILER	= 0x1;
const int D_FOREIGN		= 0x2;
const int D_IMPORT		= 0x3;
const int D_INTERNAL	= 0x4;
const int D_NATIVE		= 0x5;

using namespace std;

struct AST_Declaration;
struct AST_Block;
struct AST_Type_Definition;
struct AST_Ident;
struct AST_Procedure;

struct AST_Expression {
	AST_Expression() {}

	int type = AST_EXPRESSION;
	unsigned int flags = 0;

	AST_Block* scope = NULL;
	Token* token = NULL;
	AST_Expression* expression = NULL;

	//For debug
	int line_number = 0;
	int character_number = 0;
	String *file_name = NULL;
	int bytecode_address = 0;
	int bytecode_index = 0;

#if _DEBUG
	const char* _debug_file;
	int _debug_line;
#endif

};

struct New_String {
	char* data;
	long long size;
};

const int AST_BLOCK_FLAG_MAIN_BLOCK = 0x1;

const int AST_BLOCK_BELONGS_TO_NOTHING = 0;
const int AST_BLOCK_BELONGS_TO_PROCEDURE = 0x1;

struct AST_Block : public AST_Expression {
	AST_Block() { type = AST_BLOCK; }

	vector<AST_Expression*> expressions;

	int belongs_to = 0;
	AST_Procedure* belongs_to_procedure = NULL;
};

enum AST_Internal_Types {
	AST_Type_unitialized	= 0x0,
	AST_Type_s8				= 0x1,
	AST_Type_s16			= 0x2,
	AST_Type_s32			= 0x4,
	AST_Type_s64			= 0x8,
	AST_Type_u8				= 0x10,
	AST_Type_u16			= 0x12,
	AST_Type_u32			= 0x14,
	AST_Type_u64			= 0x18, //24
	AST_Type_char			= 0x20,
	AST_Type_struct			= 0x21,
	AST_Type_enum			= 0x22,
	AST_Type_float			= 0x24,
	AST_Type_long			= 0x28,
	AST_Type_bit			= 0x30,
	AST_Type_pointer		= 0x31,
	AST_Type_string			= 0x32  //50
};

enum AST_Type_Types {
	AST_TYPE_DEFINITION		= 0x0,
	AST_TYPE_POINTER		= 0x1,
	AST_TYPE_ADDRESSOF		= 0x2,
	AST_TYPE_STRUCT			= 0x3,
	AST_TYPE_ARRAY			= 0x4
};

struct AST_Type : public AST_Expression {
	AST_Type() { type = AST_TYPE; }

	int kind = AST_TYPE_DEFINITION;
};

struct AST_Type_Definition : public AST_Type {
	AST_Type_Definition() { kind = AST_TYPE_DEFINITION; }

	int size = 1;
	int aligment = 1;
	int internal_type = 0;
};

struct AST_Struct : public AST_Type {
	AST_Struct() { kind = AST_TYPE_STRUCT; }

	AST_Block* members = NULL;
	int size = 0;
};

enum AST_ARRAY_FLAGS {
	ARRAY_DYNAMIC = 0x1,
};
struct AST_Array : public AST_Type {
	AST_Array() { kind = AST_TYPE_ARRAY; }

	AST_Expression* size = NULL;
	AST_Expression* point_to = NULL;
};

struct AST_Pointer : public AST_Type {
	AST_Pointer() { kind = AST_TYPE_POINTER; }

	AST_Expression* point_to = NULL;
	AST_Type* point_type = NULL;
};

struct AST_Addressof : public AST_Type {
	AST_Addressof() { kind = AST_TYPE_ADDRESSOF; }

	AST_Expression* address_of = NULL;
	AST_Type* address_type = NULL;
};

const int AST_IDENT_FLAG_CONSTANT = 0x1;
struct AST_Ident : public AST_Expression {
	AST_Ident() { type = AST_IDENT; }

	Token* name = NULL;

	AST_Declaration* type_declaration = NULL;
};

const int AST_DECLARATION_FLAG_CONSTANT = 0x1;

const int TYPE_DEFINITION_STRING = 0x1;
const int TYPE_DEFINITION_NUMBER = 0x2;
const int TYPE_DEFINITION_STRUCT = 0x3;
const int TYPE_DEFINITION_IDENT  = 0x4;
const int DECLARATION_IN_HEAD	 = 0x1;

struct AST_Declaration : public AST_Expression {
	AST_Declaration() { type = AST_DECLARATION; }

	AST_Ident* ident = NULL;
	AST_Expression* assigmet_type = NULL;
	AST_Type* inferred_type = NULL; //AST_Type_Definition
	AST_Expression* value = NULL;

	int register_index = -1;
	int offset = 0;
};

const int LITERAL_NUMBER = 0x1;
const int LITERAL_STRING = 0x2;
const int LITERAL_FLOAT  = 0x4;

const int NUMBER_FLAG_SIGNED = 0x1;
const int NUMBER_FLAG_FLOAT  = 0x2;
struct AST_Literal : public AST_Expression {
	AST_Literal() { type = AST_LITERAL; }

	int value_type = 0;

	String string_value = NULL;
	long long integer_value = 0;
	float float_value = 0.0;
	int number_flags = 0;
};

enum UnaryOp {
	UNOP_REF = '*', //42
	UNOP_DEF = '&',  //38,
	UNOP_CALL = 3
};
struct AST_UnaryOp : public AST_Expression {
	AST_UnaryOp() { type = AST_UNARYOP; }

	AST_Expression* left = NULL;
	AST_Block* arguments = NULL;
	int operation = NULL;
	bool isPreppend = true;
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
	BINOP_LESSEQUAL		= 10,  //<=
	BINOP_LOGIC_AND		= 11,  //&&
	BINOP_LOGIC_OR		= 12,  //||
	BINOP_BITWISE_AND	= '&', //38
	BINOP_BITWISE_OR	= '|', //124
	BINOP_DOT			= '.'  //46
};
enum BinaryOpFlags {
	BINOP_FLAG_NOTHARD = 0x01 // x := a?b
};
struct AST_Binary : public AST_Expression {
	AST_Binary() { type = AST_BINARY; }

	AST_Expression* left = NULL;
	AST_Expression* right = NULL;
	int operation = 0;
};

const int AST_DIRECTIVE_FLAG_INITIALIZING = 0x1;
const int AST_DIRECTIVE_FLAG_INITIALIZED = 0x2;
const int AST_DIRECTIVE_FLAG_CANT_INITIALIZE = 0x4;
struct AST_Directive : public AST_Expression {
	AST_Directive() { type = AST_DIRECTIVE; }

	AST_Literal* name = NULL;
	int directive_type = 0;

	AST_Expression* value0 = NULL;
};

struct AST_Return : public AST_Expression {
	AST_Return() { type = AST_RETURN; }

	AST_Expression* value = NULL;
};

const int AST_PROCEDURE_FLAG_COMPILER = 0x1;
const int AST_PROCEDURE_FLAG_INTERNAL = 0x2;
const int AST_PROCEDURE_FLAG_INTRINSIC = 0x4;
const int AST_PROCEDURE_FLAG_FOREIGN = 0x8;
struct AST_Procedure : public AST_Expression {
	AST_Procedure() { type = AST_PROCEDURE; }

	AST_Expression* returnType = NULL;
	AST_Block* header = NULL;
	AST_Block* body = NULL;

	AST_Expression* foreign_library_expression = NULL;
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

	void report_error(String message);

	void report_error(String message, va_list argptr);
	void report_error(String message, const char* file, int row, int column, va_list argptr);
	void report_warning(String message, va_list argptr);
	void report_warning(String message, const char* file, int row, int column, va_list argptr);

	void report_error(Token* token, const char* message, ...);
	void report_error(Token* token, String message, ...);
	void report_warning(Token* token, const char* message, ...);
	void report_warning(Token* token, String message, ...);

	bool isError();		

	AST_Type_Definition* type_bit = NULL;
	AST_Type_Definition* type_pointer = NULL;
	AST_Type_Definition* type_string = NULL;

	AST_Type_Definition* type_int = NULL;
	AST_Type_Definition* type_float = NULL;
	AST_Type_Definition* type_long = NULL;
	AST_Type_Definition* type_char = NULL;

	AST_Type_Definition* type_s8 = NULL;
	AST_Type_Definition* type_s16 = NULL;
	AST_Type_Definition* type_s32 = NULL;
	AST_Type_Definition* type_s64 = NULL;

	AST_Type_Definition* type_u8 = NULL;
	AST_Type_Definition* type_u16 = NULL;
	AST_Type_Definition* type_u32 = NULL;
	AST_Type_Definition* type_u64 = NULL;
};

#define AST_NEW(type) create_expression(new type(), lexer->peek_next_token(), current_scope, __FILE__, __LINE__);
#define AST_NEW_EMPTY(type) create_expression<type>(new type(), nullptr, nullptr, __FILE__, __LINE__);

template<class T>
T insert_and_return(vector<T>& _vector, T object) {
	_vector.push_back(object);
	return object;
}

template<class AST>
AST* create_expression(AST* expression, Token* current_token, AST_Block* current_scope, const char* file, int line) {
	if(current_token != nullptr)
		expression->token = current_token;

	if (current_scope != nullptr)
		expression->scope = current_scope;

#if _DEBUG
	expression->_debug_file = file;
	expression->_debug_line = line;
#endif

	return expression;
}