#include "builtins.h"

lval* builtin_head(lenv* e, lval* a) {
	LASSERT_NUM("head", a, 1);
	LASSERT_TYPE("head", a, 0, LVAL_QEXPR);
	LASSERT_NOT_EMPTY("head", a, 0);

	lval* v = lval_take(a, 0);
	while (v->count > 1) {
		lval_del(lval_pop(v, 1));
	}
	return v;
}

lval* builtin_tail(lenv* e, lval* a) {
	LASSERT_NUM("tail", a, 1);
	LASSERT_TYPE("tail", a, 0, LVAL_QEXPR);
	LASSERT_NOT_EMPTY("tail", a, 0);

	lval* v = lval_take(a, 0);
	lval_del(lval_pop(v, 0));
	return v;
}

lval* builtin_list(lenv* e, lval* a) {
	a->type = LVAL_QEXPR;
	return a;
}

lval* builtin_join(lenv* e, lval* a) {
	for (int i = 0; i < a->count; i++) {
		LASSERT_TYPE("join", a, i, LVAL_QEXPR);
	}

	lval* x = lval_pop(a, 0);
	while (a->count) {
		x = lval_join(x, lval_pop(a, 0));
	}
	lval_del(a);
	return x;
}

lval* builtin_eval(lenv* e, lval* a) {
	LASSERT_NUM("eval", a, 1);
	LASSERT_TYPE("eval", a, 0, LVAL_QEXPR);

	lval* x = lval_take(a, 0);
	x->type = LVAL_SEXPR;
	return lval_eval(e, x);
}

static lval* builtin_var(lenv* e, lval* v, char* func) {
	LASSERT_TYPE(func, v, 0, LVAL_QEXPR);

	lval* syms = v->cell[0];
	for (int i = 0; i < syms->count; i++) {
		LASSERT(v, syms->cell[i]->type == LVAL_SYM,
				"Function 'def' cannot define non-symbol "
				"Got %s, Expected %s.", ltype_name(syms->cell[i]->type),
				ltype_name(LVAL_SYM));
	}

	LASSERT(v, syms->count == v->count - 1,
			"Function 'def' cannot define incorrect "
			"number of values to symbols"
			"Got %i, Expected %i.", v->count - 1, syms->count);

	for (int i = 0; i < syms->count; i++) {
		if (strcmp(func, "def") == 0) {
			lenv_def(e, syms->cell[i], v->cell[i + 1]);
		}

		if (strcmp(func, "=") == 0) {
			lenv_put(e, syms->cell[i], v->cell[i + 1]);
		}
	}

	lval_del(v);
	return lval_sexpr();
}

lval* builtin_def(lenv* e, lval* v) {
	return builtin_var(e, v, "def");
}

lval* builtin_put(lenv* e, lval* v) {
	return builtin_var(e, v, "=");
}

lval* builtin_lambda(lenv* e, lval* a) {
	LASSERT_NUM("\\", a, 2);
	LASSERT_TYPE("\\", a, 0, LVAL_QEXPR);
	LASSERT_TYPE("\\", a, 1, LVAL_QEXPR);

	for (int i = 0; i < a->cell[0]->count; i++) {
		LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
				"Cannot define non-symbol. Got %s, Expected %s.",
				ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
	}

	lval* formals = lval_pop(a, 0);
	lval* body = lval_pop(a, 0);

	lval_del(a);
	return lval_lambda(formals, body, e);
}

lval* builtin_macro(lenv* e, lval* a) {
       	LASSERT_NUM("macro", a, 2);
	LASSERT_TYPE("macro", a, 0, LVAL_QEXPR);
	LASSERT_TYPE("macro", a, 1, LVAL_QEXPR);

	for (int i = 0; i < a->cell[0]->count; i++) {
		LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
				"Cannot define non-symbol. Got %s, Expected %s.",
				ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
	}

	lval* formals = lval_pop(a, 0);
	lval* body = lval_pop(a, 0);

	lval_del(a);
	return lval_macro(formals, body, e);
}

lval* builtin_function(lenv* e, lval* a) {
	LASSERT_NUM("fun", a, 2);
	LASSERT_TYPE("fun", a, 0, LVAL_QEXPR);
	LASSERT_TYPE("fun", a, 1, LVAL_QEXPR);

	LASSERT(a, (a->cell[0]->count >= 1), "Function name needed.");

	for (int i = 0; i < a->cell[0]->count; i++) {
		LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
				"Cannot define non-symbol. Got %s, Expected %s.",
				ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
	}

	lval* formals = lval_pop(a, 0);
	lval* body = lval_pop(a, 0);
	lval_del(a);

	lval* fun_name = lval_pop(formals, 0);
	
	lval* lambda = lval_lambda(formals, body, e);
	lenv_def(e, fun_name, lambda);
	lval_del(fun_name);
	lval_del(lambda);
	return lval_sexpr();

}

