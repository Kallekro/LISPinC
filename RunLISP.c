#include "RunLISP.h"

const char keywords[KEYWORDS_C][MAX_SYMBOL_LENGTH] = {
    "quote", "lambda", "define", "cons", "save", "load"
};

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
        } else if (!env[i].valid && free_idx == -1) {
            free_idx = i;
        }
    }
    if (free_idx != -1) {
        env[free_idx].valid = 1;
        strcpy(env[free_idx].name, x);
        env[free_idx].value = new_val;
    } else {
        printf("stack overflow. Consider increasing environment size (ENV_SIZE).\n");
    }
}

int appendEnv(Binding env1[], Binding env2[], char err_msg[]) {
    int free_idx;
    for (free_idx=0; free_idx < ENV_SIZE; free_idx++) {
        if (!env1[free_idx].valid) { break; }
    }
    for (int i=0; i < ENV_SIZE; i++) {
        if (!env2[i].valid) { break; }
        if (free_idx >= ENV_SIZE) {
            sprintf(err_msg, "stack overflow. Consider increasing environment size (ENV_SIZE).");
            return 1;
        }
        env1[free_idx++] = env2[i];
    }
    return 0;
}

int get_root_set(Binding env1[], Binding env2[], RootSet* rootSet) {
    int i;
    for (i=0; i < ENV_SIZE; i++) {
        if (!env1[i].valid) { break; }
        rootSet->set[i] = env1[i].value;
    }
    if (env2 != 0) {
        for (int j = 0; j < ENV_SIZE; j++) {
            if (!env2[j].valid) { break; }
            rootSet->set[i++] = env1[j].value;
        }
    }
    rootSet->length = i;
    rootSet->set[i] = 0;
}

Sexp* safe_allocate(Binding localEnv[]) {
    RootSet tmp_root_set;
    construct_rootSet(&tmp_root_set);
    get_root_set(globalEnv, localEnv, &tmp_root_set);
    return allocate_Sexp(&tmp_root_set);
}

int iskeyword(char x[]) {
    for (int i=0; i < KEYWORDS_C; i++) {
        if (strcmp(keywords[i], x) == 0) {
            return 1;
        }
    }
    return 0;
}

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
            *s_out = safe_allocate(localEnv);
            construct_nil(*s_out);
            break;
    }
    return 0;
}

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
            *s_out = safe_allocate(localEnv);
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
                sprintf(err_msg, "keyword %s can not be redefined", charbuf);
                return 1;
            }
            Sexp* e1 = safe_allocate(localEnv);
            if (evaluate(s2->u.cons.Sexp2->u.cons.Sexp1, &e1, localEnv, err_msg) != 0) {
                return 1;
            }
            update(globalEnv, charbuf, e1);
            *s_out = safe_allocate(localEnv);
            construct_nil(*s_out);
            return 0;
        }
        else if (strcmp(s1->u.symbol.name, "cons") == 0
        && s2->kind == Cons
        && s2->u.cons.Sexp2->kind == Cons
        && s2->u.cons.Sexp2->u.cons.Sexp2->kind == Nil) {
            Sexp* e1 = safe_allocate(localEnv); Sexp* e2 = safe_allocate(localEnv);
            if (evaluate(s2->u.cons.Sexp1, &e1, localEnv, err_msg) != 0) { return 1; }
            if (evaluate(s2->u.cons.Sexp2->u.cons.Sexp1, &e2, localEnv, err_msg) != 0) { return 1; }
            *s_out = safe_allocate(localEnv);
            construct_cons(*s_out, e1, e2);
            return 0;
        }
        else if (strcmp(s1->u.symbol.name, "save") == 0) {
            // lower carbon emissions
            saveGlobalEnvironment();
            return 0;
        }
        else if (strcmp(s1->u.symbol.name, "load") == 0
        && s2->kind == Cons
        && s2->u.cons.Sexp1->kind == Symbol
        && s2->u.cons.Sexp2->kind == Nil) {
            if (loadGlobalEnvironment(s2->u.cons.Sexp1->u.symbol.name, err_msg) != 0) { return 1; }
            *s_out = safe_allocate(localEnv);
            construct_nil(*s_out);
            return 0;
        }
    }
    // function application
    Sexp* e1 = safe_allocate(localEnv);
    if (evaluate(s1, &e1, localEnv, err_msg) != 0) { return 1; }
    if (e1->kind == Cons
    && e1->u.cons.Sexp1->kind == Symbol
    && strcmp(e1->u.cons.Sexp1->u.symbol.name, "lambda") == 0) {
        Sexp* args = safe_allocate(localEnv);
        if (evalList(s2, &args, localEnv, err_msg) != 0) { return 1; }
        *s_out = safe_allocate(localEnv);
        if (tryRules(e1->u.cons.Sexp2, s_out, args, localEnv, err_msg) != 0) { return 1; }
        return 0;
    }
    sprintf(err_msg, "no matches");
    return 1;
}

