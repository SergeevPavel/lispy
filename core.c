#include "core.h"

lval* lval_num(long x) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_NUM;
	v->num = x;
	return v;
}

lval* lval_err(char* fmt, ...) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_ERR;

	va_list va;
	va_start(va, fmt);

	v->err = malloc(512);
	vsnprintf(v->err, 511, fmt, va);

	v->err = realloc(v->err, strlen(v->err) + 1);

	va_end(va);

	return v;
}

lval* lval_sym(char* s) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SYM;
	v->sym = malloc(strlen(s) + 1);
	strcpy(v->sym, s);
	return v;
}

lval* lval_sexpr(void) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

lval* lval_qexpr(void) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_QEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

lval* lval_fun(lbuiltin func) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_FUN;
	v->builtin = func;
	return v;
}

lval* lval_lambda(lval* formals, lval* body, lenv* env) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_FUN;

	v->builtin = NULL;
	v->env = lenv_copy(env);
	v->formals = formals;
	v->body = body;

	return v;
}

lval* lval_macro(lval* formals, lval* body, lenv* env) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_MACRO;

	v->builtin = NULL;
	v->env = lenv_copy(env);
	v->formals = formals;
	v->body = body;

	return v;
}

void lval_del(lval* v) {
	switch (v->type) {
		case LVAL_NUM: break;

		case LVAL_ERR: free(v->err); break;

		case LVAL_SYM: free(v->sym); break;

		case LVAL_MACRO:
		case LVAL_FUN:
			       if (!v->builtin) {
				       lenv_del(v->env);
				       lval_del(v->formals);
				       lval_del(v->body);
			       }
			       break;

		case LVAL_QEXPR:
		case LVAL_SEXPR:
			       for (int i = 0; i < v->count; i++) {
				       lval_del(v->cell[i]);
			       }
			       free(v->cell);
			       break;
	}
	free(v);
}

lval* lval_add(lval* v, lval* x) {
	v->count++;
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	v->cell[v->count - 1] = x;
	return v;
}

lval* lval_pop(lval* v, int i) {
	lval* x = v->cell[i];
	memmove(&v->cell[i], &v->cell[i + 1], sizeof(lval*) * (v->count - i - 1));
	v->count--;
	v->cell = realloc(v->cell, sizeof(lval) * v->count);
	return x;
}

lval* lval_take(lval* v, int i) {
	lval* x = lval_pop(v, i);
	lval_del(v);
	return x;
}

lval* lval_join(lval* x, lval* y) {
	while (y->count) {
		x = lval_add(x, lval_pop(y, 0));
	}

	lval_del(y);
	return x;
}

lval* lval_copy(lval* v) {
	lval* x = malloc(sizeof(lval));
	x->type = v->type;

	switch (v->type) {
		case LVAL_MACRO:
		case LVAL_FUN:
			if (v->builtin) {
				x->builtin = v->builtin;
			} else {
				x->builtin = NULL;
				x->env = lenv_copy(v->env);
				x->formals = lval_copy(v->formals);
				x->body = lval_copy(v->body);
			}
			break;

		case LVAL_NUM: x->num = v->num; break;

		case LVAL_ERR:
			       x->err = malloc(strlen(v->err) + 1);
			       strcpy(x->err, v->err);
			       break;

		case LVAL_SYM:
			       x->sym = malloc(strlen(v->sym) + 1);
			       strcpy(x->sym, v->sym);
			       break;

		case LVAL_SEXPR:
		case LVAL_QEXPR:
			       x->count = v->count;
			       x->cell = malloc(sizeof(lval*) * v->count);
			       for (int i = 0; i < v->count; i++) {
				       x->cell[i] = lval_copy(v->cell[i]);
			       }
			       break;
	}

	return x;
}

int lval_eq(lval* x, lval* y) {
	if (x->type != y->type) {
		return 0;
	}

	switch (x->type) {
		case LVAL_NUM:
			return x->num == y->num;
		case LVAL_ERR:
			return strcmp(x->err, y->err) == 0;
		case LVAL_SYM:
			return strcmp(x->sym, y->sym) == 0;

		case LVAL_QEXPR:
		case LVAL_SEXPR:
			if (x->count != y->count) { return 0; }
			for (int i = 0; i < x->count; i++) {
				if (!lval_eq(x->cell[i], y->cell[i])) {
					return 0;
				}
			}
			return 1;
		case LVAL_MACRO:
		case LVAL_FUN:
			if (x->builtin || y->builtin) {
				return x->builtin == y->builtin;
			} else {
				return lval_eq(x->formals, y->formals) &&
					lval_eq(x->body, y->body);
			}
	}
	return 0;
}

