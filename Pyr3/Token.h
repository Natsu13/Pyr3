#pragma once

#include "Headers.h"
#include "String.h"

const int TOKEN_KEYWORD_IDENT = 1000;
const int TOKEN_KEYWORD_TRUE = 10001;
const int TOKEN_KEYWORD_FALSE = 10002;
const int TOKEN_KEYWORD_IF = 10003;
const int TOKEN_KEYWORD_ELSE = 10004;
const int TOKEN_KEYWORD_FOR = 10005;
const int TOKEN_KEYWORD_STRING = 10007;
const int TOKEN_KEYWORD_NEW = 10008;
const int TOKEN_KEYWORD_FLOAT = 10009;
const int TOKEN_KEYWORD_CHAR = 10030;
const int TOKEN_KEYWORD_LONG = 10010;
const int TOKEN_KEYWORD_RETURN = 10011;
const int TOKEN_KEYWORD_ENUM = 10012;
const int TOKEN_KEYWORD_STRUCT = 10013;
const int TOKEN_KEYWORD_DEFER = 10014;
const int TOKEN_KEYWORD_CONSTRUCTOR = 10015;
const int TOKEN_KEYWORD_DESCRUCTOR = 10016;
const int TOKEN_KEYWORD_S16 = 10018;
const int TOKEN_KEYWORD_S32 = 10019;
const int TOKEN_KEYWORD_S64 = 10020;
const int TOKEN_KEYWORD_U16 = 10021;
const int TOKEN_KEYWORD_U32 = 10022;
const int TOKEN_KEYWORD_U64 = 10023;
const int TOKEN_KEYWORD_S8 = 10024;
const int TOKEN_KEYWORD_U8 = 10025;
const int TOKEN_KEYWORD_POINTER = 10026;
const int TOKEN_KEYWORD_CAST = 10027;
const int TOKEN_KEYWORD_STATIC = 10028;
const int TOKEN_KEYWORD_NOCHECK = 10029;
const int TOKEN_KEYWORD_WHILE = 10031;

const int TOKEN_DIRECTIVE		= '#';
const int TOKEN_PLUS			= '+';
const int TOKEN_MINUS			= '-';
const int TOKEN_MUL				= '*';
const int TOKEN_DIV				= '/';
const int TOKEN_LPARENTHESE		= '(';
const int TOKEN_RPARENTHESE		= ')';
const int TOKEN_LBRACKET		= '[';
const int TOKEN_RBRACKET		= ']';
const int TOKEN_LBLOCK			= '{';
const int TOKEN_RBLOCK			= '}';
const int TOKEN_MORE			= '>';
const int TOKEN_LESS			= '<';
const int TOKEN_COLON			= ':';
const int TOKEN_DOT				= '.';
const int TOKEN_COMMA			= ',';
const int TOKEN_SEMICOLON		= ';';
const int TOKEN_ASING			= '=';
const int TOKEN_EXCLAMATION		= '!';
const int TOKEN_QUESTIONMARK	= '?';
const int TOKEN_BAND			= '&';
const int TOKEN_BOR				= '|';
const int TOKEN_DOLAR			= '$';
const int TOKEN_MOD				= '%';

const int TOKEN_EOF = -1;
const int TOKEN_NOP = -2;

const int TOKEN_DIV_ASING = 703;		///=
const int TOKEN_MUL_ASING = 702;		//*=
const int TOKEN_INCREMENT = 100;		//++
const int TOKEN_INCREMENT_ASIGN = 700;	//+=
const int TOKEN_DECREMENT = 101;		//--
const int TOKEN_DECREMENT_ASIGN = 701;	//-=
const int TOKEN_RETURNTYPE = 102;		//->
const int TOKEN_EQUAL = 104;			//==
const int TOKEN_NOTEQUAL = 105;			//!=
const int TOKEN_AND = 106;				//&&
const int TOKEN_OR = 107;				//||
const int TOKEN_RANGE = 108;			//..
const int TOKEN_MOREEQUAL = 109;		//>=
const int TOKEN_LESSEQUAL = 110;		//<=

const int TOKEN_NUMBER	= 200;
const int TOKEN_KEYWORD = 300;
const int TOKEN_IDENT	= 400;
const int TOKEN_STRING	= 500;

const int TOKEN_NUMBER_FLAG_INT		= 0x1;
const int TOKEN_NUMBER_FLAG_LONG	= 0x2;
const int TOKEN_NUMBER_FLAG_HEX		= 0x4;
const int TOKEN_NUMBER_FLAG_FLOAT	= 0x8;

class Token
{
private:
public:
	Token();
	Token(String value);
	Token(String file_name, int row, int column);

	String file_name = "";
	String value = "";
	int row = -1;
	long long column = -1;

	int index = -1;

	int type = 0;
	int token_keyword = 0;
	int token_number = 0;

	int number_value_i = 0;
	double number_value_d = 0;
	float number_value_f = 0;
	long number_value_l = 0;

	int number_flags = 0x0;
};

typedef struct token_define {
	int token_type = 0;
	int type = 0;

	String value_string;
} TokenDefine;