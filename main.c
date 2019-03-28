// TODO:
//      * update should throw error like others
//      * reduce amount of allocations of s_out
//      * don't get root set if no garbage collection required
//      -> Maybe pass pointer to root set getter function
//      * numbers does not result in parse error
//      * non error-code int -> size_t ?
//      * TODO's
//      DONE   * endless loop instead of of out of memory error
//      DONE   * save/load
//      DONE   ** !!need to consider local environment when parsing also!!
//      DONE   * why does append not work?
//      DONE   * dynamically growing heap
//      DONE   * garbage collection
//      DONE   !!!* constructing unallocated should never happen as error should be thrown earlier!!!
#include "Sexp.h"
#include "RunLISP.h"

int main() {
    printf("PLD LISP v. 1.0 - Ported to C\n");
    repl(); // exit with EOF (CTRL-D)

    // Print footer
    printf("\n");
    for (int i=0; i < 40; i++) { printf("-"); }
    printf("\nS-expression size: %ld bytes\n", sizeof(Sexp));
    printf("Heap block size: %u objects\n", HEAP_BLOCK_SIZE);
    printf("Heap blocks allocated: %lu\n", total_heapblocks_allocated);
    printf("Total heap size: %ld bytes\n", sizeof(Sexp) * HeapSize);
    printf("\nTotal bytes allocated on heap: %lu\n", total_bytes_allocated);
    printf("Total bytes freed from heap: %lu\n", total_bytes_freed);
    printf("Total garbage collections: %lu\n", total_garbage_collections);
    return 0;
}
