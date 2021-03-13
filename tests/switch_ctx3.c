#include <stdio.h>
#include "crtn.h"


int func1(void *param)
{
  int value1 = 1;

  (void)param;

  while(1) {
    printf("func1 is running, value1@%p = %d\n", &value1, value1);
    crtn_yield(0);
  }
}

int func2(void *param)
{
  int value2 = 2;

  (void)param;

  while(1) {
    printf("func2 is running, value2@%p = %d\n", &value2, value2);
    crtn_yield(0);
  }
}

int main(void)
{
  int c = 0;
  crtn_t cid1, cid2;
  crtn_attr_t attr;
  int rc;

  attr = crtn_attr_new();
  rc = crtn_set_attr_type(attr, CRTN_TYPE_STEPPER|CRTN_TYPE_STACKFUL);
  if (rc != 0) {
    return 1;
  }

  rc = crtn_spawn(&cid1, "func1", func1, 0, attr);
  if (rc != 0) {
    return 1;
  }
  rc = crtn_spawn(&cid2, "func2", func2, 0, attr);
  if (rc != 0) {
    return 1;
  }

  rc = crtn_attr_delete(attr);
  if (rc != 0) {
    return 1;
  }

  while(1) {
    if (getchar() == EOF) {
      printf("Exiting\n");
      break;
    }
    c = (c + 1) % 2;
    if (1 == c) {
      rc = crtn_wait(cid1, 0);
    } else {
      rc = crtn_wait(cid2, 0);
    }
    if (rc != 0) {
      return 1;
    }
  }

  rc = crtn_cancel(cid1);
  if (rc != 0) {
    return 1;
  }
  rc = crtn_cancel(cid2);
  if (rc != 0) {
    return 1;
  }

  rc = crtn_join(cid1, 0);
  if (rc != 0) {
    return 1;
  }
  rc = crtn_join(cid2, 0);
  if (rc != 0) {
    return 1;
  }

  return 0;
}
