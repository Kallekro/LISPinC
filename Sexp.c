#include "Sexp.h"

Sexp* construct_nil(Sexp* s) {
    if (s->memflag == 0) {
        printf("Constructing unallocated Nil S-expression.\n");
    }
    s->kind = Nil;
    return s;
}

Sexp* construct_symbol(Sexp* s, char name[MAX_SYMBOL_LENGTH]) {
    if (s->memflag == 0) {
        printf("Constructing unallocated Symbol S-expression.\n");
    }
    s->kind = Symbol;
    strcpy(s->u.symbol.name, name);
    return s;
}

Sexp* construct_cons(Sexp* s, Sexp* s1, Sexp* s2) {
    if (s->memflag == 0) {
        printf("Constructing unallocated Cons S-expression.\n");
    }
    s->kind = Cons;
    s->u.cons.Sexp1 = s1;
    s->u.cons.Sexp2 = s2;

    return s;
}

Sexp* copy_Sexp(Sexp* dest, Sexp* src) {
    dest->kind = src->kind;
    //dest->memflag = src->memflag;
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
            && s->u.cons.Sexp2->u.cons.Sexp2->kind == Nil)
            {
                strcat(result, "\'");
                showSexp(s->u.cons.Sexp2->u.cons.Sexp1, result);
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

void mark(Sexp* node) {
    if (node->memflag == 0) { return; }
    node->memflag = 2;
    if (node->kind == Cons) {
        mark(node->u.cons.Sexp1);
        mark(node->u.cons.Sexp2);
    }
}

int mark_and_sweep(RootSet* rootSet) {
    for (int i=0; i < HEAP_SIZE; i++) {
        if (Heap[i].memflag == 2) {
            Heap[i].memflag = 1;
        }
        //else if (Heap[i].memflag != 1 && Heap[i].memflag != 0) {
        //    Heap[i].memflag = 0;
        //}
    }
    int objects_freed = 0;
    for (int i=0; i < rootSet->length; i++) {
        mark(rootSet->set[i]);
    }
    for (int i=0; i < HEAP_SIZE; i++) {
        if (Heap[i].memflag == 2) {
            Heap[i].memflag = 1;
        }
        else if (Heap[i].memflag == 1) {
            Heap[i].memflag = 0;
            total_bytes_freed += sizeof(Sexp);
            objects_freed++;
        }
    }
    total_garbage_collections++;
    printf("garbage collection triggered\n");
    printf("objects freed: %d\n", objects_freed);
    return objects_freed;
}

void clear_heap() {
    for (int i=0; i < HEAP_SIZE; i++) {
        Heap[i].memflag = 0;
    }
}

void construct_rootSet(RootSet* rootSet) {
    rootSet->set[0] = 0;
    rootSet->length = 0;
}

Sexp* _allocate(RootSet* rootSet) {
    for (int i=0; i < HEAP_SIZE; i++) {
        if (Heap[i].memflag == 0) {
            total_bytes_allocated += sizeof(Sexp);
            (&Heap[i])->memflag = 1;
            rootSet->set[rootSet->length] = &Heap[i];
            rootSet->length++;
            return &Heap[i];
        }
    }
    return 0;
}

Sexp* allocate_Sexp(RootSet* rootSet) {
    Sexp* new_alloc = _allocate(rootSet);
    if (new_alloc != 0) { return new_alloc; }
    if (mark_and_sweep(rootSet) < 1) {
        printf("Fatal error: out of memory. Consider increasing heap size (HEAP_SIZE).\n");
        exit(1);
    }
    return _allocate(rootSet);
}


// TODO:
/*
int allocate_n_Sexp(Sexp* rootSet[], int n, Sexp* output[]) {
    for (int i=0; i < n; i++) {
        output[i] = _allocate(rootSet);
        if (output[i] == 0) {
            break;
        }
    }
    if (mark_and_sweep(rootSet))
}*/

void appendSet(Sexp* set[], Sexp* s) {
    int i=0;
    while (set[i++] != 0) {
        if (i >= ENV_SIZE) {
            printf("Fatal error: stack overflow. Consider increasing environment size (ENV_SIZE).");
            exit(1);
        }
    }
    set[i] = s;
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
    s->memflag = 1;
    p->success_Sexp = construct_nil(s);
}

void readSexp (char* cs, ParseResult* parse_res, RootSet* rootSet) {
    size_t len = strlen(cs);
    size_t j = 0;

    readExp(cs, 0, len, parse_res, rootSet);
    switch (parse_res->kind) {
        case Success:
            j = parse_res->position;
            // skip whitespace
            while (j < len && (cs[j] == ' ' || cs[j] == '\n')) {
                j++;
            }
            construct_PR_success(parse_res, (j == len) ? 0 : j,
                                 parse_res->success_Sexp);
            break;
        case ErrorAt:
            construct_PR_error(parse_res, parse_res->position);
            break;
        case Empty:
            break;
    }
}

void readExp(char* cs, size_t i, size_t len, ParseResult* parse_res, RootSet* rootSet) {
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
        readTail(cs, i+1, len, parse_res, rootSet);
    }
    else if (cs[i] == '\'') {
        readExp(cs, i+1, len, parse_res, rootSet);
        Sexp* s0; Sexp* s1; Sexp* s2; Sexp* s3;
        switch (parse_res->kind) {
            case Success:
                s0 = allocate_Sexp(rootSet); s1 = allocate_Sexp(rootSet);
                s2 = allocate_Sexp(rootSet); s3 = allocate_Sexp(rootSet);
                copy_Sexp(s0, parse_res->success_Sexp);
                construct_PR_success(
                    parse_res, parse_res->position,
                    construct_cons(
                        parse_res->success_Sexp,
                        construct_symbol(s1, "quote"),
                        construct_cons(s2, s0, construct_nil(s3)))
                );
                break;
            case ErrorAt:
                construct_PR_error(parse_res, parse_res->position);
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
    name[name_idx+1] = '\0';
    construct_PR_success(
        parse_res, i,
        construct_symbol(parse_res->success_Sexp, name)
    );
}

void readTail(char* cs, size_t i, size_t len, ParseResult* parse_res, RootSet* rootSet) {
    while (i < len && (cs[i] == ' ' || cs[i] == '\n')) {
        i++;
    }
    if (i == len) {
        construct_PR_error(parse_res, i);
        return;
    }
    Sexp* s0;
    if (cs[i] == ')') {
        construct_PR_success(
            parse_res, i+1,
            construct_nil(parse_res->success_Sexp)
        );
    }
    else if ('a' <= cs[i] && 'z' >= cs[i]) {
        readSymbol(cs, i, len, parse_res);
        switch (parse_res->kind) {
            case Success:
                s0 = allocate_Sexp(rootSet);
                copy_Sexp(s0, parse_res->success_Sexp);
                readSexpAndTail(s0, cs, parse_res->position, len, parse_res, rootSet);
                break;
            case ErrorAt:
                construct_PR_error(parse_res, parse_res->position);
                break;
        }
    }
    else if (cs[i] == '(') {
        readTail(cs, i+1, len, parse_res, rootSet);
        switch (parse_res->kind) {
            case Success:
                s0 = allocate_Sexp(rootSet);
                copy_Sexp(s0, parse_res->success_Sexp);
                readSexpAndTail(s0, cs, parse_res->position, len, parse_res, rootSet);
                break;
            case ErrorAt:
                construct_PR_error(parse_res, parse_res->position);
                break;
        }
    }
    else if (cs[i] == '\'') {
        readExp(cs, i+1, len, parse_res, rootSet);
        switch (parse_res->kind) {
            case Success:
                s0 = allocate_Sexp(rootSet);
                copy_Sexp(s0, parse_res->success_Sexp);
                readQuoteSexpAndTail(s0, cs, parse_res->position, len, parse_res, rootSet);
                break;
            case ErrorAt:
                construct_PR_error(parse_res, parse_res->position);
                break;
        }
    }
    else if (cs[i] == '.') {
        readExp(cs, i+1, len, parse_res, rootSet);
        Sexp* s0;
        switch (parse_res->kind) {
            case Success:
                s0 = allocate_Sexp(rootSet);
                copy_Sexp(s0, parse_res->success_Sexp);
                readClose(cs, parse_res->position, len, parse_res, rootSet);
                switch (parse_res->kind) {
                    case Success:
                        construct_PR_success(parse_res, parse_res->position, s0);
                        break;
                    case ErrorAt:
                        construct_PR_error(parse_res, parse_res->position);
                        break;
                }
                break;
            case ErrorAt:
                construct_PR_error(parse_res, parse_res->position);
                break;
        }
    }
}

void readSexpAndTail(Sexp* s, char* cs, size_t i, size_t len, ParseResult* parse_res, RootSet* rootSet) {
    Sexp* list;
    readTail(cs, i, len, parse_res, rootSet);
    switch (parse_res->kind) {
        case Success:
            list = allocate_Sexp(rootSet);
            copy_Sexp(list, parse_res->success_Sexp);
            construct_PR_success(
                parse_res, parse_res->position,
                construct_cons(parse_res->success_Sexp, s, list)
            );
            break;
        case ErrorAt:
            construct_PR_error(parse_res, parse_res->position);
            break;
    }
}

void readQuoteSexpAndTail(Sexp* quote_s, char* cs, size_t i, size_t len, ParseResult* parse_res, RootSet* rootSet) {
    readTail(cs, i, len, parse_res, rootSet);
    Sexp* list;
    Sexp* s0; Sexp* s1; Sexp* s2; Sexp* s3; Sexp* s4;
    switch (parse_res->kind) {
        case Success:
            list = allocate_Sexp(rootSet);
            s0 = allocate_Sexp(rootSet); s1 = allocate_Sexp(rootSet);
            s2 = allocate_Sexp(rootSet); s3 = allocate_Sexp(rootSet);
            copy_Sexp(list, parse_res->success_Sexp);
            construct_PR_success(
                parse_res, parse_res->position,
                construct_cons(
                    parse_res->success_Sexp,
                    construct_cons(
                        s0,
                        construct_symbol(s1, "quote"),
                        construct_cons(s2, quote_s, construct_nil(s3))
                    ),
                    list
                )
            );
            break;
        case ErrorAt:
            construct_PR_error(parse_res, parse_res->position);
            break;
    }
}

void readClose(char* cs, size_t i, size_t len, ParseResult* parse_res, RootSet* rootSet) {
    while (i < len && (cs[i] == ' ' || cs[i] == '\n')) {
        i++;
    }
    if (i == len) {
        construct_PR_error(parse_res, i);
        return;
    }
    Sexp* s0 = allocate_Sexp(rootSet);
    construct_PR_success(parse_res, i+1, s0);
}

void print_Sexp(Sexp* s) {
    char buffer[200];
    buffer[0] = '\0';
    showSexp(s, buffer);
    printf("%s\n", buffer);
}