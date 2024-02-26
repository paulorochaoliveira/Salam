#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

typedef enum {
	VALUE_TYPE_INT,
	VALUE_TYPE_FLOAT,
	VALUE_TYPE_BOOL,
	VALUE_TYPE_STRING,
} VariableDataType;

typedef struct {
	VariableDataType type;
	union {
		int int_value;
		float float_value;
		char* string_value;
	};	
} VariableData;

typedef struct SymbolTableEntry {
	char* identifier;
	VariableData* data;
	struct SymbolTableEntry* next;
} SymbolTableEntry;

typedef struct {
	SymbolTableEntry** entries;
	size_t size;
	size_t capacity;
} SymbolTable;

typedef struct SymbolTableStack {
	SymbolTable* table;
	struct SymbolTableStack* next;
} SymbolTableStack;

SymbolTableStack* symbolTableStack = NULL;

void free_variable_data(VariableData* val);

SymbolTable* createSymbolTable(size_t capacity)
{
	SymbolTable* table = (SymbolTable*) malloc(sizeof(SymbolTable));
	table->entries = (SymbolTableEntry**) calloc(capacity, sizeof(SymbolTableEntry*));
	table->size = 0;
	table->capacity = capacity;
	return table;
}

void pushSymbolTable()
{
	SymbolTable* table = createSymbolTable(3);
	SymbolTableStack* newScope = (SymbolTableStack*) malloc(sizeof(SymbolTableStack));
	newScope->table = table;
	newScope->next = symbolTableStack;
	symbolTableStack = newScope;
}

void popSymbolTable()
{
	if (symbolTableStack == NULL) {
		return;
	}

	SymbolTableStack* top = symbolTableStack;
	symbolTableStack = top->next;

	SymbolTable* table = top->table;
	for (size_t i = 0; i < table->capacity; ++i) {
		SymbolTableEntry* entry = table->entries[i];
		while (entry != NULL) {
			SymbolTableEntry* next = entry->next;
			free(entry->identifier);
			free(entry);
			entry = next;
		}
	}
	free(table->entries);
	free(table);
	free(top);
}

unsigned int hash(const char* str, size_t capacity)
{
	unsigned int hash = 0;
	while (*str) {
		hash = (hash * 31) + (*str++);
	}
	return hash % capacity;
}

void addToSymbolTable(SymbolTableStack* symbolTableStack, const char* identifier, VariableData* value)
{
	if (symbolTableStack == NULL) {
		return;
	}

	SymbolTable* table = symbolTableStack->table;
	unsigned int index = hash(identifier, table->capacity);

	SymbolTableEntry* entry = (SymbolTableEntry*) malloc(sizeof(SymbolTableEntry));
	entry->identifier = strdup(identifier);
	entry->data = value;
	entry->next = table->entries[index];
	table->entries[index] = entry;
	table->size++;
}

VariableData* findInSymbolTableCurrent(SymbolTableStack* currentScope, const char* identifier)
{
	SymbolTable* table = currentScope->table;
	unsigned int index = hash(identifier, table->capacity);
	SymbolTableEntry* entry = table->entries[index];

	while (entry != NULL) {
		if (strcmp(entry->identifier, identifier) == 0) {
			return entry->data;
		}
		entry = entry->next;
	}

	return NULL;
}

VariableData* findInSymbolTable(SymbolTableStack* currentScope, const char* identifier)
{
	while (currentScope != NULL) {
		VariableData* data = findInSymbolTableCurrent(currentScope, identifier);
		if (data != NULL) {
			return data;
		}

		currentScope = currentScope->next;
	}

	return NULL;
}

typedef struct {
	// TODO
} interpreter_state_t;

typedef enum {
	// values
	TOKEN_TYPE_IDENTIFIER,
	TOKEN_TYPE_NUMBER,
	TOKEN_TYPE_STRING,

	// keywords
	TOKEN_TYPE_FUNCTION, // عملکرد
	TOKEN_TYPE_RETURN, // برگشت
	TOKEN_TYPE_PRINT, // نمایش
	TOKEN_TYPE_IF, // اگر
	TOKEN_TYPE_TRUE, // درست
	TOKEN_TYPE_FALSE, // غلط
	TOKEN_TYPE_ELSEIF, // واگرنه
	TOKEN_TYPE_AND, // و
	TOKEN_TYPE_OR, // یا

	// symbols
	TOKEN_TYPE_SECTION_OPEN, // {
	TOKEN_TYPE_SECTION_CLOSE, // }
	TOKEN_TYPE_PARENTHESE_OPEN, // (
	TOKEN_TYPE_PARENTHESE_CLOSE, // )

	TOKEN_TYPE_PLUS, // +
	TOKEN_TYPE_MINUS, // -

	TOKEN_TYPE_EQUAL, // =
	TOKEN_TYPE_EQUAL_EQUAL, // ==
	TOKEN_TYPE_NOT_EQUAL, // !=
	TOKEN_TYPE_NOT, // !
	TOKEN_TYPE_LESS_THAN, // <
	TOKEN_TYPE_GREATER_THAN, // >
	TOKEN_TYPE_LESS_THAN_EQUAL, // <=
	TOKEN_TYPE_GREATER_THAN_EQUAL, // >=

	// others
	TOKEN_TYPE_EOF,
	TOKEN_TYPE_ERROR,
} token_type_t;

typedef struct {
	const char* keyword;
	token_type_t token_type;
} keyword_mapping_t;

keyword_mapping_t keyword_mapping[] = {
	{"عملکرد", TOKEN_TYPE_FUNCTION},
	{"برگشت", TOKEN_TYPE_RETURN},
	{"نمایش", TOKEN_TYPE_PRINT},
	{"واگرنه", TOKEN_TYPE_ELSEIF},
	{"اگر", TOKEN_TYPE_IF},
	{"درست", TOKEN_TYPE_TRUE},
	{"غلط", TOKEN_TYPE_FALSE},
	{"و", TOKEN_TYPE_AND},
	{"یا", TOKEN_TYPE_OR},
	{NULL, TOKEN_TYPE_ERROR}
};

typedef struct {
	token_type_t type;
	char* value;
	struct {
		size_t length;
		size_t line;
		size_t column;
		size_t end_line;
		size_t end_column;
	} location;
} token_t;

typedef struct {
	size_t size;
	size_t length;
	void** data;
} array_t;

typedef struct {
	char* data;
	int length;
	int index;
	int line;
	int column;

	array_t* tokens;
	int last_char_size;
} lexer_t;

struct ast_node;

typedef struct {
	char* name;
	struct ast_node* body;
} ast_function_declaration_t;

typedef struct {
	struct ast_node* expression;
} ast_statement_return_t;

typedef struct {
	struct ast_node* expression;
} ast_statement_print_t;

typedef struct {
	struct ast_node** statements;
	size_t num_statements;
} ast_block_t;

struct ast_statement_if_t;

