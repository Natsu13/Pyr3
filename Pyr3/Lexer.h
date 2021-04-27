#pragma once

#include "Headers.h"
#include "Token.h"
#include "Interpret.h"
#include "String.h"


class Lexer {
private:
	Interpret* interpreter;
	String code;
	vector<Token*> tokens;
	int pos;
	int token_pos;
	const char* current_file_name = "";
	int current_row = 0;
	int current_col = 0;
	bool peeked = false;

	bool load_file(const char* file_name);

	inline bool isidentity(char c);
	
	int decide_token_keyword(const char* word);
	Token* new_token();
	TokenDefine get_next_token();

	char peek_next_character();
	void eat_character();
	String peek_next_word(int& token_type);
	//void eat_word();
	String peek_next_string();
	String peek_next_numer();
	void eat_white();
	void rollback_eat_char();
	void eat_comment();

	void report_error(String message, ...);
	void report_warning(String  message, ...);

public:
	Lexer(Interpret* interpreter, const char* file_name, String code);
	void lex();

	vector<Token*> getTokens();
	Token* peek_next_token();
	void eat_token();
};