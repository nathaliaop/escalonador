#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <signal.h>

const long ONE_SECOND = 6e8;

// 1 - rapido
// 2 - medio
// 3 - demorado

int main(int argc, char *argv[]) {
	// 0 - leitura
	// 1 - escrita
	int fd[4][2];
	int auxiliar_process_id = 0;

	for (int i = 0; i < 4; i++) {
		if (pipe(fd[i]) == -1) {
			printf("Could not create pipe\n");
			return 1;
		};
	}

	for (int i = 0; i < 4; i++) {
		int id = fork();
		if (id == 0) {
			char process_type;

			for (int j = 0; j < 4; j++) {
				close(fd[j][1]);
			}
			
			// if (amount_written[auxiliar_process_id] > 0) {
				// amount_written[auxiliar_process_id]--;
				read(fd[auxiliar_process_id][0], &process_type, sizeof(char));
				// printf("Aqui: %c\n", process_type);
				for (long j = 0; j < 5 * ONE_SECOND; j++) {}
				printf("Esperei 5 segundos\n");
			// }

			for (int j = 0; j < 4; j++) {
				close(fd[j][0]);
			}

			return 0;
		}
		auxiliar_process_id++;
	}

	for (int i = 0; i < 4; i++) {
		close(fd[i][0]);
	}

	int file = open("process.txt", O_RDONLY);
	if (file == -1) {
		printf("Could not open file\n");
		return 2;
	}

	char process_type;
	int curr_id = 0;

	while(read(file, &process_type, 1) != 0) {
		printf("%c", process_type);

		write(fd[curr_id % 4][1], &process_type, sizeof(char));
		
		curr_id++;
	}

	for (int i = 0; i < 4; i++) {
		close(fd[i][1]);
	}

	close(file);

	wait(NULL);
	wait(NULL);
	wait(NULL);
	wait(NULL);

	return 0;
}