#ifndef builtins_h
#define builtins_h

#include "core.h"

lval* builtin_head(lenv* e, lval* a);
lval* builtin_tail(lenv* e, lval* a);
lval* builtin_list(lenv* e, lval* a);
lval* builtin_join(lenv* e, lval* a);

lval* builtin_eval(lenv* e, lval* a);

lval* builtin_def(lenv* e, lval* v);
lval* builtin_put(lenv* e, lval* v);

lval* builtin_lambda(lenv* e, lval* a);
lval* builtin_function(lenv* e, lval* a);

lval* builtin_add(lenv* e, lval* v);
lval* builtin_sub(lenv* e, lval* v);
lval* builtin_mul(lenv* e, lval* v);
lval* builtin_div(lenv* e, lval* v);
lval* builtin_mod(lenv* e, lval* v);

lval* builtin_gt(lenv* e, lval* v);
lval* builtin_lt(lenv* e, lval* v);
lval* builtin_ge(lenv* e, lval* v);
lval* builtin_le(lenv* e, lval* v);

lval* builtin_eq(lenv* e, lval* v);
lval* builtin_ne(lenv* e, lval* v);

lval* builtin_if(lenv* e, lval* v);
lval* builtin_print(lenv* e, lval* v);

lval* builtin_load(lenv* e, lval* v);

#endif
