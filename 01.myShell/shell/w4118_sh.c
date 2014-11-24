/******************************************************************************/
/* @lfred Jen-Chieh Huang (jh3478)                                            */
/******************************************************************************/
/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "w4118_sh_def.h"
#include "util.h"
#include "str_list.h"

/******************************************************************************/
/* Global Variables                                                           */
/******************************************************************************/
e_bool g_debug  = e_false;
e_bool g_kill   = e_false;
FILE *g_dbgFile = NULL;

e_bool g_test   = e_false;
e_bool g_test_fork = e_false;
e_bool g_test_exec = e_false;
e_bool g_test_pipe = e_false;

/******************************************************************************/
/* Static Variables                                                           */
/******************************************************************************/
static s_cmd g_cmd;
static s_str_list *gp_path;

/******************************************************************************/
/* Functions                                                                  */
/******************************************************************************/
/* used to clean the cmd structure */
void reset_cmd(void)
{
	g_cmd.mp_cmd[0] = 0;
	g_cmd.m_len = 0;
}

void sigint_handler(int sig)
{
	DMSG("ctrl-c has noly limited supported.");
}

void shell_init(void)
{
	/* init srand */
	srand(time(NULL));

	/* register event handler */
	signal(SIGINT, sigint_handler);

	/* init debug file */
	fclose(fopen(M_DBG_LOG, "w"));
	fclose(fopen(M_MEM_LOG, "w"));

	/* init path */
	gp_path = new_str_list();

	if (g_debug) {
		enq_to_list(gp_path, "/bin");
		enq_to_list(gp_path, "/usr/bin");
		enq_to_list(gp_path, "./");
	}

	/* init cmd */
	g_cmd.m_curLimit = MAX_CMDBUF_LEN;
	g_cmd.mp_cmd = M_SYS_ALLOC(sizeof(char) * (g_cmd.m_curLimit + 1));
	g_cmd.mp_cmd[g_cmd.m_curLimit] = 0;

	/* @lfred: it has to be okay - otherwise the program can not continue */
	if (g_cmd.mp_cmd == NULL) {
		DMSG("Failed to initialize shell. Quit.");
		print_err(e_false, "Failed to init shell.");
		exit(-1);
	}

	reset_cmd();
}

void consume_stdin(char c)
{
	do {
		if (c == M_RETURN || c == M_NEW_LINE)
			return;

		c = getc(stdin);
	} while (e_true);
}

e_bool read_stdin(void)
{
	char c;
	char pc = 0;
	e_bool starting_flag = e_false;

	reset_cmd();

	do {
		c = getc(stdin);

		if (c == EOF) {
			DMSG("STDIN EOF !? WTF");
			exit(-1);
		}

		/* process memory problem */
		if (g_cmd.m_len == g_cmd.m_curLimit) {
			DMSG("input too long. realloc: %s", g_cmd.mp_cmd);

			int sz = g_cmd.m_curLimit + M_CMDBUF_INC + 1;
			char *p = realloc(g_cmd.mp_cmd, sz);

			if (p != NULL) {
				g_cmd.m_curLimit = sz - 1;
				g_cmd.mp_cmd = p;
				g_cmd.mp_cmd[g_cmd.m_curLimit] = 0;

				DMSG("new input buffer: %d", g_cmd.m_curLimit);
			} else {
				print_err(e_true, "system out of memory");
				reset_cmd();
				consume_stdin(c);
				return e_false;
			}
		}

		/* start processing */
		if (starting_flag == e_false) {
			if (c == M_SPACE || c == M_TAB) {
				continue;
			} else if (c == M_PIPE_SYM) {
				reset_cmd();
				print_err(
					e_true,
					"incorrect pipe position");
				consume_stdin(c);
				return e_false;
			}

			/* other cases */
			starting_flag = e_true;
		}

		if (c == M_RETURN || c == M_NEW_LINE) {

			/* do not execute empty string */
			if (g_cmd.m_len == 0)
				return e_false;
			else if (pc == M_PIPE_SYM) {
				reset_cmd();
				print_err(e_true, "finished with pipe");
				return e_false;
			}

			g_cmd.mp_cmd[g_cmd.m_len] = 0;
			DMSG("The input completed: %s", g_cmd.mp_cmd);
			break;
		}

		/* other cases */
		pc = c;
		g_cmd.mp_cmd[g_cmd.m_len] = c;
		g_cmd.m_len++;

		if (c == M_PIPE_SYM)
			starting_flag = e_false;

	} while (1);

	return e_true;
}

