#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "crtn.h"

static void hdl_segv(int sig, siginfo_t *info, void *p)
{
  int local_var;

  (void)info;
  (void)p;
  printf("sig#%d, stack@%p\n", sig, &local_var);
}


int func1(void *param)
{
  int value1;
  char *p = 0;

  (void)param;

  value1 = 4;
  while(1) {
    // Crash
    if (value1 == 2) {
      *p = (*(char *)param);
    }
    printf("func1 is running, value1@%p = %d\n", &value1, value1);
    crtn_yield(0);
    value1 ++;
  }
}

int func2(void *param)
{
  int value2;
  char *p = 0;

  (void)param;

  value2 = 2;
  while(1) {
    // Crash
    if (value2 == 3) {
      *p = (*(char *)param);
    }
    printf("func2 is running, value2@%p = %d\n", &value2, value2);
    crtn_yield(0);
    value2++;
  }
}

int main(void)
{
  int c = 0;
  crtn_t cid1, cid2;
  crtn_attr_t attr;
  struct sigaction action;

  printf("Main is running, stack@%p\n", &c);

  memset(&action, 0, sizeof(action));
  action.sa_sigaction = hdl_segv;
  sigemptyset(&(action.sa_mask));
  sigaddset(&(action.sa_mask), SIGSEGV);
  action.sa_flags |= SA_SIGINFO;
  sigaction(SIGSEGV, &action, 0);

  attr = crtn_attr_new();
  crtn_set_attr_type(attr, CRTN_TYPE_STEPPER|CRTN_TYPE_STACKFUL);

  crtn_spawn(&cid1, "func1", func1, 0, attr);
  crtn_spawn(&cid2, "func2", func2, 0, attr);

  crtn_attr_delete(attr);

  while(1) {
    if (getchar() == EOF) {
      printf("Exiting\n");
      break;
    }
    c = (c + 1) % 2;
    if (1 == c) {
      crtn_wait(cid1, 0);
    } else {
      crtn_wait(cid2, 0);
    }
  }

  crtn_cancel(cid1);
  crtn_cancel(cid2);

  crtn_join(cid1, 0);
  crtn_join(cid2, 0);

  return 0;
}
