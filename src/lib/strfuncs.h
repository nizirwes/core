#ifndef __STRFUNC_H
#define __STRFUNC_H

#define is_empty_str(str) \
        ((str) == NULL || (str)[0] == '\0')

#define MAX_INT_STRLEN ((sizeof(uintmax_t) * CHAR_BIT + 2) / 3 + 1)

const char *printf_string_fix_format(const char *fmt) __attr_format_arg__(1);

/* Returns -1 if dest wasn't large enough, 0 if not. */
int i_snprintf(char *dest, size_t max_chars, const char *format, ...)
	__attr_format__(3, 4);

char *p_strdup(Pool pool, const char *str);
char *p_strdup_empty(Pool pool, const char *str); /* return NULL if str = "" */
char *p_strdup_until(Pool pool, const void *start, const void *end); /* *end isn't included */
char *p_strndup(Pool pool, const void *str, size_t max_chars);
char *p_strdup_printf(Pool pool, const char *format, ...) __attr_format__(2, 3);
char *p_strdup_vprintf(Pool pool, const char *format, va_list args);
char *p_strconcat(Pool pool, const char *str1, ...); /* NULL terminated */

/* same with temporary memory allocations: */
const char *t_strdup(const char *str);
char *t_strdup_noconst(const char *str);
const char *t_strdup_empty(const char *str); /* return NULL if str = "" */
const char *t_strdup_until(const void *start, const void *end); /* *end isn't included */
const char *t_strndup(const void *str, size_t max_chars);
const char *t_strdup_printf(const char *format, ...) __attr_format__(1, 2);
const char *t_strdup_vprintf(const char *format, va_list args);
const char *t_strconcat(const char *str1, ...); /* NULL terminated */

/* Like t_strdup(), but stop at cutchar. */
const char *t_strcut(const char *str, char cutchar);

/* Return TRUE if all characters in string are numbers.
   Stop when `end_char' is found from string. */
int is_numeric(const char *str, char end_char);

/* Like strlcpy(), but return -1 if buffer was overflown, 0 if not. */
int strocpy(char *dest, const char *src, size_t dstsize);

/* Print given directory and file to dest buffer, separated with '/'.
   If destination buffer is too small, it's set to empty string and errno is
   set to ENAMETOOLONG. Retuns -1 if buffer is too small, or 0 if not. */
int str_path(char *dest, size_t dstsize, const char *dir, const char *file);
int str_ppath(char *dest, size_t dstsize, const char *dir,
	      const char *file_prefix, const char *file);

char *str_ucase(char *str);
char *str_lcase(char *str);

int memcasecmp(const void *p1, const void *p2, size_t size);

/* seprators is an array of separator characters, not a separator string. */
const char **t_strsplit(const char *data, const char *separators);

const char *dec2str(uintmax_t number);

/* INTERNAL */
const char *_vstrconcat(const char *str1, va_list args, size_t *ret_len);

#endif
