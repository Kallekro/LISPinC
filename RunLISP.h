#ifndef RUNLISP_H
#define RUNLISP_H
#include "Sexp.h"

typedef struct Binding Binding;
struct Binding {
    int valid;
    char name[MAX_SYMBOL_LENGTH];
    Sexp* value;
};
Binding globalEnv[ENV_SIZE];

#define KEYWORDS_C 6
const char keywords[KEYWORDS_C][MAX_SYMBOL_LENGTH];

void init_env(Binding env[]);
int lookup(Binding env[], char x[], Sexp** out_val);
int lookup_and_global(Binding env[], char x[], Sexp** out_val);
void update(Binding env[], char x[], Sexp* new_val);
int appendEnv(Binding env1[], Binding env2[], char err_msg[]);
int iskeyword(char x[]);
int evaluate(Sexp* s_in, Sexp** s_out, Binding localEnv[], char err_msg[]);
int evaluate_cons(Sexp* s_in, Sexp** s_out, Binding localEnv[], char err_msg[]);
int evalList(Sexp* es, Sexp** s_out, Binding localEnv[], char err_msg[]);
int tryRules(Sexp* rs, Sexp** s_out, Sexp* args, Binding localEnv[], char err_msg[]);
int disjoint(Binding env1[], Binding env2[], char err_msg[]);
int matchPattern(Sexp* p, Sexp* v, Binding env[], char err_msg[]);
int saveGlobalEnvironment(char* fname, char err_msg[]);
int loadGlobalEnvironment(char* fname, char err_msg[]);
void quoteExp(Sexp* v, char output[]);
void repl();

// debug
void print_Env(Binding env[]);
#endif