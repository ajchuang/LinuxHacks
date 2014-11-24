#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str_list.h"
#include "util.h"
#include "w4118_sh_ext.h"

/* List struct operators */
s_str_list *new_str_list()
{

	s_str_list *newList = (s_str_list *) M_SYS_ALLOC(sizeof(s_str_list));

	if (newList == NULL)
		return NULL;

	newList->mp_head = newList->mp_tail = NULL;
	newList->m_cnt = 0;
	return newList;
}

void clear_str_list(s_str_list *lst)
{

	s_str_elm *p_head = NULL, *temp = NULL;

	if (lst == NULL)
		return;

	/* start to traverse the list */
	p_head = lst->mp_head;

	while (p_head != NULL) {

		temp = p_head->mp_next;
		free_str_elm(p_head);
		p_head = temp;
	}

	lst->mp_head = NULL;
	lst->mp_tail = NULL;
	lst->m_cnt = 0;
}

void free_str_list(s_str_list *lst)
{

	if (lst == NULL)
		return;

	clear_str_list(lst);

	/* free the list itself */
	M_SYS_FREE(lst);
}

int list_size(s_str_list *p_list)
{
	return p_list->m_cnt;
}

e_err_code enq_to_list(s_str_list *lst, char *str)
{

	s_str_elm *p_elm = new_str_elm(str);

	if (p_elm == NULL) {
		DMSG("enq_to_list: out-of-memory");
		return e_err_out_of_memory;
	}

	if (lst->mp_tail != NULL) {
		DMSG("enq_to_list - 1");
		lst->mp_tail->mp_next = p_elm;
		p_elm->mp_prev = lst->mp_tail;
		lst->mp_tail = p_elm;
	} else {
		DMSG("enq_to_list - 2");
		lst->mp_head = lst->mp_tail = p_elm;
		p_elm->mp_prev = p_elm->mp_next = NULL;
	}

	lst->m_cnt++;
	dump_list(lst, __func__, __LINE__);
	return e_err_ok;
}

s_str_elm *deq_from_list(s_str_list *lst)
{

	s_str_elm *p_head;

	if (lst == NULL)
		return NULL;

	p_head = lst->mp_head;

	if (p_head != NULL) {
		lst->mp_head = p_head->mp_next;

		if (lst->mp_head != NULL)
			p_head->mp_prev = NULL;
		else
			lst->mp_tail = NULL;

		lst->m_cnt--;
		dump_list(lst, __func__, __LINE__);
		return p_head;
	}

	return NULL;
}

s_str_elm *rem_from_list(s_str_list *plist, char *pstr)
{

	s_str_elm *p_elm = plist->mp_head;

	while (p_elm != NULL) {

		/* found the node */
		if (strcmp(p_elm->mp_str, pstr) == 0) {

			if (p_elm->mp_prev != NULL) {
				p_elm->mp_prev->mp_next = p_elm->mp_next;

				/* if removing the last element */
				if (p_elm->mp_next == NULL)
					plist->mp_tail = p_elm->mp_prev;
				else
					p_elm->mp_next->mp_prev =
						p_elm->mp_prev;
			} else {
				if (p_elm->mp_next != NULL) {
					plist->mp_head = p_elm->mp_next;
					plist->mp_head->mp_prev = NULL;
				} else {
					/* if removing the only element */
					plist->mp_head = plist->mp_tail = NULL;
				}
			}

			plist->m_cnt--;
			dump_list(plist, __func__, __LINE__);
			return p_elm;
		}

		p_elm = p_elm->mp_next;
	}

	/* not found */
	return NULL;
}

e_bool is_empty(s_str_list *plist)
{

	if (plist == NULL)
		return e_true;

	if (plist->mp_head == NULL)
		return e_true;

	return e_false;
}

void dump_list(s_str_list *plist, const char *func, int line)
{
	s_str_elm *phead = plist->mp_head;
	FILE *dbg;

	if (g_debug) {
		if (plist == NULL)
			return;
		dbg = fopen(M_DBG_LOG, "a");

		if (dbg != NULL) {
			fprintf(dbg, "===== %s, %d =====\n", func, line);
			fprintf(dbg, "dump list cnt: %d\n", plist->m_cnt);

			while (phead != NULL) {
				fprintf(dbg, "%s\n", phead->mp_str);
				phead = phead->mp_next;
			}

			fprintf(dbg, "=====\n");
			fclose(dbg);
		}
	}
}

e_bool check_existence(s_str_list *plist, char *pstr)
{

	s_str_elm *p_elm = plist->mp_head;

	while (p_elm != NULL) {
		if (strcmp(p_elm->mp_str, pstr) == 0)
			return e_true;

		p_elm = p_elm->mp_next;
	}

	return e_false;
}

/* element struct operators */
s_str_elm *new_str_elm(char *str)
{

	char *pstr;
	int len = strlen(str);
	s_str_elm *p_elm = (s_str_elm *) M_SYS_ALLOC(sizeof(s_str_elm));

	if (p_elm == NULL)
		return NULL;

	pstr = (char *) M_SYS_ALLOC(sizeof(char) * (len + 1));

	if (pstr == NULL) {
		M_SYS_FREE(p_elm);
		return NULL;
	}

	pstr[len] = 0;
	p_elm->mp_str = strcpy(pstr, str);
	p_elm->mp_next = NULL;
	p_elm->mp_prev = NULL;

	return p_elm;
}

void free_str_elm(s_str_elm *p_elm)
{

	if (p_elm == NULL)
		return;

	if (p_elm->mp_str != NULL)
		M_SYS_FREE(p_elm->mp_str);

	M_SYS_FREE(p_elm);
}

s_tok_array *str_to_tok_array(char *p_cmd)
{

	char *p_tok;
	s_tok_array *p_array = (s_tok_array *) M_SYS_ALLOC(sizeof(s_tok_array));

	if (p_array == NULL)
		return NULL;

	p_array->m_toks[MAX_PARAM_NUM] = NULL;
	p_array->m_cnt = 0;
	p_tok = strtok(p_cmd, " ");

	while (p_tok != NULL) {

		if (p_array->m_cnt == MAX_PARAM_NUM - 1) {
			free_tok_array(p_array);
			return NULL;
		}

		p_array->m_toks[p_array->m_cnt] = p_tok;
		p_array->m_cnt++;

		p_tok = strtok(NULL, " ");
	}

	/* append a NULL */
	p_array->m_toks[p_array->m_cnt] = NULL;
	p_array->m_cnt++;

	return p_array;
}

void free_tok_array(s_tok_array *p_array)
{
	M_SYS_FREE(p_array);
}