void lval_print(lval* v) {
	switch (v->type) {
		case LVAL_NUM: printf("%li", v->num); break;
		case LVAL_ERR: printf("Error: %s", v->err); break;
		case LVAL_SYM: printf("%s", v->sym); break;
		case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
		case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;
		case LVAL_FUN: 
				 if (v->builtin) {
					 printf("<builtin>");
				 } else {
					 printf("(\\ "); lval_print(v->formals);
					 putchar(' '); lval_print(v->body); putchar(')');
				 }
				 break;
		case LVAL_MACRO:
				 printf("(macro "); lval_print(v->formals);
				 putchar(' '); lval_print(v->body); putchar(')');

	}
}

void lval_expr_print(lval* v, char open, char close) {
	putchar(open);

	for (int i = 0; i < v->count; i++) {
		lval_print(v->cell[i]);

		if (i != v->count - 1) {
			putchar(' ');
		}
	}

	putchar(close);
}

void lval_println(lval* v) {
	lval_print(v);
	putchar('\n');
}

lval* lval_read_num(mpc_ast_t* t) {
	errno = 0;
	long x = strtol(t->contents, NULL, 10);
	return errno != ERANGE ?
		lval_num(x) : lval_err("invalid number");
}

lval* lval_read(mpc_ast_t* t) {
	if (strstr(t->tag, "number")) {
		return lval_read_num(t);
	}
	if (strstr(t->tag, "symbol")) {
		return lval_sym(t->contents);
	}

	lval* x = NULL;
	if (strcmp(t->tag, ">") == 0) {
		x = lval_sexpr();
	}

	if (strstr(t->tag, "sexpr")) {
		x = lval_sexpr();
	}

	if (strstr(t->tag, "qexpr")) {
		x = lval_qexpr();
	}

	for (int i = 0; i < t->children_num; i++) {
		if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
		if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
		if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
		if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
		if (strcmp(t->children[i]->tag, "regex") == 0) { continue; }
		if (strstr(t->children[i]->tag, "comment")) { continue; }
		x = lval_add(x, lval_read(t->children[i]));
	}
	return x;
}

lval* lval_call(lenv* e, lval* f, lval* a) {
	if (f->builtin) {
		return f->builtin(e, a);
	}

	int given = a->count;
	int total = f->formals->count;
        f->env = lenv_inher(f->env);

	while (a->count) {
		if (f->formals->count == 0) {
			lval_del(a);
			return lval_err("Function passed too many arguments. "
					"Got %i, Expected %i.", given, total);
		}

		lval* sym = lval_pop(f->formals, 0);

		if (strcmp(sym->sym, "&") == 0) {
			if (f->formals->count != 1) {
				lval_del(a);
				return lval_err("Function format invalid. "
						"Symbol '&' not followed "
						"by single symbol.");
			}

			lval* nsym = lval_pop(f->formals, 0);
                        a->type = LVAL_QEXPR;
			lenv_put(f->env, nsym, a);
			lval_del(sym);
			lval_del(nsym);
			break;
		}

		lval* val = lval_pop(a, 0);

		lenv_put(f->env, sym, val);

		lval_del(sym);
		lval_del(val);
	}

	lval_del(a);

	if (f->formals->count > 0 &&
			strcmp(f->formals->cell[0]->sym, "&") == 0) {
		if (f->formals->count != 2) {
			return lval_err("Function format invalid. "
					"Symbol '&' not followed by single symbol.");
		}

		lval_del(lval_pop(f->formals, 0));

		lval* sym = lval_pop(f->formals, 0);
		lval* val = lval_qexpr();

		lenv_put(f->env, sym, val);
		lval_del(sym);
		lval_del(val);
	}

	if (f->formals->count == 0) {
		lval* body = lval_copy(f->body);
		body->type = LVAL_SEXPR;
		return lval_eval(f->env, body);
	} else {
		return lval_copy(f);
	}
}

//static lval* get_arg_by_name(char* name, lval* formals, lval* values) {
//	for (int i = 0; i < formals->count; i++) {
//		if (strcmp(formals->cell[i]->sym, name) == 0) {
//			return values->cell[i];
//		}
//	}
//
//	return lval_err("Unbound symbol %s.", name);
//}

static lval* substitute_symobl(lval* v, lval* name, lval* value) {
	switch (v->type) {
		case LVAL_ERR:
		case LVAL_NUM:
			return v;
		case LVAL_SYM:
			if (lval_eq(name, v)) {
				lval_del(v);
				return lval_copy(value);
			}
			return v;
		case LVAL_QEXPR:
		case LVAL_SEXPR:
			for (int i = 0; i < v->count; i++) {
				v->cell[i] = substitute_symobl(v->cell[i], name, value);
			}
			return v;
		case LVAL_FUN:
		case LVAL_MACRO:
			if (v->builtin) { return v; }
			for (int i = 0; i < v->formals->count; i++) {
				if (lval_eq(name, v->formals->cell[i])) {
					return v;
				}
			}
			v->body = substitute_symobl(v->body, name, value);
			return v;
	}
	return v;
}

