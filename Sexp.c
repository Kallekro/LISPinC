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

Sexp* copy_Sexp(Sexp* dest, Sexp* src) {
    dest->kind = src->kind;
    dest->allocated = src->allocated;
    switch (src->kind) {
        case Symbol:
            strcpy(dest->u.symbol.name, src->u.symbol.name);
            break;
        case Cons:
            dest->u.cons.Sexp1 = src->u.cons.Sexp1;
            dest->u.cons.Sexp2 = src->u.cons.Sexp2;
            break;
    }
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
    printf("Trigger garbage collection\n");
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
            // skip whitespace
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
    while (i < len && (cs[i] == ' ' || cs[i] == '\n')) {
        i++;
    }
    if (i == len) {
        construct_PR_error(parse_res, i);
        return;
    }
    if ('a' <= cs[i] && 'z' >= cs[i]) {
        readSymbol(cs, i, len, parse_res);
    }
    else if (cs[i] == '(') {
        readTail(cs, i+1, len, parse_res, heap);
    }
    else if (cs[i] == '\'') {
        readExp(cs, i+1, len, parse_res, heap);
        Sexp* s0; Sexp* s1; Sexp* s2; Sexp* s3;
        switch (parse_res->kind) {
            case Success:
                s0 = allocate_Sexp(heap);
                copy_Sexp(s0, parse_res->success_Sexp);
                s1 = allocate_Sexp(heap);
                s2 = allocate_Sexp(heap);
                s3 = allocate_Sexp(heap);
                construct_PR_success(
                    parse_res, parse_res->position,
                    construct_cons(
                        parse_res->success_Sexp,
                        construct_symbol(s1, "quote"),
                        construct_cons(s2, s0, construct_nil(s3)))
                );
                break;
            case ErrorAt:
                break;
        }
    }
    else {
        construct_PR_error(parse_res, i);
    }
}

void readSymbol(char* cs, size_t i, size_t len, ParseResult* parse_res) {
    char name[MAX_SYMBOL_LENGTH];
    size_t name_idx = 0;
    name[name_idx] = cs[i];
    for (++i; i < len; i++) {
        if ('a' <= cs[i] && 'z' >= cs[i]) {
            name[++name_idx] = cs[i];
        } else {
            break;
        }
    }
    construct_PR_success(
        parse_res, i,
        construct_symbol(parse_res->success_Sexp, name)
    );
}

void readTail(char* cs, size_t i, size_t len, ParseResult* parse_res, SexpHeap* heap) {
    if (i == len) {
        construct_PR_error(parse_res, i);
        return;
    }
    if (cs[i] == ')') {
        construct_PR_success(
            parse_res, i,
            construct_nil(parse_res->success_Sexp)
        );
    }
    else if ('a' <= cs[i] && 'z' >= cs[i]) {
        readSymbol(cs, i, len, parse_res);
        Sexp* sym = allocate_Sexp(heap);
        switch (parse_res->kind) {
            case Success:
                copy_Sexp(sym, parse_res->success_Sexp);
                readThingAndTail(sym, cs, parse_res->position, len, parse_res, heap);
                break;
            case ErrorAt:
                break;
        }
    }
    else if (cs[i] == '(') {
        readTail(cs, i+1, len, parse_res, heap);
    }
}

void readThingAndTail(Sexp* thing, char* cs, size_t i, size_t len, ParseResult* parse_res, SexpHeap* heap) {
    Sexp* list;
    readTail(cs, i, len, parse_res, heap);
    switch (parse_res->kind) {
        case Success:
            list = allocate_Sexp(heap);
            copy_Sexp(list, parse_res->success_Sexp);
            construct_PR_success(
                parse_res, parse_res->position,
                construct_cons(parse_res->success_Sexp, thing, list)
            );
            break;
        case ErrorAt:
            break;
    }
}
