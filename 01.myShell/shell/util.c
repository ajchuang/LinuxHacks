#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "util.h"
#include "w4118_sh_ext.h"

s_task_info_list *new_task_list()
{
	s_task_info_list *p_list;

	p_list = (s_task_info_list *) M_SYS_ALLOC(sizeof(s_task_info_list));

	if (p_list == NULL)
		return NULL;

	p_list->mp_head = NULL;
	p_list->mp_tail = NULL;

	return p_list;
}

void clear_task_list(s_task_info_list *plist)
{
	s_task_info *p_info, *tmp;

	if (plist == NULL)
		return;

	p_info = plist->mp_head;

	while (p_info != NULL) {
		tmp = p_info;
		free_task_info(tmp);
		p_info = p_info->mp_next;
	}

	M_SYS_FREE(plist);
}

s_task_info *new_task_info()
{
	s_task_info *p_task;

	p_task = (s_task_info *) M_SYS_ALLOC(sizeof(s_task_info));

	if (p_task == NULL) {
		DMSG("new_task_info: out of memory");
		return NULL;
	}

	p_task->m_mypid = M_INVALID_PID;
	p_task->mp_next = p_task->mp_prev = NULL;

	p_task->m_in_fd     = M_INVALID_FD;

	p_task->m_out_fd[0] = M_INVALID_FD;
	p_task->m_out_fd[1] = M_INVALID_FD;

	return p_task;
}

void free_task_info(s_task_info *p_task)
{
	M_SYS_FREE(p_task);
}

void enq_to_task_list(s_task_info_list *p_list, s_task_info *p_task)
{
	if (p_list == NULL || p_task == NULL)
		return;

	p_task->mp_next = NULL;

	/* the empty list case */
	if (p_list->mp_tail == NULL) {
		DMSG("enq_to_task_list - 1");
		p_list->mp_head = p_list->mp_tail = p_task;
		p_task->mp_next = p_task->mp_prev = NULL;
	} else {
		DMSG("enq_to_task_list - 2");
		p_task->mp_prev = p_list->mp_tail;
		p_list->mp_tail->mp_next = p_task;
		p_list->mp_tail = p_task;
	}
}

void print_err(e_bool is_custom, char *msg)
{
	if (is_custom == e_true)
		fprintf(stderr, "error: %s\n", msg);
	else {
		fprintf(stderr, "error: %s\n", strerror(errno));

		if (msg != NULL)
			fprintf(stderr, "more info: %s\n", msg);
	}
}

/******************************************************************************/
void *sys_alloc(size_t s, const char *file, int line)
{
	void *p = malloc(s);

	/* testing */
	if (g_test) {
		if (rand() % M_TEST_MEM_ERR_RATE == 0)
			return NULL;
	}

	/* logging */
	if (g_debug) {
		FILE *pfile = fopen(M_MEM_LOG, "a");

		if (pfile != NULL) {
			fprintf(pfile, "malloc: %x, size: %d @ %s, %d",
				(unsigned int)p, (int)s, file, line);
			fclose(pfile);
		}
	}

	return p;
}

void sys_free(void *p, const char *file, int line)
{
	if (g_debug) {
		FILE *pfile = fopen(M_MEM_LOG, "a");

		if (pfile != NULL) {
			fprintf(pfile,
				"free pointer %x @ %s, %d",
				(unsigned int)p, file, line);
			fclose(pfile);
		}
	}

	free(p);
}

pid_t sys_fork(const char *file, int line)
{
	/* testing */
	if (g_test_fork) {
		errno = EAGAIN;
		return -1;
	}

	pid_t p = fork();
	return p;
}

int sys_execv(
	const char *path,
	char *const argv[],
	const char *file,
	int line)
{
	/* testing */
	if (g_test_exec) {
		errno = EFAULT;
		return -1;
	}

	return execv(path, argv);
}

int sys_pipe(int fd[2], const char *file, int line)
{
	/* testing */
	if (g_test_pipe) {
		errno = EFAULT;
		return -1;
	}

	return pipe(fd);
}

int sys_dup(int fd, const char *file, int line)
{
	/* testing */
	if (g_test) {
		if (rand() % M_TEST_SYS_ERR_RATE == 0)
			return -1;
	}

	return dup(fd);
}

int sys_close(int fd, const char *file, int line)
{
	/* testing */
	if (g_test) {
		if (rand() % M_TEST_SYS_ERR_RATE == 0)
			return -1;
	}

	return close(fd);
}