typedef struct {
	struct ast_node* expression;
	ast_block_t* block;
	struct ast_statement_if_t* next;
} ast_statement_if_t;

typedef struct {
	token_type_t literal_type;
	char* value;
} ast_literal_t;

typedef struct {
	char* name;
} ast_identifier_t;

typedef struct {
	char* operator;
	struct ast_expression* left;
	struct ast_expression* right;
} ast_expression_binary_t;

typedef struct {
	struct ast_expression* left;
	struct ast_expression* right;
} ast_expression_assignment_t;

typedef enum {
	AST_EXPRESSION_LITERAL,
	AST_EXPRESSION_IDENTIFIER,
	AST_EXPRESSION_BINARY,
	AST_EXPRESSION_ASSIGNMENT,
} ast_expression_type_t;

typedef struct ast_expression {
	ast_expression_type_t type;

	union {
		ast_literal_t* literal;
		ast_identifier_t* identifier;
		ast_expression_binary_t* binary_op;
		ast_expression_assignment_t* assignment;
	} data;
} ast_expression_t;

typedef enum {
	AST_FUNCTION_DECLARATION,
	AST_STATEMENT_RETURN,
	AST_STATEMENT_PRINT,
	AST_STATEMENT_IF,
	AST_STATEMENT_ELSEIF,
	AST_BLOCK,
	AST_EXPRESSION
} ast_node_type_t;

typedef struct ast_node {
	ast_node_type_t type;
	union {
		ast_function_declaration_t* function_declaration;
		ast_statement_return_t* statement_return;
		ast_statement_print_t* statement_print;
		ast_block_t* block;
		ast_expression_t* expression;
	} data;
} ast_node_t;

typedef struct {
	char* name;
	ast_expression_t* expression;
} ast_variable_declaration_t;

typedef struct {
	lexer_t* lexer;
	size_t token_index;
	ast_node_t* ast_tree;
} parser_t;

wchar_t read_token(lexer_t* lexer);
void read_number(lexer_t* lexer, wchar_t ch);
size_t mb_strlen(char* identifier);
void read_identifier(lexer_t* lexer, wchar_t ch);
size_t wchar_length(wchar_t wide_char);

lexer_t* lexer_create(const char* data);
void lexer_free(lexer_t* lexer);
void lexer_lex(lexer_t* lexer);

parser_t* parser_create(lexer_t* lexer);
void parser_free(parser_t* parser);
void parser_parse(parser_t* parser);
ast_node_t* parser_function(parser_t* parser);
ast_node_t* parser_block(parser_t* parser);
ast_node_t* parser_statement(parser_t* parser);
ast_node_t* parser_statement_return(parser_t* parser);
ast_node_t* parser_statement_print(parser_t* parser);
ast_node_t* parser_expression(parser_t* parser);

VariableData* interpreter_expression(ast_expression_t* expr, interpreter_state_t* state);
VariableData* interpreter_operator_binary(ast_expression_binary_t* expr, interpreter_state_t* state);
VariableData* interpreter_literal(ast_literal_t* expr);
VariableData* interpreter_identifier(ast_identifier_t* expr, interpreter_state_t* state);

void interpreter_statement_print(ast_statement_print_t* stmt, interpreter_state_t* state);
void interpreter_statement_return(ast_statement_return_t* stmt, interpreter_state_t* state);
void interpreter_function_declaration(ast_function_declaration_t* stmt, interpreter_state_t* state);
void interpreter_block(ast_node_t* node, interpreter_state_t* state);

typedef ast_expression_t* (*nud_func_t)(parser_t* parser, token_t* token);
typedef ast_expression_t* (*led_func_t)(parser_t* parser, token_t* token, ast_expression_t* left);

typedef struct {
	int precedence;
	nud_func_t nud;
	led_func_t led;
} token_info_t;

ast_expression_t* nud_bool(parser_t* parser, token_t* token);
ast_expression_t* nud_number(parser_t* parser, token_t* token);
ast_expression_t* nud_string(parser_t* parser, token_t* token);
ast_expression_t* nud_identifier(parser_t* parser, token_t* token);
ast_expression_t* nud_parentheses(parser_t* parser, token_t* token);
ast_expression_t* led_plus_minus(parser_t* parser, token_t* token, ast_expression_t* left);
ast_expression_t* led_equal(parser_t* parser, token_t* token, ast_expression_t* left);
ast_expression_t* led_and(parser_t* parser, token_t* token, ast_expression_t* left);
ast_expression_t* led_or(parser_t* parser, token_t* token, ast_expression_t* left);

ast_expression_t* parser_expression_pratt(parser_t* parser, int precedence);

enum {
	PRECEDENCE_LOWEST = 0,    // START FROM HERE
	PRECEDENCE_HIGHEST = 1,   // =
	PRECEDENCE_ANDOR = 2,     // AND OR
	PRECEDENCE_SUM = 3,       // + -
};

token_info_t token_infos[] = {
	[TOKEN_TYPE_TRUE] = {PRECEDENCE_LOWEST, nud_bool, NULL},
	[TOKEN_TYPE_FALSE] = {PRECEDENCE_LOWEST, nud_bool, NULL},
	[TOKEN_TYPE_NUMBER] = {PRECEDENCE_LOWEST, nud_number, NULL},
	[TOKEN_TYPE_STRING] = {PRECEDENCE_LOWEST, nud_string, NULL},
	[TOKEN_TYPE_IDENTIFIER] = {PRECEDENCE_LOWEST, nud_identifier, NULL},
	[TOKEN_TYPE_PARENTHESE_OPEN] = {PRECEDENCE_LOWEST, nud_parentheses, NULL},
	[TOKEN_TYPE_PLUS] = {PRECEDENCE_SUM, NULL, led_plus_minus},
	[TOKEN_TYPE_AND] = {PRECEDENCE_ANDOR, NULL, led_and},
	[TOKEN_TYPE_OR] = {PRECEDENCE_ANDOR, NULL, led_or},
	[TOKEN_TYPE_MINUS] = {PRECEDENCE_SUM, NULL, led_plus_minus},
	[TOKEN_TYPE_EQUAL] = {PRECEDENCE_HIGHEST, NULL, led_equal},
};

char* token_op_type2str(ast_expression_type_t type)
{
	switch (type) {
		case AST_EXPRESSION_LITERAL: return "LITERAL";
		case AST_EXPRESSION_IDENTIFIER: return "IDENTIFIER";
		case AST_EXPRESSION_BINARY: return "BINARY_OP";
		case AST_EXPRESSION_ASSIGNMENT: return "ASSIGNMENT";
		default: return "OP_UNKNOWN";
	}
}

