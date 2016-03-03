#include <stdio.h>
#include <stdlib.h>

#include <histedit.h>
#include <editline/readline.h>

#include "mpc.h"

#include "core.h"
#include "builtins.h"

#define FILE_EXT "lspy"

mpc_parser_t* Number;
mpc_parser_t* Symbol;
mpc_parser_t* Sexpr;
mpc_parser_t* Qexpr;
mpc_parser_t* Expr;
mpc_parser_t* Lispy;
mpc_parser_t* Comment;

lval* load(lenv* e, char* modulename) {
        char* filename = malloc(strlen(modulename) + strlen(FILE_EXT) + 2);
        sprintf(filename, "%s.%s", modulename, FILE_EXT);

	mpc_result_t r;
	if (mpc_parse_contents(filename, Lispy, &r)) {
		lval* expr = lval_read(r.output);
		mpc_ast_delete(r.output);
		while (expr->count) {
			lval* x = lval_eval(e, lval_pop(expr, 0));
			if (x->type == LVAL_ERR) {
				lval_println(x);
			}
			lval_del(x);
		}
                lval_del(expr);
                free(filename);
		return lval_sym(modulename);
	} else {
		char* err_msg = mpc_err_string(r.error);
		mpc_err_delete(r.error);
		lval* err = lval_err("Could not load library %s", err_msg);
		free(err_msg);
                free(filename);
		return err;
	}
}

lval* builtin_load(lenv* e, lval* v) {
        LASSERT_NUM("load", v, 1);
        LASSERT_TYPE("load", v, 0, LVAL_QEXPR);

        LASSERT(v, v->cell[0]->count == 1, "Function 'load' expect one modulename.");
        LASSERT(v, v->cell[0]->cell[0]->type == LVAL_SYM,
                        "Function 'load' expect Q-Expression with Symbol name");

        lval* res = load(e, v->cell[0]->cell[0]->sym);
        lval_del(v);
        return res;
}

void add_builtins(lenv* e) {
	lenv_add_builtin(e, "\\", builtin_lambda);
	lenv_add_builtin(e, "fun", builtin_function);

	lenv_add_builtin(e, "list", builtin_list);
	lenv_add_builtin(e, "head", builtin_head);
	lenv_add_builtin(e, "tail", builtin_tail);
	lenv_add_builtin(e, "join", builtin_join);
	lenv_add_builtin(e, "eval", builtin_eval);
	lenv_add_builtin(e, "def",  builtin_def);
	lenv_add_builtin(e, "=", builtin_put);
	lenv_add_builtin(e, "print", builtin_print);

	lenv_add_builtin(e, "+", builtin_add);
	lenv_add_builtin(e, "-", builtin_sub);
	lenv_add_builtin(e, "*", builtin_mul);
	lenv_add_builtin(e, "/", builtin_div);
	lenv_add_builtin(e, "%", builtin_mod);

	lenv_add_builtin(e, "<", builtin_lt);
	lenv_add_builtin(e, ">", builtin_gt);
	lenv_add_builtin(e, "<=", builtin_le);
	lenv_add_builtin(e, ">=", builtin_ge);

	lenv_add_builtin(e, "==", builtin_eq);
	lenv_add_builtin(e, "!=", builtin_ne);

	lenv_add_builtin(e, "if", builtin_if);

        lenv_add_builtin(e, "load", builtin_load);
}

int main(int argc, char** argv) {
	Number  = mpc_new("number");
	Symbol  = mpc_new("symbol");
	Sexpr   = mpc_new("sexpr");
	Qexpr   = mpc_new("qexpr");
	Expr    = mpc_new("expr");
	Comment = mpc_new("comment");
	Lispy   = mpc_new("lispy");

	mpca_lang(MPCA_LANG_DEFAULT,
			"                                                            \
			number   : /-?[0-9]+/;                                       \
			symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&%]+/;                \
			comment  : /;[^\\n\\r]*/;                                    \
			sexpr    : '(' <expr>* ')';                                  \
			qexpr    : '{' <expr>* '}';                                  \
			expr     : <number> | <symbol> | <comment>                   \
			         | <sexpr>  | <qexpr>;                               \
			lispy    : /^/ <expr>* /$/;                                  \
			",
			Number, Symbol, Comment, Sexpr, Qexpr, Expr, Lispy);

	puts("Lispy Version 0.0.0.0.1");
	puts("Press Ctrl+c to Exit\n");

	lenv* e = lenv_new();
	add_builtins(e);

        while (1) {
		char* input = readline("lispy>");

		add_history(input);

		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lispy, &r)) {
			// mpc_ast_print(r.output);
			lval* x = lval_eval(e, lval_read(r.output));
			lval_println(x);
			lval_del(x);
			mpc_ast_delete(r.output);
		} else {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}

		free(input);
	}

	lenv_del(e);
	mpc_cleanup(7, Number, Symbol, Sexpr, Qexpr, Expr, Comment, Lispy);
	return 0;
}