static lval* lval_macro_subst(lval* m, lval* v) {
	if (m->formals->count != v->count) {
		lval* err = lval_err("Expected %d args but %d given.",
				m->formals->count, v->count);
		lval_del(v);
		return err;
	}
	
	lval* body = lval_copy(m->body);
	for (int i = 0; i < m->formals->count; i++) {
		body = substitute_symobl(body, m->formals->cell[i], v->cell[i]);
	}

	lval_del(v);
	return body;
}

lval* lval_eval_sexpr(lenv* e, lval* v) {
	if (v->count == 0) { return v; }

	lval* f = lval_pop(v, 0);
	f = lval_eval(e, f);

	if (f->type == LVAL_ERR) {
		lval_del(v);
		return f;
	}

	if (f->type != LVAL_FUN && f->type != LVAL_MACRO) {
		if (v->count == 0) { return f; }

		lval* err = lval_err("S-Expression starts with incorrect type. "
				"Got %s, Expected %s or %s.", ltype_name(f->type),
				ltype_name(LVAL_FUN), ltype_name(LVAL_MACRO));
		lval_del(f);
		lval_del(v);
		return err;

	}

	if (f->type == LVAL_FUN) {
		for (int i = 0; i < v->count; i++) {
			v->cell[i] = lval_eval(e, v->cell[i]);
			if (v->cell[i]->type == LVAL_ERR) {
				lval_del(f);
				return lval_take(v, i);
			}
		}
		lval* result = lval_call(e, f, v);
		lval_del(f);
		return result;
	}

	if (f->type == LVAL_MACRO) {
		if (v->count == 0) { return f; }
		lval* result = lval_macro_subst(f, v);
		lval_del(f);
		result->type = LVAL_SEXPR;
		return lval_eval(e, result);
	}

	return NULL; // should not be happen
}

lval* lval_eval(lenv* e, lval* v) {
	if(v->type == LVAL_SYM) {
		lval* x = lenv_get(e, v);
		lval_del(v);
		return x;
	}
	if (v->type == LVAL_SEXPR) {
		return lval_eval_sexpr(e, v);
	}
	return v;
}

lenv* lenv_new(void) {
	lenv* e = malloc(sizeof(lenv));
	e->par = NULL;
	e->count = 0;
	e->syms = NULL;
	e->vals = NULL;
        e->ref_counter = 1;
	return e;
}

void lenv_del(lenv* e) {
        e->ref_counter--;
        if (e->ref_counter)
                return;

	for (int i = 0; i < e->count; i++) {
		free(e->syms[i]);
		lval_del(e->vals[i]);
	}
	free(e->syms);
	free(e->vals);
        lenv_del(e->par);
	free(e);
}

lenv* lenv_copy(lenv* v) {
        v->ref_counter++;
        return v;
}

lenv* lenv_inher(lenv* parent) {
        lenv* e = lenv_new();
        e->par = lenv_copy(parent);
        return e;
}

lval* lenv_get(lenv* e, lval* k) {
	for (int i = 0; i < e->count; i++) {
		if (strcmp(e->syms[i], k->sym) == 0) {
			return lval_copy(e->vals[i]);
		}
	}

	if (e->par) {
		return lenv_get(e->par, k);
	} else {
		return lval_err("Unbound Symbol '%s'", k->sym);
	}
}

void lenv_put(lenv* e, lval* k, lval* v) {
	for (int i = 0; i < e->count; i++) {
		if (strcmp(e->syms[i], k->sym) == 0) {
			lval_del(e->vals[i]);
			e->vals[i] = lval_copy(v);
			return;
		}
	}

	e->count++;
	e->vals = realloc(e->vals, sizeof(lval*) * e->count);
	e->syms = realloc(e->syms, sizeof(char*) * e->count);

	e->vals[e->count - 1] = lval_copy(v);
	e->syms[e->count - 1] = malloc(strlen(k->sym) + 1);
	strcpy(e->syms[e->count - 1], k->sym);
}

void lenv_def(lenv* e, lval* k, lval* v) {
	while (e->par) { e = e->par; }

	lenv_put(e, k, v);
}

void lenv_add_builtin(lenv* e, char* name, lbuiltin func) {
	lval* k = lval_sym(name);
	lval* v = lval_fun(func);
	lenv_put(e, k, v);
	lval_del(k);
	lval_del(v);
}