char* token_type2str(token_type_t type)
{
	switch(type) {
		case TOKEN_TYPE_IDENTIFIER: return "IDENTIFIER";
		case TOKEN_TYPE_NUMBER: return "NUMBER";
		case TOKEN_TYPE_STRING: return "STRING";
		case TOKEN_TYPE_FUNCTION: return "FUNCTION";
		case TOKEN_TYPE_RETURN: return "RETURN";
		case TOKEN_TYPE_PRINT: return "PRINT";
		case TOKEN_TYPE_IF: return "IF";
		case TOKEN_TYPE_TRUE: return "TRUE";
		case TOKEN_TYPE_FALSE: return "FALSE";
		case TOKEN_TYPE_ELSEIF: return "ELSEIF";
		case TOKEN_TYPE_OR: return "OR";
		case TOKEN_TYPE_AND: return "AND";
		case TOKEN_TYPE_SECTION_OPEN: return "SECTION_OPEN";
		case TOKEN_TYPE_SECTION_CLOSE: return "SECTION_CLOSE";
		case TOKEN_TYPE_PARENTHESE_OPEN: return "PARENTHESIS_OPEN";
		case TOKEN_TYPE_PARENTHESE_CLOSE: return "PARENTHESIS_CLOSE";
		case TOKEN_TYPE_PLUS: return "PLUS";
		case TOKEN_TYPE_MINUS: return "MINUS";
		case TOKEN_TYPE_EQUAL: return "EQUAL";
		case TOKEN_TYPE_EQUAL_EQUAL: return "EQUAL_EQUAL";
		case TOKEN_TYPE_NOT_EQUAL: return "NOT_EQUAL";
		case TOKEN_TYPE_NOT: return "NOT";
		case TOKEN_TYPE_LESS_THAN: return "LESS_THAN";
		case TOKEN_TYPE_GREATER_THAN: return "GREATER_THAN";
		case TOKEN_TYPE_LESS_THAN_EQUAL: return "LESS_THAN_EQUAL";
		case TOKEN_TYPE_GREATER_THAN_EQUAL: return "GREATER_THAN_EQUAL";
		case TOKEN_TYPE_EOF: return "EOF";
		case TOKEN_TYPE_ERROR: return "ERROR";
		default: return "TOK_UNKNOWN";
	}
}

