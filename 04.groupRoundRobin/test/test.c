#define NEXUS_7
#define V1 vola
#define V2 tile
#define E1 ex
#define E2 tern
#define CAT(X, Y) X ## Y

#ifdef NEXUS_7
#define _GNU_SOURCE
#include <sys/syscall.h>
#include <linux/sched.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sched.h>
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define N_TEST_LOOP     20
#define FORK_CNT        200

#ifdef NEXUS_7
CAT(ex, tern) int set_sched_policy(int tid, int policy);
#endif

int system_cgroup_fd = 0;
int fg_cgroup_fd = 0;
int bg_cgroup_fd = 0;
int TEST_COUNT;
int N_TEST_TASK;

static void __initialize(void)
{
#ifdef NEXUS_7
	char *filename;

	if (!access("/dev/cpuctl/tasks", F_OK)) {
		filename = "/dev/cpuctl/tasks";
		system_cgroup_fd = open(filename, O_WRONLY | O_CLOEXEC);
		if (system_cgroup_fd < 0)
			fprintf(stderr, "open of %s failed: %s\n",
			filename, strerror(errno));

		filename = "/dev/cpuctl/apps/tasks";
		fg_cgroup_fd = open(filename, O_WRONLY | O_CLOEXEC);

		if (fg_cgroup_fd < 0)
			fprintf(stderr, "open of %s failed: %s\n",
				filename, strerror(errno));

		filename = "/dev/cpuctl/apps/bg_non_interactive/tasks";
		bg_cgroup_fd = open(filename, O_WRONLY | O_CLOEXEC);

		if (bg_cgroup_fd < 0)
			fprintf(stderr, "open of %s failed: %s\n",
					filename, strerror(errno));
	}
#endif
}

/* Add tid to the scheduling group defined by the policy */
#ifdef NEXUS_7
static int add_tid_to_cgroup(int tid, int policy)
{
	int fd;
	char text[22];
	char *end = text + sizeof(text) - 1;
	char *ptr = end;

	switch (policy) {
	case 0:
		fd = bg_cgroup_fd;
	break;
	case 1:
		fd = fg_cgroup_fd;
	break;
	case 2:
		fd = system_cgroup_fd;
	break;
	default:
		fd = -1;
	break;
	}

	if (fd < 0)
		return -1;

	*ptr = '\0';
	while (tid > 0) {
		*--ptr = '0' + (tid % 10);
		tid = tid / 10;
	}

	if (write(fd, ptr, end - ptr) < 0) {
		/*
		* If the thread is in the process of exiting,
		* don't flag an error
		*/
		if (errno == ESRCH)
			return 0;
		fprintf(stderr,
			"add_tid_to_cgroup failed to write '%s' (%s); policy=%d\n",
			ptr, strerror(errno), policy);
		return -1;
	}
	return 0;
}
#endif

static int get_cpu_id(void)
{
#ifdef NEXUS_7
	unsigned cpu_id;

	if (syscall(__NR_getcpu, &cpu_id, NULL, NULL) < 0)
		return -1;

	return (int) cpu_id;
#else
	return 0;
#endif
}

static inline void test_print(int fork_count)
{
#ifdef NEXUS_7
	printf(
		"task %d: run %d using %d @ cpu %d\n",
		getpid(),
		fork_count,
		sched_getscheduler(0),
		get_cpu_id());
#else
	printf(
		"task %d: run %d @ cpu %d\n",
		getpid(),
		fork_count,
		get_cpu_id());
#endif
}

static void create_new_task(int fork_count)
{
	pid_t pid;

#ifdef NEXUS_7
	struct sched_param param;
	long ret;
#endif

	CAT(vol, atile) unsigned long count = 0;
	CAT(vol, atile) unsigned int j;

	fork_count++;

	pid = fork();

	if (pid == 0) {
		srand(time(NULL));

#ifdef NEXUS_7
		/* change scheduler */
		if (rand() % 2 == 1) {
			if (sched_getscheduler(getpid()) == 6) {
				param.sched_priority = 0;
				ret = sched_setscheduler(
					getpid(), SCHED_OTHER, &param);

			if (ret != 0)
				fprintf(stderr, "set err for %d: %s\n",
						getpid(), strerror(errno));
			} else {
				param.sched_priority = 50;
				ret = sched_setscheduler(getpid(), 6, &param);

			if (ret != 0)
				fprintf(stderr, "set err for %d: %s\n",
						getpid(), strerror(errno));
			}
		}
#endif

#if 0
		/* change affiliation */
		if (rand() % 2 == 1) {
			CPU_ZERO(&mask);
			CPU_SET(rand() % 4, &mask);

			if (sched_setaffinity(
				getpid(), sizeof(cpu_set_t), &mask)
				!= 0)
				fprintf(stderr, "Change affiliation failed!\n");
		}
#endif


#ifdef NEXUS_7
		/* change CPU-group */
		if (!syscall(378, rand()%4, rand()%2+1))
			fprintf(stderr, "failed to set CPU group.\n");

		/* change c-group */
		if (rand() % 2 == 1) {
			if (add_tid_to_cgroup(getpid(), rand() % 3) != 0)
				fprintf(stderr, "failed to set task group.\n");
		}
#endif

		/* run for a while */
		for (j = 0; j < TEST_COUNT; j++)
			count++;

		usleep(rand() % 100 * 10000 + 1000);

		for (j = 0; j < TEST_COUNT; j++)
			count--;

		test_print(fork_count);
		usleep(rand() % 100 * 10000 + 1000);

		if (fork_count < FORK_CNT)
			create_new_task(fork_count);

		exit(0);

		} else if (pid < 0) {
			fprintf(stderr, "Fork failed.\n");
		}
	}

int main(int argc, char **argv)
{
	int i;
	int fork_count = 0;

	if (argc == 3) {
		N_TEST_TASK = atoi(argv[1]);
		TEST_COUNT = atoi(argv[2]);
	} else{
		N_TEST_TASK = 10;
		TEST_COUNT = 2000000;
	}

#ifdef NEXUS_7
	struct sched_param param = {.sched_priority = 50};
#endif

	__initialize();

#ifdef NEXUS_7
	if (sched_setscheduler(getpid(), 6, &param) < 0) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		return -1;
	}
#endif

	for (i = 0; i < N_TEST_TASK; ++i)
		create_new_task(fork_count);

	return 0;
}
