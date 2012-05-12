#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

int main() {
	int pipes[9][2];
	int pids[9];
	for(size_t i = 0; i < 9; ++i) {
		pipe(pipes[i]);
		pids[i] = fork();
		if(pids[i] < 0) {
			perror("fork");
			return 1;
		} else if(pids[i] == 0) {
			close(pipes[i][0]);
			int fd_test;
			char file[12] = "INPUT";
			strcat(file, "0");
			char c[2];
			c[0] = (char)(i + 1) + '0';
			c[1] = 0;
			strcat(file, c);
			strcat(file, ".TXT");
			fd_test = open(file, O_RDONLY);
			dup2(fd_test, STDIN_FILENO);
			dup2(pipes[i][1], STDOUT_FILENO);
			char *tmp = NULL;
			execlp("./dijkstra", tmp);
			return 1;
		}
		else {
			close(pipes[i][1]);
		}
	}
	while(wait(NULL) != -1){}
	printf("Checker: dijckstra.\n");
	int fd_ansv;
	for(size_t i = 0; i < 9; ++i) {
		char c_pipe = 0;
		char c_ansv = 0;
		char file[13] = "ANSWER";
		strcat(file, "0");
		char c[2];
		c[0] = (char)(i + 1) + '0';
		c[1] = 0;
		strcat(file, c);
		strcat(file, ".TXT");
		printf("Test %lu: ", i + 1);
		fd_ansv = open(file, O_RDONLY);
		int ok = 1;
		while(read(pipes[i][0], &c_pipe, 1) > 0) {
			if(read(fd_ansv, &c_ansv, 1) <= 0) {
				printf("\033[1;31mFail\033[1;0m\n");
				ok = 0;
				break;
			}
			if(ok != 0 && c_pipe != c_ansv) {
				printf("\033[1;31mFail\033[1;0m\n");
				ok = 0;
				break;
			}
		}
		if(ok != 0 && read(fd_ansv, &c_ansv, 1) > 0) {
			printf("\033[1;31mFail\033[1;0m\n");
			ok = 0;
		}
		close(fd_ansv);
		if(ok == 1)
			printf("\033[1;32mOK\033[1;0m\n");
	}
	return 0;
}