/* parse the command - seperate the command by '|' */
s_str_list *parse_cmd(s_task_info_list *p_task_list)
{
	char *p_tok;
	s_str_list *plist = new_str_list();

	if (plist == NULL)
		return NULL;

	p_tok = strtok(g_cmd.mp_cmd, "|");

	if (p_tok != NULL) {
		DMSG("token found: %s", p_tok);
		enq_to_list(plist, p_tok);
		enq_to_task_list(p_task_list, new_task_info());
	}

	while ((p_tok = strtok(NULL, "|")) != NULL) {

		if (p_tok != NULL) {
			DMSG("token found: %s", p_tok);
			enq_to_list(plist, p_tok);
			enq_to_task_list(p_task_list, new_task_info());
		}
	}

	return plist;
}

char *search_cmd_path(char *cmd, e_err_code *p_code)
{
	char *tmp;
	struct stat f_st;
	s_str_elm *ptr = gp_path->mp_head;
	int cmd_len = strlen(cmd) + 1;

	*p_code = e_err_ok;

	/* check if the path is itself a valid path */
	if (stat(cmd, &f_st) == 0 && (f_st.st_mode & S_IXUSR)) {
		tmp = (char *) M_SYS_ALLOC(sizeof(char) * (cmd_len + 1));

		if (tmp == NULL)
			*p_code = e_err_out_of_memory;
		else
			strcpy(tmp, cmd);

		return tmp;
	}

	/* check all other paths */
	while (ptr != NULL) {

		int sz = sizeof(char) * (cmd_len + strlen(ptr->mp_str) + 1);

		tmp = (char *) M_SYS_ALLOC(sz);
		if (tmp == NULL) {
			*p_code = e_err_out_of_memory;
			return NULL;
		}

		strcpy(tmp, ptr->mp_str);
		strcat(tmp, "/");
		strcat(tmp, cmd);
		DMSG("Searching: %s", tmp);

		if (stat(tmp, &f_st) == 0) {
			if (f_st.st_mode & S_IXUSR) {
				DMSG("Path found: %s", tmp);
				return tmp;
			}
		} else {
			DMSG("file not found: %s", strerror(errno));
		}

		M_SYS_FREE(tmp);
		ptr = ptr->mp_next;
	}

	/* TODO: set error code, not found */
	return NULL;
}

/* This func is used to check if the commands are executable */
/* 1. if using pipe, check if the shell commands are mixed.  */
/* 2. if the exec in the path */
e_err_code check_cmd(s_str_list *plist)
{
	int cnt = plist->m_cnt;
	s_str_elm *ptr = plist->mp_head;
	e_err_code p_ret;

	while (ptr != NULL) {

		char *p_tmp = M_SYS_ALLOC(strlen(ptr->mp_str) + 1);

		strcpy(p_tmp, ptr->mp_str);

		char *tok = strtok(p_tmp, " ");

		/* check if a shell command */
		if ((strcmp(M_CMDSTR_DEBUG, tok) == 0) ||
		    (strcmp(M_CMDSTR_EXIT,  tok) == 0) ||
		    (strcmp(M_CMDSTR_CD,    tok) == 0) ||
		    (strcmp(M_CMDSTR_PATH,  tok) == 0) ||
		    (strcmp(M_CMDSTR_TEST,  tok) == 0)) {
			if (cnt > 1) {
				print_err(e_true, "shell command with pipe");
				M_SYS_FREE(p_tmp);
				return e_err_shell_cmd_with_pipe;
			}
		} else {

			char *p = search_cmd_path(tok, &p_ret);

			if (p == NULL) {
				print_err(e_true, tok);
				print_err(e_true, "cmd not found");
				M_SYS_FREE(p_tmp);
				return e_err_cmd_not_found;
			}

			M_SYS_FREE(p);

			int c = 0;

			while ((tok = strtok(NULL, " ")) != NULL)
				c++;

			if (c > MAX_PARAM_NUM) {
				print_err(e_true, "too many params");
				M_SYS_FREE(p_tmp);
				return e_err_exceed_max_param;
			}
		}

		M_SYS_FREE(p_tmp);
		ptr = ptr->mp_next;
	}

	return e_err_ok;
}

