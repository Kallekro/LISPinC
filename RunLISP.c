#include "Sexp.h"

void repl(ParseResult* parse_res) {
    char input_buffer[200];
    char carry[200];
    char str[400];
    carry[0] = '\0';
    while (1) {
        if (feof(stdin)) { break; }
        input_buffer[0] = '\0';
        if (carry[0] == '\0') {
            printf("> ");
        }
        fgets(input_buffer, 200, stdin);
        strcpy(str, carry);
        strcat(str, input_buffer);
        carry[0] = '\0';
        size_t len = strlen(str);
        readSexp(str, parse_res);
        switch (parse_res->kind) {
            case Success:
                printf("= ");
                print_Sexp(parse_res->success_Sexp);
                break;
            case ErrorAt:
                if (parse_res->position == len) {
                    strcpy(carry, str);
                } else {
                    printf("! parse error at position %d\n", parse_res->position);
                }
                break;
        }
    }
}

int main() {
    total_bytes_allocated = 0;
    total_garbage_collections = 0;
    clear_heap();
    ParseResult parse_res;
    Sexp* parse_s_exp = allocate_Sexp();
    construct_PR_empty(&parse_res, parse_s_exp);

    repl(&parse_res);

    // Print footer
    printf("\n");
    for (int i=0; i < 40; i++) { printf("-"); }
    printf("\nS-expression size: %ld bytes\n", sizeof(Sexp));
    printf("\nTotal bytes allocated: %u\n", total_bytes_allocated);
    printf("Total bytes freed: %u\n", total_bytes_freed);
    printf("Total garbage collections: %u\n", total_garbage_collections);
    return 0;
}
