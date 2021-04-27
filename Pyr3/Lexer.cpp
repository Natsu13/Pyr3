#pragma once

#include "Headers.h"
#include "Utils.h"
#include "Lexer.h"

Lexer::Lexer(Interpret* interpreter, const char* file_name, String code) {
	this->interpreter = interpreter;
	this->code = code;
	this->pos = 0;
	this->token_pos = 0;
	this->tokens.clear();
	this->current_file_name = file_name;
	this->current_row = 1;
	this->current_col = 1;

	if (code[0] == '\0') {
		load_file(file_name);
	}
}

vector<Token*> Lexer::getTokens()
{
	return this->tokens;
}

Token* Lexer::peek_next_token() {
	return this->tokens[this->token_pos];
}

void Lexer::eat_token() {
	if(this->token_pos + 1 < static_cast<int>(this->tokens.size()))
		this->token_pos++;
}

bool Lexer::load_file(const char* file_name)
{
	this->code = Utils::load_file(file_name);
	return true;
}

void Lexer::report_error(String message, ...) {
	string file = this->current_file_name;
	std::string file_name(file.substr(file.rfind("\\") + 1));
	va_list args;
	va_start(args, message);
	this->interpreter->report_error(message, file_name.c_str(), this->current_row, this->current_col, args);
	va_end(args);
}

void Lexer::report_warning(String message, ...) {
	string file = this->current_file_name;
	std::string file_name(file.substr(file.rfind("\\") + 1));
	va_list args;
	va_start(args, message);
	this->interpreter->report_warning(message, file_name.c_str(), this->current_row, this->current_col, args);
	va_end(args);
}

