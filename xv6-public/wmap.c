#include "wmap.h"
#include "defs.h"

int 
sys_wmap(void) 
{
  uint addr;
  int length;
  int flags;
  int fd;
  if (argint(0, &addr) < 0 || argint(1, &length) < 0 || argint(2, &flags) < 0 || argint(3, &fd) < 0) {
    return -1;
  }
}

int 
sys_wunmap(void) 
{
  uint addr;
  if (argint(0, &addr) < 0) {
    return -1;
  }
}

int 
sys_wremap(void) 
{
  uint oldaddr;
  int oldsize;
  int newsize;
  int flags;
  if (argint(0, &oldaddr) < 0 || argint(1, &oldsize) < 0 || argint(2, &newsize) < 0 || argint(3, &flags) < 0) {
    return -1;
  }
}
int 
sys_getwmapinfo(void)
{
  struct pgdirinfo *pdinfo;
  if (argptr(0, &pdinfo, sizeof(struct pgdirinfo)) < 0) {
    return -1;
  }
}

int 
sys_getpgdirinfo(void)
{
  struct wmapinfo *wminfo;
  if (argptr(0, &wminfo, sizeof(struct wmapinfo)) < 0) {
    return -1;
  }
}