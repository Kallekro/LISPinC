#include "Sexp.h"

Sexp* construct_nil(Sexp* s) {
    if (s->memflag == 0) {
        printf("Constructing unallocated Nil S-expression.\n");
    }
    s->kind = Nil;
    return s;
}

Sexp* construct_symbol(Sexp* s, char* name) {
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

void init_heapblock(HeapBlock* heapblock) {
    heapblock->nextBlock = 0;
    heapblock->objects = malloc(HEAP_BLOCK_SIZE * sizeof(Sexp*));
    for (size_t i=0; i < HEAP_BLOCK_SIZE; i++) {
        heapblock->objects[i] = malloc(sizeof(Sexp));
        heapblock->objects[i]->memflag = 1;
        construct_nil(heapblock->objects[i]);
        heapblock->objects[i]->memflag = 0;
    }
}

void free_heapblock(HeapBlock* heapblock) {
    for (size_t i=0; i < HEAP_BLOCK_SIZE; i++) {
        if (heapblock->objects[i]->memflag != 0) {
            total_bytes_freed += sizeof(Sexp);
        }
        free(heapblock->objects[i]);
    }
    free(heapblock->objects);
    if (heapblock->nextBlock != 0) {
        free_heapblock(heapblock->nextBlock);
        free(heapblock->nextBlock);
    }
}

void create_heap() {
    HeapSize = HEAP_BLOCK_SIZE;
    init_heapblock(&Heap);
}

void delete_heap() {
    free_heapblock(&Heap);
}

HeapBlock* allocate_heap_block() {
    HeapBlock* current_block = &Heap;
    while (current_block->nextBlock != 0) { current_block = current_block->nextBlock; }
    current_block->nextBlock = malloc(sizeof(HeapBlock));
    init_heapblock(current_block->nextBlock);
    HeapSize += HEAP_BLOCK_SIZE;
    total_heapblocks_allocated++;
    return current_block->nextBlock;
}

void mark(Sexp* node) {
    if (node == 0 || node->memflag == 0 || node->memflag == 2) { return; }
    node->memflag = 2;
    if (node->kind == Cons) {
        mark(node->u.cons.Sexp1);
        mark(node->u.cons.Sexp2);
    }
}

int mark_and_sweep(RootSet* rootSet) {
    HeapBlock* current_block = &Heap;
    int objects_freed = 0;
    // mark
    for (size_t i=0; i < rootSet->length; i++) {
        mark(rootSet->set[i]);
    }
    // sweep
    while (current_block != 0) {
        for (size_t i=0; i < HEAP_BLOCK_SIZE; i++) {
            if (current_block->objects[i]->memflag == 1) {
                current_block->objects[i]->memflag = 0;
                bytes_freed_GC += sizeof(Sexp);
                objects_freed++;
            }
            else if (current_block->objects[i]->memflag == 2) {
                current_block->objects[i]->memflag = 1;
            }
        }
        current_block = current_block->nextBlock;
    }
    total_garbage_collections++;
    return objects_freed;
}

Sexp* _allocate(RootSet* rootSet, HeapBlock* start_block) {
    HeapBlock* current_block = start_block;
    while (current_block != 0) {
        for (size_t i=0; i < HEAP_BLOCK_SIZE; i++) {
            if (current_block->objects[i]->memflag == 0) {
                total_bytes_allocated += sizeof(Sexp);
                current_block->objects[i]->memflag = 1;
                rootSet->set[rootSet->length] = current_block->objects[i];
                rootSet->length++;
                return current_block->objects[i];
            }
        }
        current_block = current_block->nextBlock;
    }
    return 0;
}

Sexp* allocate_Sexp(RootSet* rootSet) {
    Sexp* new_alloc = _allocate(rootSet, &Heap);
    if (new_alloc != 0) { return new_alloc; }
    if (mark_and_sweep(rootSet) < 1) {
        HeapBlock* new_block = allocate_heap_block();
        _allocate(rootSet, new_block);
    }
    return _allocate(rootSet, &Heap);
}

void construct_rootSet(RootSet* rootSet) {
    rootSet->set[0] = 0;
    rootSet->length = 0;
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
    Sexp* s_res;
    readExp(cs, 0, len, parse_res, rootSet);
    switch (parse_res->kind) {
        case Success:
            j = parse_res->position;
            // skip whitespace
            while (j < len && (cs[j] == ' ' || cs[j] == '\n')) {
                j++;
            }
            s_res = allocate_Sexp(rootSet);
            copy_Sexp(s_res, parse_res->success_Sexp);
            construct_PR_success(parse_res, (j == len) ? 0 : j,
                                 s_res);
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
        readSymbol(cs, i, len, parse_res, rootSet);
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

void readSymbol(char* cs, size_t i, size_t len, ParseResult* parse_res, RootSet* rootSet) {
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
    parse_res->success_Sexp = allocate_Sexp(rootSet);
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
        parse_res->success_Sexp = allocate_Sexp(rootSet);
        construct_PR_success(
            parse_res, i+1,
            construct_nil(parse_res->success_Sexp)
        );
    }
    else if ('a' <= cs[i] && 'z' >= cs[i]) {
        readSymbol(cs, i, len, parse_res, rootSet);
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

// debug
void print_Sexp(Sexp* s) {
    char buffer[200];
    buffer[0] = '\0';
    showSexp(s, buffer);
    printf("%s\n", buffer);
}

void print_rootSet(RootSet* rootSet) {
    for (size_t i=0; i < rootSet->length; i++) {
        printf("%lu = ", i);
        print_Sexp(rootSet->set[i]);
    }
}