static lval* builtin_op(lenv* e, lval* a, char* op) {
	for (int i = 0; i < a->count; i++) {
		if (a->cell[i]->type != LVAL_NUM) {
			lval_del(a);
			return lval_err("Cannot operate on non-number!");
		}
	}

	lval* x = lval_pop(a, 0);
	if ((strcmp(op, "-") == 0) && a->count == 0) {
		x->num = -x->num;
	}

	while (a->count > 0) {
		lval* y = lval_pop(a, 0);

		if (strcmp(op, "+") == 0) { x->num += y->num; }
		if (strcmp(op, "-") == 0) { x->num -= y->num; }
		if (strcmp(op, "*") == 0) { x->num *= y->num; }
		if (strcmp(op, "/") == 0) {
			if (y->num == 0) {
				lval_del(x);
				lval_del(y);
				x = lval_err("Division By Zero!");
				break;
			}
			x->num /= y->num;
		}
		if (strcmp(op, "%") == 0) {
			if (y->num == 0) {
				lval_del(x);
				lval_del(y);
				x = lval_err("Division By Zero!");
				break;
			}
			x->num %= y->num;
		}
		lval_del(y);
	}
	lval_del(a);
	return x;
}

lval* builtin_add(lenv* e, lval* v) {
	return builtin_op(e, v, "+");
}

lval* builtin_sub(lenv* e, lval* v) {
	return builtin_op(e, v, "-");
}

lval* builtin_mul(lenv* e, lval* v) {
	return builtin_op(e, v, "*");
}

lval* builtin_div(lenv* e, lval* v) {
	return builtin_op(e, v, "/");
}

lval* builtin_mod(lenv* e, lval* v) {
	return builtin_op(e, v, "%");
}

static lval* builtin_ord(lenv* e, lval* v, char* fun) {
	LASSERT_NUM(fun, v, 2);
	LASSERT_TYPE(fun, v, 0, LVAL_NUM);
	LASSERT_TYPE(fun, v, 1, LVAL_NUM);

	int r = 0;
	int vl = v->cell[0]->num;
	int vr = v->cell[1]->num;
	if (strcmp(fun, ">") == 0) {
		r = vl > vr;
	} else if (strcmp(fun, "<") == 0) {
		r = vl < vr;
	} else if (strcmp(fun, ">=") == 0) {
		r = vl >= vr;
	} else if (strcmp(fun, ">") == 0) {
		r = vl > vr;
	} else if (strcmp(fun, "<=") == 0) {
		r = vl <= vr;
	}
	lval_del(v);
	return lval_num(r);
}

lval* builtin_gt(lenv* e, lval* v) {
	return builtin_ord(e, v, ">");
}

lval* builtin_lt(lenv* e, lval* v) {
	return builtin_ord(e, v, "<");
}

lval* builtin_ge(lenv* e, lval* v) {
	return builtin_ord(e, v, ">=");
}

lval* builtin_le(lenv* e, lval* v) {
	return builtin_ord(e, v, "<=");
}

static lval* builtin_cmp(lenv* e, lval* v, char* fun) {
	LASSERT_NUM(fun, v, 2);

	int r = 0;
	lval* vl = v->cell[0];
	lval* vr = v->cell[1];
	if (strcmp(fun, "==") == 0) {
		r = lval_eq(vl, vr);
	} else if (strcmp(fun, "!=") == 0) {
		r = !lval_eq(vl, vr);
	}

	lval_del(v);
	return lval_num(r);
}

lval* builtin_eq(lenv* e, lval* v) {
	return builtin_cmp(e, v, "==");
}

lval* builtin_ne(lenv* e, lval* v) {
	return builtin_cmp(e, v, "!=");
}

lval* builtin_if(lenv* e, lval* v) {
	LASSERT_NUM("if", v, 3);
	LASSERT_TYPE("if", v, 0, LVAL_NUM);
	LASSERT_TYPE("if", v, 1, LVAL_QEXPR);
	LASSERT_TYPE("if", v, 2, LVAL_QEXPR);

	v->cell[1]->type = LVAL_SEXPR;
	v->cell[2]->type = LVAL_SEXPR;

	if (v->cell[0]->num) {
		return lval_eval(e, lval_take(v, 1));
	} else {
		return lval_eval(e, lval_take(v, 2));
	}
}

lval* builtin_print(lenv* e, lval* v) {
	LASSERT_NUM("print", v, 1);
	lval_println(v->cell[0]);
	lval_del(v);
	return lval_sexpr();
}
