#include "Sexp.h"
#include "RunLISP.h"

int main() {
    init_env(globalEnv);
    total_bytes_allocated = 0;
    total_garbage_collections = 0;
    clear_heap();
    ParseResult parse_res;
    Sexp* parse_s_exp = allocate_Sexp();
    construct_PR_empty(&parse_res, parse_s_exp);

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
