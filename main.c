#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <signal.h>

const long ONE_SECOND = 3e8;

// 1 - rapido
// 2 - medio
// 3 - demorado

int fd[4][2];
int ta[4][2];
int work_stealing_mode = 0;
int total_time = 0;

void execute_process(char process_type) {
  int seconds = 0;
  if (process_type == '1') seconds = 5;
  else if (process_type == '2') seconds = 10;
  else if (process_type == '3') seconds = 20;

  for (long j = 0; j < seconds * ONE_SECOND; j++) {} // simula o processo (prefiro com fork())

  total_time += seconds;
  printf("Esperei %d segundos\n", seconds);
}

// entra no modo roubo de trabalho
static void handle_signusr1(int sig, siginfo_t *siginfo, void* context) {
  // printf("I am work stealing %d\n", getpid());
  
  for (int i = 0; i < 4; i++) {
    char steal = '0';
    char process_type = '4';
    while (process_type != '0') {
      write(fd[i][1], &steal, sizeof(char));
      read(fd[i][0], &process_type, sizeof(char));
      // printf("Process type %c\n", process_type);

      if (process_type != '0') {
        execute_process(process_type);
      }
    }
  }

}

// não entra no modo roubo de trabalho
static void handle_signusr2(int sig, siginfo_t *siginfo, void* context) {
  // printf("I am not work stealing %d\n", getpid());
}

int main(int argc, char *argv[]) {
  if (argc > 1) {
    // -work-stealing
    work_stealing_mode = 1;
    // printf("You passed the flag: %s\n", argv[1]);
  }

  // 0 - leitura
  // 1 - escrita
  int auxiliar_process_id = 0;
  int work_steal[2];
  pipe(work_steal);

  for (int i = 0; i < 4; i++) {
    pipe(ta[i]);
    if (pipe(fd[i]) == -1) {
      printf("Could not create pipe\n");
      return 1;
    };
  }

  for (int i = 0; i < 4; i++) {
    int id = fork();
    if (id == 0) {
      struct sigaction sa = { 0 };
      sa.sa_sigaction = *handle_signusr1;
      sa.sa_flags |= SA_SIGINFO;
      sigaction(SIGUSR1, &sa, NULL);

      struct sigaction sa2 = { 0 };
      sa2.sa_sigaction = *handle_signusr2;
      sa2.sa_flags |= SA_SIGINFO;
      sigaction(SIGUSR2, &sa2, NULL);

      int n_processes;
      read(fd[auxiliar_process_id][0], &n_processes, sizeof(int));

      char process_type;
      for (int j = 0; j < n_processes; j++) {
        read(fd[auxiliar_process_id][0], &process_type, sizeof(char));
        if (process_type == '0') continue;
        execute_process(process_type);
      }

      // printf("%d finalizou seus processos.\n", auxiliar_process_id);
      
      if (work_stealing_mode) {
        int pid = getpid();
        write(work_steal[1], &pid, sizeof(int));
        pause();

        close(work_steal[0]);
        close(work_steal[1]);
      }

      write(ta[i][1], &total_time, sizeof(int));

      for (int j = 0; j < 4; j++) {
        close(fd[j][0]);
        close(fd[j][1]);
        close(ta[j][0]);
        close(ta[j][1]);
      }

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

  if (work_stealing_mode) {
    int finished_processes = 0;
    for (int i = 0;  i < 4; i++) {
      int id;
      read(work_steal[0], &id, sizeof(int));
      finished_processes++;
      if (finished_processes < 4) {
        // printf("I will sent %d into work stealing.\n", id);
        kill(id, SIGUSR1);
      } else {
        kill(id, SIGUSR2);
      }  
    }

    close(work_steal[0]);
    close(work_steal[1]);
  }

  for (int i = 0; i < 4; i++) {
    close(fd[i][1]);
  }
  close(file);

  int turnaround = 0;
  for (int i = 0; i < 4; i++) {
    read(ta[i][0], &total_time, sizeof(int));
    printf("O processo auxiliar %d levou %d segundos\n", i, total_time);
    turnaround += total_time;
    close(ta[i][0]);
    close(ta[i][1]);
  }

  wait(NULL);
  wait(NULL);
  wait(NULL);
  wait(NULL);

  printf("O tempo de turnaround é: %d\n", turnaround);

  return 0;
}
