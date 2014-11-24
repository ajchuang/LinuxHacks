#ifndef __UTIL_H__
#define __UTIL_H__

#include "w4118_sh_def.h"

/******************************************************************************/
/* structures                                                                 */
/******************************************************************************/
TYPE struct s_task_info {

	pid_t   m_mypid;        /* my pid, filled by myself */
	int     m_in_fd;        /* the input pipe fd, filled by the prev task */
	int     m_out_fd[2];    /* the output pipe fd, filled by myself */

	e_err_code  m_status;   /* returned status */

	/* list elements */
	struct s_task_info  *mp_prev;
	struct s_task_info  *mp_next;

} s_task_info;

TYPE struct s_task_info_list {
	s_task_info *mp_head;
	s_task_info *mp_tail;
} s_task_info_list;

/******************************************************************************/
/* task list operators                                                        */
/******************************************************************************/
s_task_info_list *new_task_list();
void clear_task_list(s_task_info_list *);

s_task_info *new_task_info();
void free_task_info(s_task_info *);
void enq_to_task_list(s_task_info_list *, s_task_info *);

/******************************************************************************/
/* Err print function                                                         */
/******************************************************************************/
void print_err(e_bool, char *);

/******************************************************************************/
/* system call wrappers                                                       */
/******************************************************************************/
void *sys_alloc(size_t, const char *, int);
void sys_free(void *, const char *, int);

pid_t sys_fork(const char *, int);
int sys_execv(const char *, char *const[], const char *, int);
int sys_pipe(int fd[2], const char *, int);

int sys_dup(int, const char *, int);
int sys_close(int, const char *, int);

#endif