char* file_read(char* file_Name)
{
	FILE* file = fopen(file_Name, "r");
	if (file == NULL) {
		printf("Error: file not found\n");
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* file_data = (char*) malloc(file_size + 1);
	fread(file_data, 1, file_size, file);
	file_data[file_size] = 0;

	fclose(file);
	return file_data;
}

token_t* token_create(token_type_t type, const char* value, int a, int b, int c, int b2, int c2)
{
	token_t* t = (token_t*) malloc(sizeof(token_t));
	t->type = type;
	t->value = strdup(value);
	t->location.length = a;
	t->location.line = b;
	t->location.column = c;
	t->location.end_line = b2;
	t->location.end_column = c2;

	return t;
}

array_t* array_create(size_t size)
{
	size_t min_size = 1;

	array_t* arr = malloc(sizeof(array_t));
	arr->length = 0;
	arr->size = size > min_size ? size : min_size;
	arr->data = malloc(sizeof(void*) * arr->size);
	return arr;
}

void array_push(array_t* arr, void* data)
{
	if (arr->length >= arr->size) {
		size_t new_size = arr->size * 2;
		arr->data = realloc(arr->data, sizeof(void*) * new_size);
		arr->size = new_size;
	}

	arr->data[arr->length++] = data;
}

void array_free(array_t* arr)
{
	free(arr->data);
	free(arr);
}

void token_print(token_t* t)
{
	printf("%d ", t->type);
	// printf("...\n");
	// printf("%zu - ", t->location.length);
	printf("%s - ", token_type2str(t->type));
	printf("%s\n", t->value);
}

void array_print(array_t* arr)
{
	// printf("Array Length: %zu\n", arr->length);
	// printf("Array Size: %zu\n", arr->size);
	// printf("Array Contents:\n");

	for (size_t i = 0; i < arr->length; i++) {
		token_t* t = arr->data[i];
		printf("[%zu]: ", i);
		token_print(t);
	}
}

lexer_t* lexer_create(const char* data)
{
	lexer_t* lexer = (lexer_t*) malloc(sizeof(lexer_t));
	lexer->data = (char*) data;
	lexer->index = 0;
	lexer->tokens = array_create(10);
	lexer->length = strlen(data);

	return lexer;
}

void lexer_free(lexer_t* lexer)
{
	array_free(lexer->tokens);
	free(lexer);
}

bool is_number(wchar_t ch)
{
	return ch >= L'۰' && ch <= L'۹';
}

bool is_alpha(wchar_t ch)
{
	return ch >= L'آ' && ch <= L'ی' || ch == L'_';
}

bool is_ident(wchar_t ch)
{
	return is_alpha(ch) || is_number(ch);
}

wchar_t read_token(lexer_t* lexer)
{
	wchar_t current_char;
	int char_size = mbtowc(&current_char, &lexer->data[lexer->index], MB_CUR_MAX);
	if (char_size < 0) {
		printf("Syntax Error: invalid unicode character\n");
		exit(EXIT_FAILURE);
		return 0;
	}

	if (current_char == '\n') {
		lexer->line++;
		lexer->column = 0;
	} else {
		lexer->column += char_size;
	}

	lexer->index += char_size;
	lexer->last_char_size = char_size;

	return current_char;
}

wchar_t unread_token(lexer_t* lexer)
{
	lexer->index -= lexer->last_char_size;
	lexer->column -= lexer->last_char_size;

	wchar_t current_char;
	int char_size = mbtowc(&current_char, &lexer->data[lexer->index], MB_CUR_MAX);
	if (char_size < 0) {
		printf("Syntax Error: invalid unicode character\n");
		exit(EXIT_FAILURE);
		return 0;
	}

	return current_char;
}

void read_number(lexer_t* lexer, wchar_t ch)
{
	char number[21];
	int i = 0;
	while (is_number(ch)) {
		number[i++] = ch - L'۰' + '0';
		ch = read_token(lexer);
	}
	number[i] = 0;

	token_t* t = token_create(TOKEN_TYPE_NUMBER, number, i, lexer->line, lexer->column - i, lexer->line, lexer->column);
	array_push(lexer->tokens, t);

	unread_token(lexer);
}

void read_string(lexer_t* lexer, wchar_t ch)
{
    char* string = (char*) malloc(sizeof(char) * 1024); // 1023 + 1 for null terminator
    int i = 0;

    while (ch != L'"') {
        if (i >= 1023) {
            printf("Error: String length exceeds the maximum allowed length.\n");
            exit(EXIT_FAILURE);
        }

        int char_size = wctomb(&string[i], ch);
        if (char_size < 0) {
            printf("Error: Failed to convert wide character to multibyte\n");
            exit(EXIT_FAILURE);
        }
        i += char_size;

        ch = read_token(lexer);
    }

    string[i] = '\0';

    token_t* t = token_create(TOKEN_TYPE_STRING, string, i, lexer->line, lexer->column - i, lexer->line, lexer->column);
    array_push(lexer->tokens, t);

    free(string);
}

size_t mb_strlen(char* identifier)
{
	size_t wcs_len = mbstowcs(NULL, identifier, 0);
	if (wcs_len == (size_t)-1) {
		perror("Error in mbstowcs");
		exit(EXIT_FAILURE);
	}

	return wcs_len;
}

void read_identifier(lexer_t* lexer, wchar_t ch)
{
	char identifier[256];
	int i = 0;
	while (is_ident(ch)) {
		int char_size = wctomb(&identifier[i], ch);
		if (char_size < 0) {
			printf("Error: Failed to convert wide character to multibyte\n");
			exit(EXIT_FAILURE);
		}
		i += char_size;
		ch = read_token(lexer);
	}
	identifier[i] = 0;

	int mapping_index = 0;
	token_type_t type = TOKEN_TYPE_IDENTIFIER;
	while (keyword_mapping[mapping_index].keyword != NULL) {
		if (strcmp(identifier, keyword_mapping[mapping_index].keyword) == 0) {
			type = keyword_mapping[mapping_index].token_type;
			break;
		}
		mapping_index++;
	}

	size_t length = mb_strlen(identifier);
	token_t* t = token_create(type, identifier, length, lexer->line, lexer->column - length, lexer->line, lexer->column);
	array_push(lexer->tokens, t);

	unread_token(lexer);
}

size_t wchar_length(wchar_t wide_char)
{
	char mb_char[MB_LEN_MAX];
	if (wcrtomb(mb_char, wide_char, NULL) == (size_t)-1) {
		perror("Error in wcrtomb");
		return 0;
	}

	return mbrlen(mb_char, MB_LEN_MAX, NULL);
}

void lexer_lex(lexer_t* lexer)
{
	while (lexer->data[lexer->index] != 0) {
		char current_char = lexer->data[lexer->index];

		if (current_char == '\a' || current_char == '\r' || current_char == ' ' || current_char == '\t') {
			lexer->column++;
			lexer->index++;
			continue;
		} else if (current_char == '\n') {
			lexer->index++;
			lexer->line++;
			lexer->column = 0;
			continue;
		}

		wchar_t current_wchar = read_token(lexer);
		if (current_wchar == L'\u200C') {
			lexer->index++;
			lexer->column++;
			continue;
		} else if (current_wchar == '{') {
			token_t* t = token_create(TOKEN_TYPE_SECTION_OPEN, "{", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_wchar == '}') {
			token_t* t = token_create(TOKEN_TYPE_SECTION_CLOSE, "}", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_wchar == '(') {
			token_t* t = token_create(TOKEN_TYPE_PARENTHESE_OPEN, "(", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_wchar == ')') {
			token_t* t = token_create(TOKEN_TYPE_PARENTHESE_CLOSE, ")", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_wchar == '+') {
			token_t* t = token_create(TOKEN_TYPE_PLUS, "+", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_wchar == '-') {
			token_t* t = token_create(TOKEN_TYPE_MINUS, "-", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_char == '=') {
			if (lexer->index + 1 < lexer->length && lexer->data[lexer->index + 1] == '=') {
				token_t* t = token_create(TOKEN_TYPE_EQUAL_EQUAL, "==", 2, lexer->line, lexer->column - 2, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
				lexer->index++;
				lexer->column++;
			} else {
				token_t* t = token_create(TOKEN_TYPE_EQUAL, "=", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
			}
		} else if (current_char == '!') {
			if (lexer->index + 1 < lexer->length && lexer->data[lexer->index + 1] == '=') {
				token_t* t = token_create(TOKEN_TYPE_NOT_EQUAL, "!=", 2, lexer->line, lexer->column - 2, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
				lexer->index++;
				lexer->column++;
			} else {
				token_t* t = token_create(TOKEN_TYPE_NOT, "!", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
			}
		} else if (current_char == '>') {
			if (lexer->index + 1 < lexer->length && lexer->data[lexer->index + 1] == '=') {
				token_t* t = token_create(TOKEN_TYPE_GREATER_THAN_EQUAL, ">=", 2, lexer->line, lexer->column - 2, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
				lexer->index++;
				lexer->column++;
			} else {
				token_t* t = token_create(TOKEN_TYPE_GREATER_THAN, ">", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
			}
		} else if (current_char == '<') {
			if (lexer->index + 1 < lexer->length && lexer->data[lexer->index + 1] == '=') {
				token_t* t = token_create(TOKEN_TYPE_LESS_THAN_EQUAL, "<=", 2, lexer->line, lexer->column - 2, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
				lexer->index++;
				lexer->column++;
			} else {
				token_t* t = token_create(TOKEN_TYPE_LESS_THAN, "<", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
			}
		} else if (current_wchar == '\"') {
			current_wchar = read_token(lexer);
			read_string(lexer, current_wchar);
		} else if (is_number(current_wchar)) {
			read_number(lexer, current_wchar);
		} else if (is_alpha(current_wchar)) {
			read_identifier(lexer, current_wchar);
		} else {
			printf("Error: Unexpected character '%c' at line %d, column %d\n", current_char, lexer->line, lexer->column - 1);

			token_t* t = token_create(TOKEN_TYPE_ERROR, (char[]){current_char,'\0'}, 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		}
	}

	if (lexer->data[lexer->index] == 0) {
		token_t* t = token_create(TOKEN_TYPE_EOF, "\0", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
		array_push(lexer->tokens, t);
	}
}

void help()
{
	printf("Welcome to Sallam Programming Language!\n");
	printf("Sallam is the first Persian/Iranian computer scripting language.\n");
	printf("\n");

	printf("Usage:\n");
	printf("  sallam <filename>\t\t\t# Execute a Sallam script\n");
	printf("\n");

	printf("Example:\n");
	printf("  sallam my_script.sallam\t\t# Run the Sallam script 'my_script.sallam'\n");
	printf("\n");

	printf("Feel free to explore and create using Sallam!\n");
	printf("For more information, visit: https://sallam-lang.js.org\n");
	printf("\n");
}

parser_t* parser_create(lexer_t* lexer)
{
	parser_t* parser = (parser_t*) malloc(sizeof(parser_t));
	parser->lexer = lexer;
	parser->token_index = 0;
	parser->ast_tree = NULL;

	return parser;
}

void ast_expression_free(ast_expression_t* expr)
{
	switch (expr->type) {
		case AST_EXPRESSION_LITERAL:
			free(expr->data.literal);
			break;
		case AST_EXPRESSION_IDENTIFIER:
			free(expr->data.identifier);
			break;
		case AST_EXPRESSION_BINARY:
			free(expr->data.binary_op->operator);
			ast_expression_free(expr->data.binary_op->left);
			ast_expression_free(expr->data.binary_op->right);
			break;
	}
}

void ast_node_free(ast_node_t* node)
{
	if (node == NULL) {
		return;
	}

	switch (node->type) {
		case AST_FUNCTION_DECLARATION:
			free(node->data.function_declaration->name);
			ast_node_free((ast_node_t*) node->data.function_declaration->body);
			break;
		case AST_STATEMENT_RETURN:
			ast_node_free((ast_node_t*) node->data.statement_return->expression);
			break;
		case AST_BLOCK:
			for (size_t i = 0; i < node->data.block->num_statements; i++) {
				ast_node_free((ast_node_t*) node->data.block->statements[i]);
			}
			free(node->data.block->statements);
			break;
		case AST_EXPRESSION:
			ast_expression_free(node->data.expression);
			break;
	}

	free(node);
}

void parser_free(parser_t* parser)
{
	ast_node_free(parser->ast_tree);
	lexer_free(parser->lexer);
	free(parser);
}

void parser_token_next(parser_t* parser)
{
	if (parser->lexer->tokens->length > parser->token_index) {
		parser->token_index++;
	} else {
		printf("Error: Unexpected end of file\n");
		exit(EXIT_FAILURE);
	}
}

token_t* parser_token_skip(parser_t* parser, token_type_t type)
{
	if (parser->lexer->tokens->length > parser->token_index) {
		token_t* token = (token_t*)parser->lexer->tokens->data[parser->token_index];

		if (token->type == type) {
			return (token_t*)parser->lexer->tokens->data[parser->token_index++];
		}
	}

	return NULL;
}

token_t* parser_token_eat(parser_t* parser, token_type_t type)
{
	if (parser->lexer->tokens->length > parser->token_index) {
		token_t* token = (token_t*)parser->lexer->tokens->data[parser->token_index];

		if (token->type == type) {
			return (token_t*)parser->lexer->tokens->data[parser->token_index++];
		} else {
			printf("Error: Expected %s\n", token_type2str(type));
			exit(EXIT_FAILURE);
		}
	} else {
		printf("Error: Unexpected end of file\n");
		exit(EXIT_FAILURE);
	}

	return NULL;
}

ast_node_t* parser_function(parser_t* parser)
{
	printf("Parsing function\n");

	ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
	node->type = AST_FUNCTION_DECLARATION;

	parser_token_eat(parser, TOKEN_TYPE_FUNCTION);

	token_t* name = parser_token_eat(parser, TOKEN_TYPE_IDENTIFIER);
	token_print(name);
	printf("FUNCTION NAME: %s\n", name->value);

	// if (parser->lexer->tokens->length > parser->token_index && ((token_t*)parser->lexer->tokens->data[parser->token_index])->type == TOKEN_TYPE_PARENTHESE_OPEN) {
	//     printf("Parsing parameters\n");
	//     parser->token_index++;

	//     if (parser->lexer->tokens->length > parser->token_index && ((token_t*) parser->lexer->tokens->data[parser->token_index])->type == TOKEN_TYPE_PARENTHESE_CLOSE) {
	//         parser->token_index++;
	//     } else {
	//         printf("Error: Expected closing parenthesis\n");
	//         exit(EXIT_FAILURE);
	//     }
	// }

	node->data.function_declaration = (ast_function_declaration_t*) malloc(sizeof(ast_function_declaration_t));
	node->data.function_declaration->name = strdup(name->value);
	// free(name);
	// node->data.function_declaration->name = (name->value);
	// printf("===>!\n");
	// node->data.function_declaration->name = "name";
	// malloc(sizeof(char) * 10);
	// strcpy(node->data.function_declaration->name, "name");
	node->data.function_declaration->body = parser_block(parser);

	return node;
}

ast_node_t* parser_statement_print(parser_t* parser) {
	printf("Parsing statement print\n");

	parser->token_index++;

	ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
	node->type = AST_STATEMENT_PRINT;

	node->data.statement_print = (ast_statement_print_t*) malloc(sizeof(ast_statement_print_t));
	node->data.statement_print->expression = parser_expression(parser);

	return node;
}

ast_node_t* parser_statement_return(parser_t* parser) {
	printf("Parsing statement return\n");

	parser->token_index++;

	ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
	node->type = AST_STATEMENT_RETURN;

	node->data.statement_return = (ast_statement_return_t*) malloc(sizeof(ast_statement_return_t));
	node->data.statement_return->expression = parser_expression(parser);

	return node;
}

ast_node_t* parser_statement(parser_t* parser)
{
	printf("Parsing statement\n");

	ast_node_t* stmt = NULL;

	if (parser->lexer->tokens->length > parser->token_index) {
		token_t* tok = (token_t*)parser->lexer->tokens->data[parser->token_index];

		switch (tok->type) {
			case TOKEN_TYPE_RETURN:
				stmt = parser_statement_return(parser);
				break;
			
			case TOKEN_TYPE_PRINT:
				stmt = parser_statement_print(parser);
				break;
			
			case TOKEN_TYPE_SECTION_OPEN:
				stmt = parser_block(parser);
				break;
			
			case TOKEN_TYPE_IDENTIFIER:
			case TOKEN_TYPE_PLUS:
			case TOKEN_TYPE_MINUS:
			case TOKEN_TYPE_PARENTHESE_OPEN:
			case TOKEN_TYPE_STRING:
			case TOKEN_TYPE_NUMBER:
				stmt = parser_expression(parser);
				break;
		}
	}
	
	if (stmt == NULL) {
		printf("Error: Unexpected token as statement %s\n", token_type2str(((token_t*)parser->lexer->tokens->data[parser->token_index])->type));
		exit(EXIT_FAILURE);
	}
	return stmt;
}

ast_node_t* parser_expression(parser_t* parser)
{
	printf("Parsing expression\n");

	ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t) * 2);
	node->type = AST_EXPRESSION;
	node->data.expression = parser_expression_pratt(parser, PRECEDENCE_LOWEST);

	return node;
}

ast_expression_t* parser_expression_pratt(parser_t* parser, int precedence)
{
	printf("Parsing pratt\n");

	token_t* current_token = (token_t*)parser->lexer->tokens->data[parser->token_index];
	parser->token_index++;

	ast_expression_t* left = token_infos[current_token->type].nud(parser, current_token);

	while (precedence < token_infos[((token_t*)parser->lexer->tokens->data[parser->token_index])->type].precedence) {
		current_token = (token_t*)parser->lexer->tokens->data[parser->token_index];
		parser->token_index++;

		left = token_infos[current_token->type].led(parser, current_token, left);
	}

	return left;
}

ast_expression_t* led_equal(parser_t* parser, token_t* token, ast_expression_t* left)
{
	printf("Parsing operator binary\n");

	ast_expression_t* right = parser_expression_pratt(parser, token_infos[token->type].precedence);

	ast_expression_t* binary_op_expr = (ast_expression_t*) malloc(sizeof(ast_expression_t));
	binary_op_expr->type = AST_EXPRESSION_ASSIGNMENT;
	binary_op_expr->data.assignment = (ast_expression_assignment_t*) malloc(sizeof(ast_expression_assignment_t));
	binary_op_expr->data.assignment->left = left;
	binary_op_expr->data.assignment->right = right;

	return binary_op_expr;
}

ast_expression_t* led_and(parser_t* parser, token_t* token, ast_expression_t* left)
{
	printf("Parsing operator and\n");

	ast_expression_t* right = parser_expression_pratt(parser, token_infos[token->type].precedence);

	ast_expression_t* binary_op_expr = (ast_expression_t*) malloc(sizeof(ast_expression_t));
	binary_op_expr->type = AST_EXPRESSION_BINARY;
	binary_op_expr->data.binary_op = (ast_expression_binary_t*) malloc(sizeof(ast_expression_binary_t));
	binary_op_expr->data.binary_op->operator = strdup(token->value);
	binary_op_expr->data.binary_op->left = left;
	binary_op_expr->data.binary_op->right = right;

	return binary_op_expr;
}

ast_expression_t* led_or(parser_t* parser, token_t* token, ast_expression_t* left)
{
	printf("Parsing operator and\n");

	ast_expression_t* right = parser_expression_pratt(parser, token_infos[token->type].precedence);

	ast_expression_t* binary_op_expr = (ast_expression_t*) malloc(sizeof(ast_expression_t));
	binary_op_expr->type = AST_EXPRESSION_BINARY;
	binary_op_expr->data.binary_op = (ast_expression_binary_t*) malloc(sizeof(ast_expression_binary_t));
	binary_op_expr->data.binary_op->operator = strdup(token->value);
	binary_op_expr->data.binary_op->left = left;
	binary_op_expr->data.binary_op->right = right;

	return binary_op_expr;
}

ast_expression_t* led_plus_minus(parser_t* parser, token_t* token, ast_expression_t* left)
{
	printf("Parsing operator binary\n");

	ast_expression_t* right = parser_expression_pratt(parser, token_infos[token->type].precedence);

	ast_expression_t* binary_op_expr = (ast_expression_t*) malloc(sizeof(ast_expression_t));
	binary_op_expr->type = AST_EXPRESSION_BINARY;
	binary_op_expr->data.binary_op = (ast_expression_binary_t*) malloc(sizeof(ast_expression_binary_t));
	binary_op_expr->data.binary_op->operator = strdup(token->value);
	binary_op_expr->data.binary_op->left = left;
	binary_op_expr->data.binary_op->right = right;

	return binary_op_expr;
}

ast_expression_t* nud_bool(parser_t* parser, token_t* token)
{
	printf("Parsing bool\n");

	ast_expression_t* literal_expr = malloc(sizeof(ast_expression_t));
	literal_expr->type = AST_EXPRESSION_LITERAL;

	literal_expr->data.literal = malloc(sizeof(ast_literal_t));
	literal_expr->data.literal->value = strdup(token->value);
	literal_expr->data.literal->literal_type = token->type;

	return literal_expr;
}

ast_expression_t* nud_number(parser_t* parser, token_t* token)
{
	printf("Parsing number\n");

	ast_expression_t* literal_expr = malloc(sizeof(ast_expression_t));
	literal_expr->type = AST_EXPRESSION_LITERAL;

	literal_expr->data.literal = malloc(sizeof(ast_literal_t));
	literal_expr->data.literal->value = strdup(token->value);
	literal_expr->data.literal->literal_type = token->type;

	return literal_expr;
}

ast_expression_t* nud_string(parser_t* parser, token_t* token)
{
	printf("Parsing string\n");

	ast_expression_t* literal_expr = malloc(sizeof(ast_expression_t));
	literal_expr->type = AST_EXPRESSION_LITERAL;

	literal_expr->data.literal = malloc(sizeof(ast_literal_t));
	literal_expr->data.literal->literal_type = token->type;
	literal_expr->data.literal->value = strdup(token->value);

	return literal_expr;
}

ast_expression_t* nud_identifier(parser_t* parser, token_t* token)
{
	printf("Parsing identifier\n");

	ast_expression_t* identifier_expr = malloc(sizeof(ast_expression_t));
	identifier_expr->type = AST_EXPRESSION_IDENTIFIER;

	identifier_expr->data.identifier = malloc(sizeof(ast_identifier_t));
	identifier_expr->data.identifier->name = strdup(token->value);

	return identifier_expr;
}

ast_expression_t* nud_parentheses(parser_t* parser, token_t* token)
{
	printf("Parsing parentheses\n");

	ast_expression_t* expression_node = parser_expression_pratt(parser, PRECEDENCE_LOWEST);

	parser_token_eat(parser, TOKEN_TYPE_PARENTHESE_CLOSE);

	return expression_node;
}

ast_node_t* parser_block(parser_t* parser)
{
	printf("Parsing block\n");

	ast_node_t* block_node = malloc(sizeof(ast_node_t));
	block_node->type = AST_BLOCK;
	block_node->data.block = malloc(sizeof(ast_block_t));
	block_node->data.block->num_statements = 0;
	block_node->data.block->statements = malloc(sizeof(ast_node_t*) * 10 + 1);

	parser_token_eat(parser, TOKEN_TYPE_SECTION_OPEN);

	while (
		parser->lexer->tokens->length > parser->token_index &&
		((token_t*) parser->lexer->tokens->data[parser->token_index])->type != TOKEN_TYPE_SECTION_CLOSE
	) {
		ast_node_t* statement = parser_statement(parser);
		// block_node->data.block->statements = realloc(block_node->data.block->statements, (block_node->data.block->num_statements + 1) * sizeof(ast_node_t*));
		block_node->data.block->statements[
			block_node->data.block->num_statements++
		] = statement;
	}
	block_node->data.block->statements[block_node->data.block->num_statements] = NULL;

	parser_token_eat(parser, TOKEN_TYPE_SECTION_CLOSE);

	return block_node;
}

void parser_parse(parser_t* parser)
{
	if (parser->lexer->tokens->length == 1 &&
		((ast_node_t*)parser->lexer->tokens->data[0])->type == TOKEN_TYPE_EOF
	) {
		parser->ast_tree = NULL;
		return;
	}

	while (parser->token_index < parser->lexer->tokens->length) {
		token_t* current_token = (token_t*)parser->lexer->tokens->data[parser->token_index];

		switch (current_token->type) {
			case TOKEN_TYPE_FUNCTION:
				ast_node_t* function_node = parser_function(parser);
				parser->ast_tree = function_node;
				break;

			// case TOKEN_TYPE_SECTION_OPEN:
			// 	ast_node_t* block_node = parser_block(parser);
			// 	parser->ast_tree = block_node;
			// 	break;

			// case TOKEN_TYPE_RETURN:
			// 	ast_node_t* return_node = parser_statement_return(parser);
			// 	parser->ast_tree = return_node;
			// 	break;

			// default:
			// 	ast_node_t* statement_node = parser_statement(parser);
			// 	parser->ast_tree = statement_node;
			// 	break;

			default:
				printf("Error: bad token as statement\n");
				exit(EXIT_FAILURE);
				break;
		}

		parser->token_index++;
	}
}

void print_indentation(int indent_level)
{
	for (int i = 0; i < indent_level; i++) {
		printf("  ");
	}
}

void print_xml_ast_expression(ast_expression_t* expr, int indent_level)
{
	printf("<Expression>\n");

	switch (expr->type) {
		case AST_EXPRESSION_LITERAL:
			print_indentation(indent_level + 1);
			printf("<Literal>\n");

				print_indentation(indent_level + 2);
				printf("<Type>%s</Type>\n", token_type2str(expr->data.literal->literal_type));

				print_indentation(indent_level + 2);
				printf("<Value>%s</Value>\n", expr->data.literal->value);

			print_indentation(indent_level + 1);
			printf("</Literal>\n");
			break;

		case AST_EXPRESSION_IDENTIFIER:
			print_indentation(indent_level + 1);
			printf("<Identifier>\n");

				print_indentation(indent_level + 2);
				printf("<Name>%s</Name>\n", expr->data.identifier->name);

			print_indentation(indent_level + 1);
			printf("</Identifier>\n");
			break;

		case AST_EXPRESSION_BINARY:
			print_indentation(indent_level + 1);
			printf("<BinaryOperator>\n");

				print_indentation(indent_level + 2);
				printf("<Operator>%s</Operator>\n", expr->data.binary_op->operator);

				print_indentation(indent_level + 2);
				printf("<Left>\n");

					print_indentation(indent_level + 3);
					print_xml_ast_expression(expr->data.binary_op->left, indent_level + 3);

				print_indentation(indent_level + 2);
				printf("</Left>\n");

				print_indentation(indent_level + 2);
				printf("<Right>\n");

					print_indentation(indent_level + 3);
					print_xml_ast_expression(expr->data.binary_op->right, indent_level + 3);

				print_indentation(indent_level + 2);
				printf("</Right>\n");

			print_indentation(indent_level + 1);
			printf("</BinaryOperator>\n");
			break;

		case AST_EXPRESSION_ASSIGNMENT:
			print_indentation(indent_level + 1);
			printf("<Assignment>\n");

				print_indentation(indent_level + 2);
				printf("<Left>\n");

					print_indentation(indent_level + 3);
					print_xml_ast_expression(expr->data.assignment->left, indent_level + 3);

				print_indentation(indent_level + 2);
				printf("</Left>\n");

				print_indentation(indent_level + 2);
				printf("<Right>\n");

					print_indentation(indent_level + 3);
					print_xml_ast_expression(expr->data.assignment->right, indent_level + 3);

				print_indentation(indent_level + 2);
				printf("</Right>\n");

			print_indentation(indent_level + 1);
			printf("</Assignment>\n");
			break;

		default:
			print_indentation(indent_level + 1);
			printf("<!-- Unhandled Expression Type -->\n");
			break;
	}

	print_indentation(indent_level);
	printf("</Expression>\n");
}

void print_xml_ast_node(ast_node_t* node, int indent_level)
{
	if (node == NULL) {
		return;
	}
	
	print_indentation(indent_level);

	switch (node->type) {
		case AST_FUNCTION_DECLARATION:
			printf("<FunctionDeclaration>\n");
			print_indentation(indent_level + 1);

				printf("<Name>%s</Name>\n", node->data.function_declaration->name);
	
				print_xml_ast_node(node->data.function_declaration->body, indent_level + 1);

			print_indentation(indent_level);
			printf("</FunctionDeclaration>\n");
			break;

		case AST_STATEMENT_PRINT:
			printf("<StatementPrint>\n");

				print_xml_ast_node(node->data.statement_print->expression, indent_level + 1);

			print_indentation(indent_level);
			printf("</StatementPrint>\n");
			break;
			
		case AST_STATEMENT_RETURN:
			printf("<StatementReturn>\n");

				print_xml_ast_node(node->data.statement_return->expression, indent_level + 1);

			print_indentation(indent_level);
			printf("</StatementReturn>\n");
			break;

		case AST_BLOCK:
			printf("<Block>\n");

			for (size_t i = 0; i < node->data.block->num_statements; i++) {
				print_xml_ast_node(node->data.block->statements[i], indent_level + 1);
			}

			print_indentation(indent_level);
			printf("</Block>\n");
			break;

		case AST_EXPRESSION:
			print_xml_ast_expression(node->data.expression, indent_level);
			break;

		default:
			print_indentation(indent_level);
			printf("<!-- Unhandled AST Node Type -->\n");
			break;
	}
}

void print_xml_ast_tree(ast_node_t* root)
{
	printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	printf("<AST>\n");

		print_xml_ast_node(root, 1);

	printf("</AST>\n");
}

void interpreter_create()
{

}

bool interpreter_interpret(ast_node_t* node, interpreter_state_t* state)
{
	if (node == NULL) {
		return false;
	}

	switch (node->type) {
		case AST_BLOCK:
			interpreter_block(node, state);
			break;

		case AST_FUNCTION_DECLARATION:
			interpreter_function_declaration(node->data.function_declaration, state);
			break;

		case AST_STATEMENT_RETURN:
			interpreter_statement_return(node->data.statement_return, state);
			return true;
			break;

		case AST_STATEMENT_PRINT:
			interpreter_statement_print(node->data.statement_print, state);
			break;

		case AST_EXPRESSION:
			interpreter_expression(node->data.expression, state);
			break;

		default:
			break;
	}

	return false;
}

void interpreter_function_declaration(ast_function_declaration_t* stmt, interpreter_state_t* state)
{
	// printf("Function Declaration: %s\n", stmt->name);

	interpreter_block(stmt->body, state);
}

void interpreter_expression_data(VariableData* data)
{
	if (data == NULL) {
		printf("NULL\n");
		return;
	} else if (data->type == VALUE_TYPE_INT) {
		printf("%d\n", data->int_value);
	} else if (data->type == VALUE_TYPE_FLOAT) {
		printf("%f\n", data->float_value);
	} else if (data->type == VALUE_TYPE_BOOL) {
		printf("%s\n", data->int_value == 1 ? "True" : "False");
	} else if (data->type == VALUE_TYPE_STRING) {
		printf("%s\n", data->string_value);
	} else {
		printf("Unknown\n");
	}
}

void interpreter_statement_return(ast_statement_return_t* stmt, interpreter_state_t* state)
{
	// printf("Return Statement\n");

	VariableData* res = interpreter_expression(stmt->expression->data.expression, state);
	printf("Return Result: ");
	interpreter_expression_data(res);
	free_variable_data(res);
}

void interpreter_statement_print(ast_statement_print_t* stmt, interpreter_state_t* state)
{
	// printf("Print Statement\n");

	VariableData* res = interpreter_expression(stmt->expression->data.expression, state);
	printf("Print Result: ");
	interpreter_expression_data(res);
	free_variable_data(res);
}

void interpreter_block(ast_node_t* node, interpreter_state_t* state)
{
	// printf("Block\n");

	// Scope entry
	pushSymbolTable(symbolTableStack);

	for (size_t i = 0; i < node->data.block->num_statements; i++) {
		bool res = interpreter_interpret(node->data.block->statements[i], state);

		if (res == true) {
			break;
		}
	}

	// Scope exit
	popSymbolTable(symbolTableStack);
}

VariableData* interpreter_literal(ast_literal_t* expr)
{
	if (expr == NULL) {
		return NULL;
	}

    VariableData* val = (VariableData*) malloc(sizeof(VariableData));

    if (expr->literal_type == TOKEN_TYPE_NUMBER) {
        val->type = VALUE_TYPE_INT;
        val->int_value = atoi(expr->value);
		return val;
    } else if (expr->literal_type == TOKEN_TYPE_STRING) {
        val->type = VALUE_TYPE_STRING;
        val->string_value = strdup(expr->value);
		return val;
    } else if (expr->literal_type == TOKEN_TYPE_TRUE) {
		val->type = VALUE_TYPE_BOOL;
		val->int_value = 1;
		return val;
	} else if (expr->literal_type == TOKEN_TYPE_FALSE) {
		val->type = VALUE_TYPE_BOOL;
		val->int_value = 0;
		return val;
	} else {
        // TODO: Handle other literal types if needed
		printf("Error: No handler for other literal types!\n");
		exit(EXIT_FAILURE);
    }

	free_variable_data(val);
	return NULL;
}

void free_variable_data(VariableData* val)
{
	if (val == NULL) {
		return;
	} else if (val->type == VALUE_TYPE_STRING) {
        free(val->string_value);
    }

    free(val);
}

VariableData* interpreter_identifier(ast_identifier_t* expr, interpreter_state_t* state)
{
	// printf("Variable: %s\n", expr->name);

	VariableData* val = findInSymbolTable(symbolTableStack, expr->name);
	if (val != NULL) {
		// printf("Variable found: %s = %d\n", expr->name, val->value);
		return val;
	} else {
		printf("Error: Variable not found: %s\n", expr->name);
		exit(EXIT_FAILURE);
	}

	free_variable_data(val);
	return NULL;
}

VariableData* interpreter_operator_binary(ast_expression_binary_t* binary_op, interpreter_state_t* state) {
	const char* operator_str = binary_op->operator;

	VariableData* val = (VariableData*) malloc(sizeof(VariableData));
	VariableData* left = interpreter_expression(binary_op->left, state);
	VariableData* right = interpreter_expression(binary_op->right, state);

	if ((left->type != VALUE_TYPE_INT && left->type != VALUE_TYPE_BOOL) || (right->type != VALUE_TYPE_INT && right->type != VALUE_TYPE_BOOL)) {
		printf("Error: cannot calculate binary operator for non-int values!\n");
		exit(EXIT_FAILURE);
	}

	if (strcmp(operator_str, "+") == 0) {
		val->type = VALUE_TYPE_INT;
		val->int_value = left->int_value + right->int_value;
		return val;
	} else if (strcmp(operator_str, "-") == 0) {
		val->type = VALUE_TYPE_INT;
		val->int_value = left->int_value - right->int_value;
		return val;
	} else if (strcmp(operator_str, "*") == 0) {
		val->type = VALUE_TYPE_INT;
		val->int_value = left->int_value * right->int_value;
		return val;
	} else if (strcmp(operator_str, "و") == 0) {
		val->type = VALUE_TYPE_BOOL;
		val->int_value = left->int_value && right->int_value;
		return val;
	} else if (strcmp(operator_str, "یا") == 0) {
		val->type = VALUE_TYPE_BOOL;
		val->int_value = left->int_value || right->int_value;
		return val;
	} else if (strcmp(operator_str, "/") == 0) {
		if (right->int_value == 0) {
			printf("Error: cannot divide by zero!\n");
			exit(EXIT_FAILURE);
		} else {
			val->type = VALUE_TYPE_INT;
			val->int_value = left->int_value / right->int_value;
			return val;
		}
	}

	free_variable_data(left);
	free_variable_data(right);
	free_variable_data(val);
	return NULL;
}

// int interpreter_function_call(ast_node_t* node, interpreter_state_t* state)
// {
//     printf("Function Call: %s\n", node->data.function_call.name);
//     return 0;
// }

VariableData* interpreter_expression_assignment(ast_expression_assignment_t* expr, interpreter_state_t* state)
{
	// printf("Assignment\n");

	if (expr->left->type == AST_EXPRESSION_IDENTIFIER) {
		VariableData* res = interpreter_expression(expr->right, state);

		char* identifier = expr->left->data.identifier->name;
		VariableData* variable = findInSymbolTableCurrent(symbolTableStack, identifier);
		if (variable != NULL) {
			// printf("Variable found: %s = %d\n", identifier, variable->value);
			variable = res;
		} else {
			// printf("Variable not found: %s\n", identifier);
			addToSymbolTable(symbolTableStack, identifier, res);
		}

		return res;
	} else {
		printf("Error: Assignment to non-variable\n");
		exit(EXIT_FAILURE);
	}

	return NULL;
}

VariableData* interpreter_expression(ast_expression_t* expr, interpreter_state_t* state)
{
	if (expr == NULL) {
		return NULL;
	}

	switch (expr->type) {
		case AST_EXPRESSION_LITERAL:
			return interpreter_literal(expr->data.literal);

		case AST_EXPRESSION_IDENTIFIER:
			return interpreter_identifier(expr->data.identifier, state);

		case AST_EXPRESSION_BINARY:
			return interpreter_operator_binary(expr->data.binary_op, state);

		// case AST_EXPRESION_FUNCTION_CALL:
		//     return interpreter_function_call(expr, state);

		case AST_EXPRESSION_ASSIGNMENT:
			return interpreter_expression_assignment(expr->data.assignment, state);

		default:
			printf("Error: default expr type: %d\n", expr->type);
			exit(EXIT_FAILURE);
			// return NULL;
	}
	
	return NULL;
}

void interpreter_free()
{
	while (symbolTableStack != NULL) {
		popSymbolTable();
	}
	free(symbolTableStack);
}

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "");

	if (argc == 1 || argc > 2) {
		help();
		return 0;
	}

	char* file_data = file_read(argv[1]);
	// printf("%s\n", file_data);

	lexer_t* lexer = lexer_create(file_data);
	lexer_lex(lexer);

	array_print(lexer->tokens);

	parser_t* parser = parser_create(lexer);
	parser_parse(parser);

	print_xml_ast_tree(parser->ast_tree);

	interpreter_create();
	interpreter_interpret(parser->ast_tree, NULL);
	interpreter_free();

	exit(EXIT_SUCCESS);
}
