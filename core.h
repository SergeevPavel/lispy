#ifndef core_h
#define core_h

#include "mpc.h"

struct lval;
typedef struct lval lval;
struct lenv;
typedef struct lenv lenv;

enum { LVAL_ERR, LVAL_NUM, LVAL_SYM,
	LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR};

static inline char* ltype_name(int t) {
	switch (t) {
		case LVAL_ERR: return "Error";
		case LVAL_NUM: return "Number";
		case LVAL_SYM: return "Symbol";
		case LVAL_FUN: return "Function";
		case LVAL_SEXPR: return "S-Expression";
		case LVAL_QEXPR: return "Q-Expression";
		default:
				 return "Unknown";
	}
}

typedef lval*(*lbuiltin)(lenv*, lval*);

struct lval {
	int type;

	long num;
	char* err;
	char* sym;

	lbuiltin builtin;
	lenv* env;
	lval* formals;
	lval* body;

	int count;
	struct lval** cell;
};

// lval constructors
lval* lval_num(long x);
lval* lval_err(char* fmt, ...);
lval* lval_sym(char* s);
lval* lval_sexpr(void);
lval* lval_qexpr(void);
lval* lval_fun(lbuiltin func);
lval* lval_lambda(lval* formals, lval* body, lenv* env);

// lval destructor
void lval_del(lval* v);

// lval operations
lval* lval_add(lval* v, lval* x);
lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);
lval* lval_join(lval* x, lval* y);
lval* lval_copy(lval* v);
int lval_eq(lval* x, lval* y);
void lval_print(lval* v);
void lval_expr_print(lval* v, char open, char close);
void lval_println(lval* v);
lval* lval_read_num(mpc_ast_t* t);
lval* lval_read(mpc_ast_t* t);
lval* lval_eval(lenv* e, lval* v);


struct lenv {
	lenv* par;
	int count;
	char** syms;
	lval** vals;

	int ref_counter;
};

// lenv constructor
lenv* lenv_new(void);
lenv* lenv_inher(lenv* par);

// lenv destructor
void lenv_del(lenv* e);

// lenv operations
lenv* lenv_copy(lenv* v);
lval* lenv_get(lenv* e, lval* k);
void lenv_put(lenv* e, lval* k, lval* v);
void lenv_def(lenv* e, lval* k, lval* v);
void lenv_add_builtin(lenv* e, char* name, lbuiltin func);

#define LASSERT(args, cond, fmt, ...)                        \
	if (!(cond)) {                                       \
		lval* err = lval_err(fmt, ##__VA_ARGS__);    \
		lval_del(args);                              \
		return err;                                  \
	}

#define LASSERT_TYPE(func, args, index, expect)                           \
	LASSERT(args, args->cell[index]->type == expect,                  \
			"Function '%s' passed incorrect type "            \
			"for argument %i. Got %s, Expected %s.",          \
			func, index, ltype_name(args->cell[index]->type), \
			ltype_name(expect))

#define LASSERT_NUM(func, args, num)                                           \
	LASSERT(args, args->count == num,                                      \
			"Function '%s' passed incorrect number of arguments. " \
			"Got %i, Expected %i.", func, args->count, num)

#define LASSERT_NOT_EMPTY(func, args, index)                                      \
	LASSERT(args, args->cell[index]->count != 0,                              \
			"Function '%s' passed {} for argument %i.", func, index)

#endif
