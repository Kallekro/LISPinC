#include "Sexp.h"

Sexp* construct_nil(Sexp* s) {
    s->kind = Nil;
    s->allocated = 1;
    return s;
}

Sexp* construct_symbol(Sexp* s, char name[MAX_SYMBOL_LENGTH]) {
    s->kind = Symbol;
    s->allocated = 1;
    strcpy(s->u.symbol.name, name);
    return s;
}

Sexp* construct_cons(Sexp* s, Sexp* s1, Sexp* s2) {
    s->kind = Cons;
    s->allocated = 1;
    s->u.cons.Sexp1 = s1;
    s->u.cons.Sexp2 = s2;
    return s;
}


void showSexp (Sexp* s, char* result) {
    switch (s->kind) {
        case Symbol:
            strcat(result, s->u.symbol.name);
            break;
        case Cons:
            if (s->u.cons.Sexp1->kind == Symbol
            && strcmp(s->u.cons.Sexp1->u.symbol.name, "quote") == 0
            && s->u.cons.Sexp2->kind == Cons
            && s->u.cons.Sexp2->u.cons.Sexp2->kind == Nil) {
                strcat(result, "\'");
                showSexp(s->u.cons.Sexp2, result);
            } else {
                strcat(result, "(");
                showSexp(s->u.cons.Sexp1, result);
                showTail(s->u.cons.Sexp2, result);
            }
            break;
        case Nil:
            strcat(result, "()");
            break;
    }
}

void showTail (Sexp* s, char* result) {
    switch (s->kind) {
        case Symbol:
            strcat(result, " . ");
            strcat(result, s->u.symbol.name);
            strcat(result, ")");
            break;
        case Cons:
            strcat(result, " ");
            showSexp(s->u.cons.Sexp1, result);
            showTail(s->u.cons.Sexp2, result);
            break;
        case Nil:
            strcat(result, ")");
            break;
    }
}

void initialize_heap(SexpHeap* heap) {
    for (int i=0; i < HEAP_SIZE; i++) {
        heap->nodes[i].allocated = 0;
    }
}

void clear_heap(SexpHeap* heap) {
    for (int i=0; i < HEAP_SIZE; i++) {
        heap->nodes[i].allocated = 0;
    }
}

Sexp* allocate_Sexp(SexpHeap* heap) {
    for (int i=0; i < HEAP_SIZE; i++) {
        if (heap->nodes[i].allocated == 0) {
            return &heap->nodes[i];
        }
    }
    return 0;
}

ParseResult* construct_PR_success(ParseResult* p, unsigned int position, Sexp* s) {
    p->kind = Success;
    p->position = position;
    p->success_Sexp = s;
}

ParseResult* construct_PR_error(ParseResult* p, unsigned int position) {
    p->kind = ErrorAt;
    p->position = position;
}

ParseResult* construct_PR_empty(ParseResult* p, Sexp* s) {
    p->kind = Empty;
    p->position = -1;
    p->success_Sexp = construct_nil(s);
}

void readSexp (char* cs, ParseResult* parse_res, SexpHeap* heap) {
    size_t len = strlen(cs);
    size_t j = 0;
    ParseResult tmp_parse_res;
    Sexp* tmp_success_Sexp;
    tmp_success_Sexp = allocate_Sexp(heap);
    construct_PR_empty(&tmp_parse_res, tmp_success_Sexp);

    readExp(cs, 0, len, &tmp_parse_res, heap);
    switch (tmp_parse_res.kind) {
        case Success:
            j = tmp_parse_res.position;
            while (j < len && (cs[j] == ' ' || cs[j] == '\n')) {
                j++;
            }
            construct_PR_success(parse_res, (j == len) ? 0 : j,
                                 tmp_parse_res.success_Sexp);
            break;
        case ErrorAt:
            construct_PR_error(parse_res, tmp_parse_res.position);
            break;
    }
}

void readExp(char* cs, size_t i, size_t len, ParseResult* parse_res, SexpHeap* heap) {
    construct_PR_success(parse_res, i, construct_symbol(parse_res->success_Sexp, "ABC"));
}