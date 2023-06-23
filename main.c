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

    if (argc > 1) {
      // -work-stealing
      printf("You passed the flag: %s\n", argv[1]);
    }


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
            for (int j = 0; j < 4; j++) {
                close(fd[j][1]);
            }
            
            int n_processes;
            read(fd[auxiliar_process_id][0], &n_processes, sizeof(int));
            // printf("Quantity of processes: %d\n", n_processes);

            char process_type;
            for (int j = 0; j < n_processes; j++) {
                read(fd[auxiliar_process_id][0], &process_type, sizeof(char));

                int seconds = 0;

                if (process_type == '1') seconds = 5;
                else if (process_type == '2') seconds = 10;
                else if (process_type == '3') seconds = 20;

                for (long j = 0; j < seconds * ONE_SECOND; j++) {} // simula o processo (prefiro com fork())

                printf("Esperei %d segundos\n", seconds);
            }

            for (int j = 0; j < 4; j++) {
                close(fd[j][0]);
            }

            printf("%d finalizou seus processos.\n", auxiliar_process_id);

            return 0;
        }

        auxiliar_process_id++;
    }

    for (int i = 0; i < 4; i++) {
        close(fd[i][0]);
    }

    int file = open("processes.txt", O_RDONLY);
    if (file == -1) {
        printf("Could not open file\n");
        return 2;
    }

    char process_type;
    int curr_id = 0;

    char processes_type[4][512];
    int nxt[4] = {};

    while(read(file, &process_type, 1) != 0) {
        if (process_type == '\n') continue;

        processes_type[curr_id][nxt[curr_id]] = process_type;
        nxt[curr_id]++;
        curr_id = (curr_id + 1) % 4;
    }

    for (int i = 0; i < 4; i++) {
        write(fd[i][1], &nxt[i], sizeof(int));
        
        for (int j = 0; j < nxt[i]; j++) {
            write(fd[i][1], &processes_type[i][j], sizeof(char));
        }
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
