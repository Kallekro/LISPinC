#include "Sexp.h"

#define ENV_SIZE 100

typedef struct Binding Binding;
struct Binding {
    int valid;
    char name[MAX_SYMBOL_LENGTH];
    Sexp* value;
};

Binding globalEnv[ENV_SIZE];

void init_env(Binding env[]) {
    for (int i=0; i < ENV_SIZE; i++) {
        env[i].valid = 0;
    }
}

int lookup(Binding env[], char x[], Sexp** out_val) {
    for (int i=0; i < ENV_SIZE; i++) {
        if (env[i].valid
        && strcmp(env[i].name, x) == 0) {
            *out_val = env[i].value;
            return 0;
        }
    }
    return 1;
}

void update(Binding env[], char x[], Sexp* new_val) {
    int free_idx = -1;
    for (int i=0; i < ENV_SIZE; i++) {
        if (env[i].valid
        && strcmp(env[i].name, x) == 0) {
            env[i].value = new_val;
            return;
        } else if (!env[i].valid) {
            free_idx = i;
        }
    }
    if (free_idx != -1) {
        env[free_idx].valid = 1;
        strcpy(env[free_idx].name, x);
        env[free_idx].value = new_val;
    } else {
        printf("Environment is full. Consider increasing size.\n");
    }
}

#define KEYWORDS_C 6
char keywords[MAX_SYMBOL_LENGTH][KEYWORDS_C] = {
    "quote", "lambda", "define", "cons", "save", "load"
};

int evaluate(Sexp* s, char err_msg[]) {
    Binding localEnv[ENV_SIZE];
    init_env(localEnv);

    return 0;
}

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
                if (parse_res->position == 0) {
                    printf("= ");
                    char error_msg[100];
                    int ec = evaluate(parse_res->success_Sexp, error_msg);
                    switch (ec) {
                        case 0:
                            print_Sexp(parse_res->success_Sexp);
                            break;
                        default:
                            printf("! %s\n", error_msg);
                            break;
                    }
                } else {
                    printf("! input is not a single S-expression\n");
                }
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
    init_env(globalEnv);
    total_bytes_allocated = 0;
    total_garbage_collections = 0;
    clear_heap();
    ParseResult parse_res;
    Sexp* parse_s_exp = allocate_Sexp();
    construct_PR_empty(&parse_res, parse_s_exp);

    repl(&parse_res);
/*
    Sexp* s0 = allocate_Sexp();
    construct_symbol(s0, "abc");
    Sexp* s1 = allocate_Sexp();
    construct_symbol(s1, "cba");
    update(globalEnv, "x", s0);
    Sexp* out;
    if (lookup(globalEnv, "x", &out) == 0) {
        print_Sexp(out);
    } else {
        printf("not found\n");
    }
    update(globalEnv, "x", s1);
    if (lookup(globalEnv, "x", &out) == 0) {
        print_Sexp(out);
    } else {
        printf("not found\n");
    }
*/
    // Print footer
    printf("\n");
    for (int i=0; i < 40; i++) { printf("-"); }
    printf("\nS-expression size: %ld bytes\n", sizeof(Sexp));
    printf("\nTotal bytes allocated: %u\n", total_bytes_allocated);
    printf("Total bytes freed: %u\n", total_bytes_freed);
    printf("Total garbage collections: %u\n", total_garbage_collections);
    return 0;
}
