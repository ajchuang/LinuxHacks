#ifndef __STR_LIST_H__
#define __STR_LIST_H__

#include "w4118_sh_def.h"

TYPE struct s_str_elm {
	char *mp_str;
	struct s_str_elm *mp_next;
	struct s_str_elm *mp_prev;
} s_str_elm;

TYPE struct s_str_list {
	s_str_elm *mp_head;
	s_str_elm *mp_tail;
	int m_cnt;
} s_str_list;

TYPE struct s_tok_array {
	int m_cnt;
	char *m_toks[MAX_PARAM_NUM + 1];
} s_tok_array;

/* List struct operators */
s_str_list *new_str_list();
void free_str_list(s_str_list *);
void clear_str_list(s_str_list *lst);
e_bool is_empty(s_str_list *);
void dump_list(s_str_list *, const char *, int);
e_bool check_existence(s_str_list *, char *);
int list_size(s_str_list *);

e_err_code enq_to_list(s_str_list *, char *);
s_str_elm *deq_from_list(s_str_list *);
s_str_elm *rem_from_list(s_str_list *, char *);

/* element struct operators */
s_str_elm *new_str_elm(char *);
void free_str_elm(s_str_elm *);

/* Util func: transform long string into a token array (del: space) */
s_tok_array *str_to_tok_array(char *);
void free_tok_array(s_tok_array *);

#endif
