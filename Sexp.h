#ifndef SEXP_H
#define SEXP_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOL_LENGTH 30
#define MAX_DISPLAY_SEXP 400
#define MAX_FILE_NAME 100

#define HEAP_BLOCK_SIZE 500
#define ENV_SIZE 10000

enum Sexp_kind {
    Symbol, Nil, Cons
};
typedef enum Sexp_kind Sexp_kind;

typedef struct Sexp Sexp;
struct Sexp {
    Sexp_kind kind;
    int memflag; // 0: unallocated, 1: unmarked, 2: marked
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
Sexp* construct_symbol(Sexp* s, char* name);
Sexp* construct_cons(Sexp* s, Sexp* s1, Sexp* s2);
Sexp* copy_Sexp(Sexp* dest, Sexp* src);

void showSexp (Sexp* s, char* result);
void showTail (Sexp* s, char* result);

typedef struct RootSet RootSet;
struct RootSet {
    Sexp* set[ENV_SIZE*4];
    size_t length;
};

void construct_rootSet(RootSet* rootSet);
void copy_set(RootSet* dest, RootSet* src);

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

typedef struct HeapBlock HeapBlock;
struct HeapBlock {
    Sexp** objects;
    HeapBlock* nextBlock;
};

HeapBlock Heap;
size_t HeapSize;
size_t total_bytes_allocated;
size_t total_bytes_freed;
size_t total_garbage_collections;
size_t total_heapblocks_allocated;
void init_heapblock(HeapBlock* heapblock);
void free_heapblock(HeapBlock* heapblock);
void create_heap();
void delete_heap();
HeapBlock* allocate_heap_block();
Sexp* allocate_Sexp(RootSet* rootSet);
void mark(Sexp* node);
int mark_and_sweep(RootSet* rootSet);
void clear_marks();

void readSexp (char* cs, ParseResult* parse_res, RootSet* rootSet);
void readExp (char* cs, size_t i, size_t len, ParseResult* parse_res, RootSet* rootSet);
void readSymbol(char* cs, size_t i, size_t len, ParseResult* parse_res, RootSet* rootSet);
void readTail (char* cs, size_t i, size_t len, ParseResult* parse_res, RootSet* rootSet);
void readSexpAndTail(Sexp* s, char* cs, size_t i, size_t len, ParseResult* parse_res, RootSet* rootSet);
void readQuoteSexpAndTail(Sexp* s, char* cs, size_t i, size_t len, ParseResult* parse_res, RootSet* rootSet);
void readClose (char* cs, size_t i, size_t len, ParseResult* parse_res, RootSet* rootSet);

// debug
void print_Sexp(Sexp* s);
void print_rootSet(RootSet* rootSet);


#endif