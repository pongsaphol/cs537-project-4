#include "wmap.h"
#include "defs.h"
#include "mmu.h"
#include "param.h"
#include "proc.h"


int 
sys_wmap(void) 
{
  uint addr;
  int length;
  int flags;
  int fd;
  if (argint(0, (int*)&addr) < 0 || argint(1, &length) < 0 || argint(2, &flags) < 0 || argint(3, &fd) < 0) {
    return -1;
  }
  struct proc* p = myproc();
  if (!(flags & MAP_FIXED)) {
    // add init addr

  }
  for (int i = 0; i < MAX_MEMMAPS; ++i) {
    if (p->memmaps[i].used == 0) {
      p->memmaps[i].used = 1;
      p->memmaps[i].base = addr;
      p->memmaps[i].length = length;
      p->memmaps[i].flags = flags;
      p->memmaps[i].fd = fd;
    }
  }
  return (int)addr;
}

int 
sys_wunmap(void) 
{
  uint addr;
  if (argint(0, (int*)&addr) < 0) {
    return -1;
  }
  return 0;
}

int 
sys_wremap(void) 
{
  uint oldaddr;
  int oldsize;
  int newsize;
  int flags;
  if (argint(0, (int*)&oldaddr) < 0 || argint(1, &oldsize) < 0 || argint(2, &newsize) < 0 || argint(3, &flags) < 0) {
    return -1;
  }
  return 0;
}

int 
sys_getwmapinfo(void)
{
  struct pgdirinfo *pdinfo;
  if (argptr(0, (char**)&pdinfo, sizeof(struct pgdirinfo)) < 0) {
    return -1;
  }
  return 0;
}

int 
sys_getpgdirinfo(void)
{
  struct wmapinfo *wminfo;
  if (argptr(0, (char**)&wminfo, sizeof(struct wmapinfo)) < 0) {
    return -1;
  }
  return 0;
}
