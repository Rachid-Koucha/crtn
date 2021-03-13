
#include <stdio.h>

#include "crtn.h"



int entry(void *p)
{
  (void)p;

  while (1) {
    printf("Coroutine#%d is running\n", crtn_self());
    crtn_yield(0);
  }

  return crtn_self();

} // entry


int main(void)
{
  crtn_t cid[3];
  int    status;
  int    rc;
  int    i;
  char   name[CRTN_NAME_SZ];

  printf("CRTN tests\n");

  for (i = 0; i < 3; i ++) {
    snprintf(name, CRTN_NAME_SZ, "crtn_%02d", i);
    rc = crtn_spawn(&(cid[i]), name, entry, 0, 0);
    printf("crtn_%02d, rc = %d, cid=%d\n", i, rc, cid[i]);
  }

  printf("Spawned the coroutines\n");

  for (i = 0; i < 3; i ++) {
    rc = crtn_cancel(cid[i]);
    printf("Cancelled coroutine#%d, rc = %d\n", cid[i], rc);
  }

  for (i = 0; i < 3; i ++) {
    rc = crtn_join(cid[i], &status);
    printf("Joined coroutine#%d, rc = %d, status = %d\n", cid[i], rc, status);
  }

  printf("Joined the coroutines\n");

  return 0;
} // main
