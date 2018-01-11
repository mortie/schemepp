#include <chibi/eval.h>

char *scheme_stdlib =
#include "../scheme-std.c"
;

enum parsestate {
	PARSE_NORMAL,
	PARSE_COMMENT,
	PARSE_ML_COMMENT,
	PARSE_STRING
};

void run_scheme(sexp ctx, char *str, FILE *out)
{
	sexp res = sexp_eval_string(ctx, str, -1, NULL);
	if (sexp_stringp(res))
	{
		fprintf(
			out, "%.*s",
			sexp_string_size(res),
			sexp_string_data(res));
	}
	else if (sexp_exceptionp(res))
	{
		sexp_print_exception(ctx, res, sexp_current_error_port(ctx));
	}
	else if (res != SEXP_VOID)
	{
		fprintf(stderr, "Warning: value is neither void nor string\n");
	}
}

int read_scheme(sexp ctx, FILE *in, FILE *out)
{
	// Init static buffer
	static char *buf;
	static size_t bufsize;
	if (bufsize == 0)
	{
		bufsize = 4;
		buf = malloc(bufsize);
	}

	size_t buflen;

	buflen = 0;
	buf[buflen++] = '(';

	enum parsestate state = PARSE_NORMAL;
	int depth = 1;
	char prev2 = '\0';
	char prev = '\0';
	char curr = '(';

	while (depth > 0)
	{
		prev2 = prev;
		prev = curr;
		int r = getc(in);
		if (r == EOF)
		{
			fprintf(stderr, "Reached EOF while searching for a closing ')'.\n");
			return -1;
		}
		curr = r;

		if (state == PARSE_NORMAL)
		{
			if (curr == '(')
				depth += 1;
			else if (curr == ')')
				depth -= 1;
		}

		// Enter string literal if we're in the normal state,
		// and the current char is a " which is not part of
		// a character literal
		if (state == PARSE_NORMAL && prev2 != '#' && prev != '\\' && curr == '"')
		{
			state = PARSE_STRING;
		}
		// Leave string literal if the current char is a " which isn't escaped
		else if (state == PARSE_STRING && curr == '"')
		{
			if (prev2 == '\\' && prev == '\\')
				prev = '\0';
			else if (prev != '\\')
				state = PARSE_NORMAL;
		}

		// Enter comment if we're in the normal state,
		// and the current char is a ;
		else if (state == PARSE_NORMAL && curr == ';')
		{
			state = PARSE_COMMENT;
		}
		// Leave comment if we reach a newline
		else if (state == PARSE_COMMENT && curr == '\n')
		{
			state = PARSE_NORMAL;
		}

		// Append char, buflen + 1 because we want to be able to append '\0'
		if (buflen + 1 == bufsize)
		{
			bufsize *= 2;
			buf = realloc(buf, bufsize);
		}

		// We might modify curr, but r is the true value getc returned
		buf[buflen++] = curr;
	}

	// Run scheme with the buffer we created
	buf[buflen++] = '\0';
	run_scheme(ctx, buf, out);
	return 0;
}

int preprocess(char *path, FILE *in, FILE *out)
{
	sexp ctx;
	ctx = sexp_make_eval_context(NULL, NULL, NULL, 0, 0);
	sexp_load_standard_env(ctx, NULL, SEXP_SEVEN);
	sexp_load_standard_ports(ctx, NULL, NULL, out, stderr, 0);
	sexp env = sexp_context_env(ctx);

	sexp var_file = sexp_c_string(ctx, path, -1);
	sexp var_file_name = sexp_c_string(ctx, "__file__", -1);
	sexp_env_define(
		ctx, env,
		sexp_string_to_symbol(ctx, var_file_name), var_file);

	run_scheme(ctx, scheme_stdlib, out);

	char start[] = { '#', '#', '(' };
	char pending[sizeof(start)];
	int startidx = 0;

	enum parsestate state = PARSE_NORMAL;
	char prev2 = '\0';
	char prev = '\0';
	char curr = '\0';

	while (1)
	{
		prev2 = prev;
		prev = curr;
		int r = getc(in);
		if (r == EOF)
			break;
		curr = (char)r;

		// Enter string literal if we're in the normal state,
		// and the current char is a " which is not part of
		// a character literal
		if (state == PARSE_NORMAL && prev != '\'' && curr == '"')
		{
			state = PARSE_STRING;
		}
		// Leave string literal if the current char is a " which isn't escaped
		else if (state == PARSE_STRING && curr == '"')
		{
			if (prev2 == '\\' && prev == '\\')
				prev = '\0';
			else if (prev != '\\')
				state = PARSE_NORMAL;
		}

		// Enter comment if we're in the normal state,
		// and the current and previous char is //
		else if (state == PARSE_NORMAL && prev == '/' && curr == '/')
		{
			state = PARSE_COMMENT;
		}
		// Leave comment if we reach a newline
		else if (state == PARSE_COMMENT && curr == '\n')
		{
			state = PARSE_NORMAL;
		}

		// Enter multi-line comment if we're in the normal state,
		// and the current and previous char is /*
		else if (state == PARSE_NORMAL && prev == '/' && curr == '*')
		{
			state = PARSE_ML_COMMENT;
		}
		// Leave multi-line comment if we reach */
		else if (state == PARSE_ML_COMMENT && prev == '*' && curr == '/')
		{
			state = PARSE_NORMAL;
		}

		if (state == PARSE_NORMAL && curr == start[startidx])
		{
			pending[startidx++] = curr;
			if (startidx == sizeof(start))
			{
				startidx = 0;
				if (read_scheme(ctx, in, out) < 0)
					return -1;
				continue;
			}
		}
		else if (startidx != 0)
		{
			fwrite(pending, 1, startidx, out);
			startidx = 0;
			fputc(curr, out);
		}
		else
		{
			fputc(curr, out);
		}
	}

	sexp_destroy_context(ctx);
}

int main(int argc, char *argv[])
{
	char *inpath = NULL;
	FILE *in = stdin, *out = stdout;

	if (argc > 1)
	{
		inpath = realpath(argv[1], NULL);
		if (inpath == NULL)
		{
			perror(argv[1]);
			return 1;
		}

		in = fopen(argv[1], "r");
		if (in == NULL)
		{
			perror(argv[1]);
			return 1;
		}
	}

	if (argc > 2)
	{
		out = fopen(argv[2], "w");
		if (out == NULL)
		{
			perror(argv[2]);
			return 1;
		}
	}

	if (preprocess(inpath, in, out) < 0)
		return 1;
}