void cmd_proc_debug(void)
{
	DMSG("cmd_proc_debug");

	if (g_debug == e_true)
		g_debug = e_false;
	else
		g_debug = e_true;

	M_ERR("current debug mode: %d", g_debug);
}

void cmd_proc_test(s_tok_array *toks)
{
	DMSG("cmd_proc_test, cnt = %d", toks->m_cnt);

	if (toks->m_cnt != 2 && toks->m_cnt != 3) {
		print_err(e_true, "incorrect format");
		return;
	}

	if (toks->m_cnt == 2) {
		if (g_test == e_true) {
			g_test = e_false;
			print_err(e_true, "test mode off");
		} else {
			g_test = e_true;
			print_err(e_true, "test mode on");
		}
	}

	if (toks->m_cnt == 3) {
		if (strcmp(toks->m_toks[1], "fork") == 0) {
			if (g_test_fork == e_true) {
				g_test_fork = e_false;
				print_err(e_true, "fork test off");
			} else {
				g_test_fork = e_true;
				print_err(e_true, "fork test on");
			}
		}

		if (strcmp(toks->m_toks[1], "exec") == 0) {
			if (g_test_exec == e_true) {
				g_test_exec = e_false;
				print_err(e_true, "exec test off");
			} else {
				g_test_exec = e_true;
				print_err(e_true, "exec test on");
			}
		}

		if (strcmp(toks->m_toks[1], "pipe") == 0) {
			if (g_test_pipe == e_true) {
				g_test_pipe = e_false;
				print_err(e_true, "exec pipe off");
			} else {
				g_test_pipe = e_true;
				print_err(e_true, "exec pipe on");
			}
		}

	}

	DMSG("current testing mode: %d", g_test);
	DMSG("current testing mode (fork): %d", g_test_fork);
	DMSG("current testing mode (exec): %d", g_test_exec);
	DMSG("current testing mode (pipe): %d", g_test_pipe);
}

void cmd_proc_cd(s_tok_array *toks)
{
	DMSG("cmd_proc_cd");

	if (toks->m_cnt != 3) {
		DMSG("token count = %d", toks->m_cnt);
		print_err(e_true, "incorrect format");
		return;
	}

	/* DO actual jobs */
	DMSG("change dir to %s", toks->m_toks[1]);

	if (chdir(toks->m_toks[1]) != 0)
		print_err(e_false, NULL);
}