void Lexer::eat_white() {
	int ch = peek_next_character();
	while ((ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') && ch != '0') {
		eat_character();
		ch = peek_next_character();
	}
}

void Lexer::eat_comment() {
	int ch = peek_next_character();
	while (ch != '\n' && ch != '\0') {
		eat_character();
		ch = peek_next_character();
	}
}

char Lexer::peek_next_character() {	
	if (!peeked) {
		if (this->code[this->pos] == '\n') { this->current_row++; this->current_col = 0; }
		peeked = true;
	}

	if (this->pos < this->code.size) {		
		return this->code[this->pos];
	}

	return 0;
}

void Lexer::eat_character() {		
	peeked = false;
	this->pos++;	
	this->current_col++;
}

//We parse it as string because it's string anyway then we parse it to the actual number
//this will maybe be changed in future
String Lexer::peek_next_numer() {
	String str = "";

	bool has_x = false;
	//bool has_l = false;
	bool has_dot = false;

	char ch = peek_next_character();
	while ((ch >= '0' && ch <= '9') || ch == 'x' || ch == '.' || ch == 'l' || ch == '-') {
		eat_character();

		str += ch;
		ch = peek_next_character();

		if (ch == '-') {
			if (str.size != 0) {
				return str;
			}
		}
		else if (ch == 'x') { 			
			if (has_x || !(str.size == 1 && str[0] == '0') || has_dot) {
				report_error("Unkown number modifier 'x'");
				return "0";
			}

			has_x = true; }
		/*else if (ch == 'l') { 
			has_l = true; 
		}*/
		else if (ch == '.') { 
			if (has_dot || has_x) {
				report_error("Unkown number modifier '.'");
				return "0";
			}
			has_dot = true; 
		}
	}

	return str;
}

String Lexer::peek_next_string() {
	String str = "";

	char ch = peek_next_character();
	while (ch != '"' && ch != '0') {
		eat_character();

		str += ch;

		ch = peek_next_character();
		if (ch == '\\') {
			eat_character();
			ch = peek_next_character();

			if (ch == '"') {
				str += '"';

				eat_character();
				ch = peek_next_character();
			}
			else {
				str += '\\' + ch;
				//report_warning(L"Unkown escape char '%c'", ch);
			}
		}
	}
	eat_character();

	return str;
}

//This can peek keyword, string or number
String Lexer::peek_next_word(int& token_type) {
	String str = "";

	int character = peek_next_character();
	
	if (character == '"') {
		eat_character();
		str = peek_next_string();
		token_type = TOKEN_STRING;
	}
	else if (character >= '0' && character <= '9') {
		str = peek_next_numer();
		token_type = TOKEN_NUMBER;
	}
	else {
		while (isidentity((character = peek_next_character()))) {
			eat_character();
			str += static_cast<char>(character);
		}
		token_type = TOKEN_IDENT;
	}

	if (str.size == 0) return "\0";
	return str;
}

Token* Lexer::new_token() {
	Token* token = new Token();
	token->file_name = this->current_file_name;
	token->row = this->current_row;
	token->column = this->current_col;
	token->index = this->tokens.size();

	return token;
}

int Lexer::decide_token_keyword(const char* word) {
	if (COMPARE(word, "true"))			return TOKEN_KEYWORD_TRUE;
	if (COMPARE(word, "false"))			return TOKEN_KEYWORD_FALSE;
	if (COMPARE(word, "if"))			return TOKEN_KEYWORD_IF;
	if (COMPARE(word, "else"))			return TOKEN_KEYWORD_ELSE;
	if (COMPARE(word, "for"))			return TOKEN_KEYWORD_FOR;
	if (COMPARE(word, "new"))			return TOKEN_KEYWORD_NEW;	
	if (COMPARE(word, "return"))		return TOKEN_KEYWORD_RETURN;
	if (COMPARE(word, "enum"))			return TOKEN_KEYWORD_ENUM;
	if (COMPARE(word, "struct"))		return TOKEN_KEYWORD_STRUCT;
	if (COMPARE(word, "defer"))			return TOKEN_KEYWORD_DEFER;
	if (COMPARE(word, "constructor"))	return TOKEN_KEYWORD_CONSTRUCTOR;
	if (COMPARE(word, "destructor"))	return TOKEN_KEYWORD_DESCRUCTOR;

	if (COMPARE(word, "float"))			return TOKEN_KEYWORD_FLOAT;
	if (COMPARE(word, "long"))			return TOKEN_KEYWORD_LONG;
	if (COMPARE(word, "string"))		return TOKEN_KEYWORD_STRING;

	if (COMPARE(word, "s8"))			return TOKEN_KEYWORD_S8;
	if (COMPARE(word, "s16"))			return TOKEN_KEYWORD_S16;
	if (COMPARE(word, "s32"))			return TOKEN_KEYWORD_S32;
	if (COMPARE(word, "s64"))			return TOKEN_KEYWORD_S64;
	if (COMPARE(word, "u8"))			return TOKEN_KEYWORD_U8;
	if (COMPARE(word, "u16"))			return TOKEN_KEYWORD_U16;
	if (COMPARE(word, "u32"))			return TOKEN_KEYWORD_U32;
	if (COMPARE(word, "u64"))			return TOKEN_KEYWORD_U64;
	return 0;
}

void Lexer::lex() {
	TokenDefine token_type;

	while (token_type.token_type != TOKEN_EOF) {
		peek_next_character();		
		token_type = get_next_token();	

		if (token_type.type == TOKEN_NOP) {
			continue;
		}

		auto token = new_token();
		if (token_type.type == 0) {			
			token->type = token_type.token_type;	
			token->column -= token_type.value_string.size;
		}
		else if (token_type.type == TOKEN_IDENT) {
			token->type = token_type.type;
			token->value = token_type.value_string;
			token->column -= token_type.value_string.size;
		}
		else if (token_type.type == TOKEN_KEYWORD) {
			token->type = token_type.token_type;
			token->value = token_type.value_string;
			token->column -= token_type.value_string.size;
		}
		else if (token_type.type == TOKEN_STRING) {
			token->type = token_type.type;
			token->value = token_type.value_string;
			token->column -= token_type.value_string.size;
		}
		else if (token_type.type == TOKEN_NUMBER) {
			token->type = token_type.type;
			token->value = token_type.value_string;
			auto number_as_string = token->value;
			const char* wvalue = number_as_string.data;
			token->column -= strlen(wvalue);

			if (Utils::strContChar(wvalue, 'x')) {
				token->number_flags |= TOKEN_NUMBER_FLAG_HEX;
			}
			if (Utils::strContChar(wvalue, '.')) {
				token->number_flags |= TOKEN_NUMBER_FLAG_FLOAT;
				token->number_value_f = strtof(wvalue, NULL);
			}
			/*else if (wvalue[strlen(wvalue) - 1] == 'l') {
				token->number_flags |= TOKEN_NUMBER_FLAG_LONG;
				token->number_value_l = atol(wvalue);
			}*/
			else {
				token->number_flags |= TOKEN_NUMBER_FLAG_INT;

				if (token->number_flags & TOKEN_NUMBER_FLAG_HEX) {
					const char* ivalue = wvalue + 2; //skip 0x
					token->number_value_l = std::stol(ivalue, nullptr, 16);//atoi(ivalue);
				}
				else {
					token->number_value_i = atoi(wvalue);
				}
			}
		}
		this->tokens.push_back(token);
	}
}

void Lexer::rollback_eat_char() {
	this->pos--;
	this->current_col--;
}

TokenDefine Lexer::get_next_token() {
	eat_white();

	char character = peek_next_character();
	eat_character();

	String token_value_string = "";
	int token = 0;
	int type = 0;

	switch (character) {
	case '+':
		if (peek_next_character() == '+') {
			eat_character();
			token_value_string = "++";
			token = TOKEN_INCREMENT;
			break;
		}
		token = TOKEN_PLUS;
		break;
	case '-':
		if (peek_next_character() == '>') {
			token_value_string = "->";
			eat_character();
			token = TOKEN_RETURNTYPE;
			break;
		}
		if (peek_next_character() == '-') {
			token_value_string = "--";
			eat_character();
			token = TOKEN_DECREMENT;
			break;
		}
		token = TOKEN_MINUS;
		break;
	case '*':
		token = TOKEN_MUL;
		break;
	case '/':
		if (peek_next_character() == '/') {
			token_value_string = "//";
			eat_character();
			eat_comment();
			type = TOKEN_NOP;
			break;
		}
		token = TOKEN_DIV;
		break;
	case '#':
		token = TOKEN_DIRECTIVE;
		break;
	case '%':
		token = TOKEN_MOD;
		break;
	case '.':
		if (peek_next_character() == '.') {
			token_value_string = "..";
			eat_character();
			token = TOKEN_RANGE;
			break;
		}
		token = TOKEN_DOT;
		break;
	case ',':
		token = TOKEN_COMMA;
		break;
	case '(':
		token = TOKEN_LPARENTHESE;
		break;
	case ')':
		token = TOKEN_RPARENTHESE;
		break;
	case '[':
		token = TOKEN_LBRACKET;
		break;
	case ']':
		token = TOKEN_RBRACKET;
		break;
	case '{':
		token = TOKEN_LBLOCK;
		break;
	case '}':
		token = TOKEN_RBLOCK;
		break;
	case ';':
		token = TOKEN_SEMICOLON;
		break;
	case '&':
		if (peek_next_character() == '&') {
			token_value_string = "&&";
			eat_character();
			token = TOKEN_AND;
			break;
		}
		token = TOKEN_BAND;
		break;
	case '|':
		if (peek_next_character() == '|') {
			token_value_string = "||";
			eat_character();
			token = TOKEN_OR;
			break;
		}
		token = TOKEN_BOR;
		break;
	case '!':
		if (COMPARE_CH(peek_next_character(), '=')) {
			token_value_string = "!=";
			eat_character();
			token = TOKEN_NOTEQUAL;
			break;
		}
		token = TOKEN_EXCLAMATION;
		break;
	case '=':
		if (peek_next_character() == '=') {
			token_value_string = "==";
			eat_character();
			token = TOKEN_EQUAL;
			break;
		}
		token = TOKEN_ASING;
		break;
	case ':':
		token = TOKEN_COLON;
		break;
	case '?':
		token = TOKEN_QUESTIONMARK;
		break;
	case '>':
		if (peek_next_character() == '=') {
			token_value_string = ">=";
			eat_character();
			token = TOKEN_MOREEQUAL;
			break;
		}
		token = TOKEN_MORE;
		break;
	case '<':
		if (peek_next_character() == '=') {
			token_value_string = ">=";
			eat_character();
			token = TOKEN_LESSEQUAL;
			break;
		}
		token = TOKEN_LESS;
		break;
	case '$':
		token = TOKEN_DOLAR;
		break;
	case '\0':
		token = TOKEN_EOF;
		break;
	default:
		rollback_eat_char();
		token_value_string = peek_next_word(type);

		token = decide_token_keyword(token_value_string.data);
		if(token != 0)
			type = TOKEN_KEYWORD;
		//rollback_eat_char();
		break;
	}

	//report_warning(L"Found: %c [ %ls ]", token, token_value_string.c_str());

	TokenDefine token_type;
	token_type.token_type = token;
	token_type.type = type;
	if (token_value_string != "") {
		token_type.value_string = token_value_string;
	}

	return token_type;
}


inline bool Lexer::isidentity(char c) {
	return (c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'));
}