#ifndef MY_ABC_HERE
#define MY_ABC_HERE
#endif
 
struct match_token {
	int token;
	const char *pattern;
};

typedef struct match_token match_table_t[];

enum {MAX_OPT_ARGS = 3};

typedef struct {
	char *from;
	char *to;
} substring_t;

int match_token(char *, const match_table_t table, substring_t args[]);
int match_int(substring_t *, int *result);
int match_octal(substring_t *, int *result);
int match_hex(substring_t *, int *result);
bool match_wildcard(const char *pattern, const char *str);
size_t match_strlcpy(char *, const substring_t *, size_t);
char *match_strdup(const substring_t *);

#if defined(MY_ABC_HERE) || defined(MY_ABC_HERE)
int SYNO_get_option_ul(substring_t *args, unsigned long *option);
#endif