void cmd_proc_path(s_tok_array *p_tok_ary)
{
	s_str_elm *p_str;

	DMSG("cmd_proc_path, Num of toks = %d", p_tok_ary->m_cnt);

	if (p_tok_ary->m_cnt == 2) {

		dump_list(gp_path, __func__, __LINE__);

		p_str = gp_path->mp_head;

		while (p_str != NULL) {
			fprintf(stderr, "%s", p_str->mp_str);
			p_str = p_str->mp_next;

			if (p_str != NULL)
				fprintf(stderr, ":");
		}

		fprintf(stderr, "\n");
	} else {

		if (p_tok_ary->m_cnt != 4) {
			print_err(e_true, "incorrect format");
			return;
		}

		/* if token count is correct */
		if (strcmp(p_tok_ary->m_toks[1], "+") == 0) {
			if (check_existence(gp_path, p_tok_ary->m_toks[2])) {
				print_err(e_true, "path already set.");
			} else {
				enq_to_list(gp_path, p_tok_ary->m_toks[2]);
				dump_list(gp_path, __func__, __LINE__);
				M_ERR("path %s added", p_tok_ary->m_toks[2]);
			}
		} else if (strcmp(p_tok_ary->m_toks[1], "-") == 0) {
			/* TODO */
			p_str = rem_from_list(gp_path, p_tok_ary->m_toks[2]);

			if (p_str != NULL) {
				M_ERR("path %s removed", p_tok_ary->m_toks[2]);
				free_str_elm(p_str);
			} else {
				print_err(e_true, "designated path not found");
			}
		} else {
			print_err(e_true, "incorrect format");
		}
	}
}

e_err_code shell_cmd_proc(s_tok_array *p_tok_ary)
{
	e_err_code ret = e_err_ok;

	/* toggle debug function */
	if (strcmp(M_CMDSTR_DEBUG, p_tok_ary->m_toks[0]) == 0)
		cmd_proc_debug();
	else if (strcmp(M_CMDSTR_EXIT, p_tok_ary->m_toks[0]) == 0)
		ret = e_err_quit;
	else if (strcmp(M_CMDSTR_CD, p_tok_ary->m_toks[0]) == 0)
		cmd_proc_cd(p_tok_ary);
	else if (strcmp(M_CMDSTR_PATH, p_tok_ary->m_toks[0]) == 0)
		cmd_proc_path(p_tok_ary);
	else if (strcmp(M_CMDSTR_TEST, p_tok_ary->m_toks[0]) == 0)
		cmd_proc_test(p_tok_ary);
	else
		ret = e_err_not_a_shell_cmd;

	return ret;
}

e_err_code cmd_proc(s_str_elm *p_cmd, s_task_info *p_task)
{
	pid_t my_pid;
	s_tok_array *p_tok_ary;
	char *tmp;
	e_err_code r;

	DMSG("Exec cmd: %s", p_cmd->mp_str);
	p_tok_ary = str_to_tok_array(p_cmd->mp_str);

	if (p_tok_ary == NULL)
		return e_err_out_of_memory;

	r = shell_cmd_proc(p_tok_ary);

	if (r != e_err_not_a_shell_cmd) {
		free_tok_array(p_tok_ary);
		return r;
	}

	/* search path */
	e_err_code ret_code;

	tmp = search_cmd_path(p_tok_ary->m_toks[0], &ret_code);
	if (tmp == NULL) {
		switch (ret_code) {
		case e_err_ok:
			print_err(e_true, "command not found");
			break;

		case e_err_out_of_memory:
			print_err(e_true, "system out of memory");
			break;

		case e_err_sys_call_error:
			print_err(e_false, "system call failed");
			break;

		default:
			print_err(e_true, "unknown error");
			break;
		}

		free_tok_array(p_tok_ary);
		return e_err_ok;
	}

	/* create and assign pipes */
	int fds[2] = {M_INVALID_FD, M_INVALID_FD};

	if (p_task->mp_next != NULL || p_task->mp_prev != NULL) {

		if (p_task->mp_prev != NULL) {
			p_task->m_in_fd = p_task->mp_prev->m_out_fd[M_READ_END];
			DMSG("set input fd: %d", p_task->m_in_fd);
		}

		if (p_task->mp_next != NULL) {

			if (M_SYS_PIPE(fds) != 0) {
				print_err(e_false, "failed to create pipe.");
				M_SYS_FREE(tmp);
				return e_err_ok;
			}

			p_task->m_out_fd[M_READ_END]  = fds[M_READ_END];
			p_task->m_out_fd[M_WRITE_END] = fds[M_WRITE_END];

			DMSG("set output fd: %d, %d",
			     p_task->m_out_fd[M_READ_END],
			     p_task->m_out_fd[M_WRITE_END]);
		}
	}

	my_pid = M_SYS_FORK();

	if (my_pid == 0) {
		/* I am the child */
		DMSG("child process is running: %s", tmp);

		if (p_task->m_in_fd != M_INVALID_FD) {
			DMSG("closing stdin. use %d instead", p_task->m_in_fd);
			M_SYS_CLOSE(M_STDIN);
			M_SYS_DUP(p_task->m_in_fd);
		}

		if (p_task->m_out_fd[M_WRITE_END] != M_INVALID_FD) {
			DMSG("closing stdout. use %d instead",
			     p_task->m_out_fd[M_WRITE_END]);

			M_SYS_CLOSE(M_STDOUT);
			M_SYS_DUP(p_task->m_out_fd[M_WRITE_END]);
		}

		if (p_task->m_out_fd[M_READ_END] != M_INVALID_FD)
			M_SYS_CLOSE(p_task->m_out_fd[M_READ_END]);

		if (M_SYS_EXECV(tmp, p_tok_ary->m_toks) == -1) {
			print_err(e_false, NULL);
			exit(-1);
		}

	} else if (my_pid > 0) {
		/* I am the parent */
		DMSG("pid %d is created.", my_pid);
		p_task->m_mypid = my_pid;
		p_task->m_status = e_err_task_running;

		/* Pipe creator needs also to close the pipe */
		if (p_task->m_in_fd != M_INVALID_FD)
			M_SYS_CLOSE(p_task->m_in_fd);

		if (p_task->m_out_fd[M_WRITE_END] != M_INVALID_FD)
			M_SYS_CLOSE(p_task->m_out_fd[M_WRITE_END]);

	} else {
		M_SYS_FREE(tmp);
		free_tok_array(p_tok_ary);
		print_err(e_false, "fork failure");
		return e_err_fork_failed;
	}

	M_SYS_FREE(tmp);
	free_tok_array(p_tok_ary);
	return e_err_ok;
}

