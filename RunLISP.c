#include "Sexp.h"

int main() {
    SexpHeap heap;
    initialize_heap(&heap);
    ParseResult parse_res;
    Sexp* parse_s_exp = allocate_Sexp(&heap);
    construct_PR_empty(&parse_res, parse_s_exp);

    char buffer[200];
    buffer[0] = '\0';
    readSexp("lol", &parse_res, &heap);
    showSexp(parse_res.success_Sexp, buffer);
    printf("%s\n", buffer);
    //printf("%d\n", parse_res.position);

    //construct_symbol(&sexps[1], "abe\0");
    //construct_cons(&sexps[4], &sexps[1], construct_nil(&sexps[3]));
    //construct_symbol(&sexps[0], "quote\0");
    //construct_cons(&sexps[2], &sexps[0], &sexps[4]);
//
    //char buffer[200];
    //buffer[0] = '\0';
//
    //showSexp(&sexps[2], buffer);
    //printf("%s", buffer);
    //printf("lo: %s%s", sexps[2].u.cons.Sexp1->u.symbol.name, sexps[2].u.cons.Sexp2->u.symbol.name);
    return 0;

}