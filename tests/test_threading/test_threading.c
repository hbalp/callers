#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

int Global;

void *Thread1(void *x)
{
  Global = 42;
  return x;
}

int main() {

  pthread_t t1, t2;
  pthread_create(&t1, NULL, Thread1, NULL);
  Global = 43;
  pthread_create(&t2, NULL, Thread1, NULL);
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  return Global;
}

#ifdef __cplusplus
}
#endif

/* Local Variables: */
/* compile-command: "clang++ -fsanitize=thread -g -O1 -o test_threading test_threading.c" */
/* End: */
