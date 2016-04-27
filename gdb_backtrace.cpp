// from http://stackoverflow.com/questions/4636456/how-to-get-a-stack-trace-for-c-using-gcc-with-line-number-information
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void gdb_print_trace()
{
  char pid_buf[30];
  sprintf(pid_buf, "%d", getpid());
  char name_buf[512];
  name_buf[readlink("/proc/self/exe", name_buf, 511)]=0;
  int child_pid = fork();
  if (!child_pid) {
      dup2(2,1); // redirect output to stderr
      fprintf(stdout,"stack trace for %s pid=%s\n",name_buf,pid_buf);
      // execlp("gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "bt", name_buf, pid_buf, NULL);
      execlp("gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "bt full", name_buf, pid_buf, NULL);
      abort(); /* If gdb failed to start */
  } else {
      waitpid(child_pid,NULL,0);
  }
}