int evalList(Sexp* es, Sexp** s_out, Binding localEnv[], char err_msg[]) {
    Sexp* e1; Sexp* e2; char msgbuf[200];
    switch (es->kind) {
        case Cons:
            e1 = safe_allocate(localEnv); e2 = safe_allocate(localEnv);
            if (evaluate(es->u.cons.Sexp1, &e1, localEnv, err_msg) != 0) { return 1; }
            if (evalList(es->u.cons.Sexp2, &e2, localEnv, err_msg) != 0) { return 1; }
            *s_out = safe_allocate(localEnv);
            construct_cons(*s_out, e1, e2);
            break;
        case Nil:
            *s_out = safe_allocate(localEnv);
            construct_nil(*s_out);
            break;
        default:
            showSexp(es, msgbuf);
            sprintf(err_msg, "arguments are not a list: %s", msgbuf);
            return 1;
    }
    return 0;
}

int tryRules(Sexp* rs, Sexp** s_out, Sexp* args, Binding localEnv[], char err_msg[]) {
    char msgbuf[200];
    switch (rs->kind) {
        case Cons:
            if (rs->u.cons.Sexp2->kind == Cons) {
                Binding outputEnv[ENV_SIZE];
                init_env(outputEnv);
                int ec = matchPattern(rs->u.cons.Sexp1, args, outputEnv, err_msg);
                if (ec == 0) {
                    return tryRules(rs->u.cons.Sexp2->u.cons.Sexp2, s_out, args, localEnv, err_msg);
                }
                else if (ec == -1) {return 1; }

                if (appendEnv(localEnv, outputEnv, err_msg) != 0) { return 1; }
                *s_out = safe_allocate(localEnv);
                evaluate(rs->u.cons.Sexp2->u.cons.Sexp1, s_out, localEnv, err_msg);
                return 0;
            }
            break;
        case Nil:
            showSexp(args, msgbuf);
            sprintf(err_msg, "no patterns matched arguments: %s", msgbuf);
            return 1;
    }
    showSexp(rs, msgbuf);
    sprintf(err_msg, "malformed rules: %s", msgbuf);
    return 1;
}


int disjoint(Binding env1[], Binding env2[], char err_msg[]) {
    for (int i=0; i < ENV_SIZE; i++) {
        if (!env1[i].valid) { break; }
        Sexp* lookup_res;
        if (lookup(env2, env1[i].name, &lookup_res) == 0) {
            sprintf(err_msg, "repeated variable %s in pattern", env1[i].name);
            return 1;
        }
    }
    return 0;
}

// returns 0=false, 1=true, -1=error
int matchPattern(Sexp* p, Sexp* v, Binding env[], char err_msg[]) {
    if (p->kind == Nil && v->kind == Nil) {
        return 1;
    }
    if (p->kind == Symbol) {
        if (iskeyword(p->u.symbol.name)) {
            sprintf(err_msg, "keyword %s can not be used in pattern", p->u.symbol.name);
            return -1;
        }
        update(env, p->u.symbol.name, v);
        return 1;
    }
    if (p->kind == Cons && v->kind == Cons) {
        Binding env2[ENV_SIZE]; init_env(env2);
        int ec1 = matchPattern(p->u.cons.Sexp1, v->u.cons.Sexp1, env, err_msg);
        if (ec1 == -1) { return -1; }
        int ec2 = matchPattern(p->u.cons.Sexp2, v->u.cons.Sexp2, env2, err_msg);
        if (ec2 == -1) { return -1; }
        if (ec1 == 1 && ec2 == 1) {
            if (disjoint(env, env2, err_msg) != 0) {
                return -1;
            }
            if (appendEnv(env, env2, err_msg) != 0) {
                return -1;
            }
            return 1;
        }
    }
    return 0;
}

void saveGlobalEnvironment() {

}

int loadGlobalEnvironment(char* fname, char err_msg[]) {
    FILE* fp;
    char* line = 0;
    size_t len = 0;
    size_t read;
    char path[MAX_FILE_NAME];
    sprintf(path, "%s.le", fname);
    fp = fopen(path, "r");
    if (fp == 0) {
        sprintf(err_msg, "could not open file %s", path);
        return 1;
    }
    ParseResult parse_res;
    Sexp parse_s_exp;
    construct_PR_empty(&parse_res, &parse_s_exp);
    RootSet rootSet;
    construct_rootSet(&rootSet);
    Sexp* out_s;
    Binding localEnv[ENV_SIZE];
    while ((read = getline(&line, &len, fp)) != -1) {
        if (read == 0) { continue; }
        get_root_set(globalEnv, 0, &rootSet);
        readSexp(line, &parse_res, &rootSet);
        switch (parse_res.kind) {
            case Success:
                init_env(localEnv);
                if (evaluate(parse_res.success_Sexp, &out_s, localEnv, err_msg) != 0) { return 1; }
                break;
            case ErrorAt:
                sprintf(err_msg, "error while reading %s", path);
                return 1;
        }
    }
    fclose(fp);
    if (line) { free(line); }
    return 0;
}

int quoteExp() {
    // TODO
}

void repl(ParseResult* parse_res) {
    char input_buffer[200];
    char carry[200];
    carry[0] = '\0';
    char str[400];
    Sexp* out_s;
    //Sexp* root_set[ENV_SIZE*2];
    RootSet rootSet;
    construct_rootSet(&rootSet);
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
        get_root_set(globalEnv, 0, &rootSet);
        readSexp(str, parse_res, &rootSet);
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
