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
    printf("Total bytes freed from heap: %lu\n", total_bytes_freed + bytes_freed_GC);
    printf("Bytes freed during garbage collection: %lu\n", bytes_freed_GC);
    printf("Total garbage collections: %lu\n", total_garbage_collections);
    return 0;
}