int main(void)
{
	e_err_code cmd_state;

	DMSG("w4118_sh is starting.");
	shell_init();

	do {
		/* print the prompt */
		fprintf(stderr, "$");

		/* read user input */
		if (read_stdin() == e_false)
			continue;

		/* find all the processed needed to run */
		/* parse the | symbol to get a series of commands */
		s_task_info_list *p_task_list = new_task_list();

		if (p_task_list == NULL) {
			print_err(e_true, "system out of memory");
			continue;
		}

		/* parsing the input command */
		s_str_list *procs = parse_cmd(p_task_list);

		if (procs == NULL) {
			print_err(e_true, "system out of memory");

			/* clean up the memory */
			clear_task_list(p_task_list);
			continue;
		}

		/* the commands are not correct */
		if (check_cmd(procs) != e_err_ok) {
			free_str_list(procs);
			clear_task_list(p_task_list);
			continue;
		}

		s_task_info *p_task = p_task_list->mp_head;

		do {
			if (is_empty(procs) == e_true) {
				DMSG("empty procs list");
				break;
			}

			if (p_task == NULL) {
				print_err(e_true, "system out of memory");
				break;
			}

			s_str_elm *p_elm = deq_from_list(procs);

			/* actually exec the command */
			cmd_state = cmd_proc(p_elm, p_task);
			free_str_elm(p_elm);

			if (cmd_state == e_err_quit) {
				DMSG("user quit");
				break;
			} else if (cmd_state == e_err_fork_failed) {
				DMSG("sys call failed.");
				break;
			}

			p_task = p_task->mp_next;

		} while (e_true);

		/* Prof. Nieh's code */
		int status, r;

		while ((r = wait(&status)) != -1 || errno != ECHILD)
			DMSG("child process %d is over.", r);

		/* clean up the memory */
		free_str_list(procs);
		clear_task_list(p_task_list);

	} while (cmd_state != e_err_quit);

	DMSG("\n\nw4118_sh is now stopped.");
	return 1;
}
