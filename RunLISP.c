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

int lookup_and_global(Binding env[], char x[], Sexp** out_val) {
    if (lookup(globalEnv, x, out_val) == 0) {
        return 0;
    }
    return lookup(env, x, out_val);
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
char keywords[KEYWORDS_C][MAX_SYMBOL_LENGTH] = {
    "quote", "lambda", "define", "cons", "save", "load"
};
int iskeyword(char x[]) {
    for (int i=0; i < KEYWORDS_C; i++) {
        if (strcmp(keywords[i], x) == 0) {
            return 1;
        }
    }
    return 0;
}

int evaluate(Sexp* s, Sexp* out_s, Binding localEnv[], char err_msg[]);

int evaluate_cons(Sexp* s, Sexp* out_s, Binding localEnv[], char err_msg[]) {
    Sexp* s1 = s->u.cons.Sexp1; Sexp* s2 = s->u.cons.Sexp2;
    char* charbuf;
    switch (s1->kind) {
        case Symbol:
            if (strcmp(s1->u.symbol.name, "quote") == 0
            && s2->kind == Cons
            && s2->u.cons.Sexp2->kind == Nil) {
                copy_Sexp(out_s, s2->u.cons.Sexp1);
            }
            else if (strcmp(s1->u.symbol.name, "lambda") == 0) {
                copy_Sexp(out_s, s);
            }
            else if (strcmp(s1->u.symbol.name, "define") == 0
            && s2->kind == Cons
            && s2->u.cons.Sexp1->kind == Symbol
            && s2->u.cons.Sexp2->kind == Cons
            && s2->u.cons.Sexp2->u.cons.Sexp2->kind == Nil) {
                charbuf = s2->u.cons.Sexp1->u.symbol.name;
                if (iskeyword(charbuf)) {
                    sprintf(err_msg,
                        "keyword %s can not be redefined", charbuf);
                    return 1;
                }
                if (evaluate(s2->u.cons.Sexp2->u.cons.Sexp1,
                             s2->u.cons.Sexp2->u.cons.Sexp1, localEnv, err_msg) != 0) {
                    return 1;
                }
                update(globalEnv, charbuf, s2->u.cons.Sexp2->u.cons.Sexp1);
                construct_nil(s);
            }
            else if (strcmp(s1->u.symbol.name, "cons") == 0) {
                // Todo
            }
            break;
        default:
            // function application?
            break;
    }
    return 0;
}

int evaluate(Sexp* s, Sexp* out_s, Binding localEnv[], char err_msg[]) {
    switch (s->kind) {
        case Symbol:
            if (iskeyword(s->u.symbol.name)) {
                sprintf(err_msg,
                    "keyword %s can not be used as variable", s->u.symbol.name);
                return 1;
            } else {
                if (lookup_and_global(localEnv, s->u.symbol.name, &out_s) != 0) {
                    sprintf(err_msg,
                        "undefined variable %s", s->u.symbol.name);
                    return 1;
                }
            }
            break;
        case Cons:
            return evaluate_cons(s, out_s, localEnv, err_msg);
            break;
        case Nil:
            break;
    }
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
                    char error_msg[100];
                    Binding localEnv[ENV_SIZE];
                    init_env(localEnv);
                    Sexp out_s;
                    out_s.allocated = 1;
                    construct_nil(&out_s);
                    int ec = evaluate(parse_res->success_Sexp, &out_s, localEnv, error_msg);
                    switch (ec) {
                        case 0:
                            printf("= ");
                            print_Sexp(&out_s);
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
