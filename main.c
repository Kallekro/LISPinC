// TODO:
//      * garbage collection
//      DONE   ** !!need to consider local environment when parsing also!!
//      !!!* constructing unallocated should never happen as error should be thrown earlier!!!
//      * endless loop instead of of out of memory error
//      * save/load
//      * update should throw error like others
//      * reduce amount of allocations of s_out
//      * why does append not work?
//      * don't get root set if no garbage collection required
//      -> Maybe pass pointer to root set getter function
//      * numbers does not result in parse error
//      * non error-code int -> size_t ?
//      * TODO's

#include "Sexp.h"
#include "RunLISP.h"

int main() {
    clear_heap();
    init_env(globalEnv);
    total_bytes_allocated = 0;
    total_garbage_collections = 0;
    ParseResult parse_res;
    Sexp parse_s_exp;
    construct_PR_empty(&parse_res, &parse_s_exp);

    printf("PLD LISP v. 1.0 - Ported to C\n");
    repl(&parse_res);

    // Print footer
    printf("\n");
    for (int i=0; i < 40; i++) { printf("-"); }
    printf("\nS-expression size: %ld bytes\n", sizeof(Sexp));
    printf("Total heap size: %ld bytes\n", sizeof(Heap));
    printf("\nTotal bytes allocated: %u\n", total_bytes_allocated);
    printf("Total bytes freed: %u\n", total_bytes_freed);
    printf("Total garbage collections: %u\n", total_garbage_collections);
    return 0;
}
