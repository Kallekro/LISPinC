// TODO:
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

int evaluate(Sexp* s_in, Sexp** s_out, Binding localEnv[], char err_msg[]);

int evaluate_cons(Sexp* s_in, Sexp** s_out, Binding localEnv[], char err_msg[]) {
    Sexp* s1 = s_in->u.cons.Sexp1; Sexp* s2 = s_in->u.cons.Sexp2;
    char* charbuf;
    if (s1->kind == Symbol) {
        if (strcmp(s1->u.symbol.name, "quote") == 0
        && s2->kind == Cons
        && s2->u.cons.Sexp2->kind == Nil) {
            *s_out = s2->u.cons.Sexp1;
            return 0;
        }
        else if (strcmp(s1->u.symbol.name, "lambda") == 0) {
            *s_out = allocate_Sexp();
            copy_Sexp(*s_out, s_in);
            return 0;
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
            if (evaluate(s2->u.cons.Sexp2->u.cons.Sexp1, s_out, localEnv, err_msg) != 0) {
                return 1;
            }
            update(globalEnv, charbuf, *s_out);
            *s_out = allocate_Sexp();
            construct_nil(*s_out);

        }
        else if (strcmp(s1->u.symbol.name, "cons") == 0
        && s2->kind == Cons
        && s2->u.cons.Sexp2->kind == Cons
        && s2->u.cons.Sexp2->u.cons.Sexp2->kind == Nil) {
            Sexp* e1 = allocate_Sexp(); Sexp* e2 = allocate_Sexp();
            if (evaluate(s2->u.cons.Sexp1, &e1, localEnv, err_msg) != 0) {
                return 1;
            }
            if (evaluate(s2->u.cons.Sexp2->u.cons.Sexp1, &e2, localEnv, err_msg) != 0) {
                return 1;
            }
            *s_out = allocate_Sexp();
            construct_cons(*s_out, e1, e2);
        }
        else if (strcmp(s1->u.symbol.name, "save") == 0) {

        }
        else if (strcmp(s1->u.symbol.name, "load") == 0) {

        }
    }
    return 0;
}

// Might corrupt s_in to avoid extra allocation
int evaluate(Sexp* s_in, Sexp** s_out, Binding localEnv[], char err_msg[]) {
    switch (s_in->kind) {
        case Symbol:
            if (iskeyword(s_in->u.symbol.name)) {
                sprintf(err_msg,
                    "keyword %s can not be used as variable", s_in->u.symbol.name);
                return 1;
            } else {
                if (lookup_and_global(localEnv, s_in->u.symbol.name, s_out) != 0) {
                    sprintf(err_msg,
                        "undefined variable %s", s_in->u.symbol.name);
                    return 1;
                }
            }
            break;
        case Cons:
            return evaluate_cons(s_in, s_out, localEnv, err_msg);
            break;
        case Nil:
            break;
    }
    return 0;
}

void repl(ParseResult* parse_res) {
    char input_buffer[200];
    char carry[200];
    carry[0] = '\0';
    char str[400];
    Sexp* out_s;
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
                    int ec = evaluate(parse_res->success_Sexp, &out_s, localEnv, error_msg);
                    switch (ec) {
                        case 0:
                            printf("= ");
                            print_Sexp(out_s);
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

    printf("PLD LISP v. 1.0 - Ported to C\n");
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
    printf("Total heap size: %ld bytes\n", sizeof(Heap));
    printf("\nTotal bytes allocated: %u\n", total_bytes_allocated);
    printf("Total bytes freed: %u\n", total_bytes_freed);
    printf("Total garbage collections: %u\n", total_garbage_collections);
    return 0;
}
