#ifndef SEXP_H
#define SEXP_H
#include <stdio.h>
#include <string.h>

#define MAX_SYMBOL_LENGTH 30
#define MAX_DISPLAY_SEXP 200

#define HEAP_SIZE 10000

enum Sexp_kind {
    Symbol, Nil, Cons
};
typedef enum Sexp_kind Sexp_kind;

typedef struct Sexp Sexp;
struct Sexp {
    Sexp_kind kind;
    int allocated;
    union {
        struct {
            char name[MAX_SYMBOL_LENGTH];
        } symbol;
        struct {
            Sexp* Sexp1;
            Sexp* Sexp2;
        } cons;
    } u;
};

Sexp* construct_nil(Sexp* s);
Sexp* construct_symbol(Sexp* s, char name[MAX_SYMBOL_LENGTH]);
Sexp* construct_cons(Sexp* s, Sexp* s1, Sexp* s2);
Sexp* copy_Sexp(Sexp* dest, Sexp* src);

void showSexp (Sexp* s, char* result);
void showTail (Sexp* s, char* result);

enum ParseResult_kind {
    Success, ErrorAt, Empty
};
typedef enum ParseResult_kind ParseResult_kind;

typedef struct ParseResult ParseResult;
struct ParseResult {
    ParseResult_kind kind;
    unsigned int position;
    Sexp* success_Sexp;
};

ParseResult* construct_PR_success(ParseResult* p, unsigned int position, Sexp* s);
ParseResult* construct_PR_error(ParseResult* p, unsigned int position);
ParseResult* construct_PR_empty(ParseResult* p, Sexp* s);

//ParseResult parseResult;
//Sexp parse_res_s;

Sexp Heap[HEAP_SIZE];
unsigned int total_bytes_allocated;
unsigned int total_bytes_freed;
unsigned int total_garbage_collections;
void clear_heap();
Sexp* allocate_Sexp();


void readSexp (char* cs, ParseResult* parse_res);
void readExp (char* cs, size_t i, size_t len, ParseResult* parse_res);
void readSymbol (char* cs, size_t i, size_t len, ParseResult* parse_res);
void readTail (char* cs, size_t i, size_t len, ParseResult* parse_res);
void readSexpAndTail(Sexp* s, char* cs, size_t i, size_t len, ParseResult* parse_res);
void readQuoteSexpAndTail(Sexp* s, char* cs, size_t i, size_t len, ParseResult* parse_res);
void readClose (char* cs, size_t i, size_t len, ParseResult* parse_res);

// Debugging
void print_Sexp(Sexp* s);
#endif