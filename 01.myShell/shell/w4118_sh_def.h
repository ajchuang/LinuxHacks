#ifndef __W4118_SH_DEF_H__
#define __W4118_SH_DEF_H__

#define MAX_CMDBUF_LEN	(1024)
#define M_CMDBUF_INC	    (1024)
#define MAX_PARAM_NUM	50
#define TYPE		    typedef

#define M_TEST_MEM_ERR_RATE 10000
#define M_TEST_SYS_ERR_RATE 3

#define M_DBG_LOG       "/tmp/w4118_sh.log"
#define M_MEM_LOG       "/tmp/w4118_sh_mem.log"

#define M_INVALID_PID   (-1)
#define M_INVALID_FD    (-1)
#define M_READ_END      (0)
#define M_WRITE_END     (1)
#define M_STDIN         (0)
#define M_STDOUT        (1)

#define M_NEW_LINE  0x0d
#define M_RETURN    0x0a
#define M_SPACE     0x20
#define M_TAB       0x09
#define M_PIPE_SYM  0x7c

/* pre-defined commands */
#define M_CMDSTR_EXIT   "exit"
#define M_CMDSTR_QUIT   "quit"
#define M_CMDSTR_PATH   "path"
#define M_CMDSTR_DEBUG  "debug"
#define M_CMDSTR_CD     "cd"
#define M_CMDSTR_TEST   "testing"

#define M_ERR(...) \
		do { \
			fprintf(stderr, __VA_ARGS__); \
			fprintf(stderr, "\n"); \
		} while (0)

#define DMSG(...) \
	do { \
		extern e_bool g_debug; \
		FILE *dbg; \
		if (g_debug) { \
			dbg = fopen(M_DBG_LOG, "a"); \
			if (dbg != NULL) { \
				fprintf(dbg, __VA_ARGS__); \
				fprintf( \
					dbg, \
					" @ %s, %d\n", __FILE__, __LINE__); \
				fclose(dbg); \
			} \
		} \
	} while (0)

#define M_SYS_ALLOC(s) \
		sys_alloc(s, __FILE__, __LINE__)

#define M_SYS_FREE(p) \
		sys_free(p, __FILE__, __LINE__)

#define M_SYS_FORK() \
		sys_fork(__FILE__, __LINE__)

#define M_SYS_EXECV(p, a) \
		sys_execv(p, a, __FILE__, __LINE__)

#define M_SYS_PIPE(fd) \
		sys_pipe(fd, __FILE__, __LINE__)

#define M_SYS_DUP(fd) \
		sys_dup(fd, __FILE__, __LINE__)

#define M_SYS_CLOSE(fd) \
		sys_close(fd, __FILE__, __LINE__)


/******************************************************************************/
TYPE struct s_cmd {
	char *mp_cmd;
	unsigned int m_len;
	unsigned int m_curLimit;
} s_cmd;

TYPE enum e_bool {
	e_true = 1,
	e_false = 0
} e_bool;

TYPE enum e_err_code {
	e_err_ok,
	e_err_quit,
	e_err_fork_failed,
	e_err_out_of_memory,
	e_err_unknown_err,
	e_err_not_a_shell_cmd,
	e_err_task_running,
	e_err_sys_call_error,
	e_err_shell_cmd_with_pipe,
	e_err_cmd_not_found,
	e_err_exceed_max_param
} e_err_code;

#endif
