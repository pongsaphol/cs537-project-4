#include "wmap.h"
#include "defs.h"
#include "mmu.h"
#include "param.h"
#include "memlayout.h"
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
      break;
    }
  }
  return (int)addr;
}

int real_wunmap(uint addr) {
  return 0;
}

int 
sys_wunmap(void) 
{
  uint addr;
  if (argint(0, (int*)&addr) < 0) {
    return -1;
  }
  return real_wunmap(addr);
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
  struct wmapinfo *wminfo;
  if (argptr(0, (char**)&wminfo, sizeof(struct wmapinfo)) < 0) {
    return -1;
  }
  wminfo->total_mmaps = 0;
  for (int i = 0; i < MAX_MEMMAPS; ++i) {
    if (myproc()->memmaps[i].used == 1) {
      wminfo->addr[wminfo->total_mmaps] = myproc()->memmaps[i].base;
      wminfo->length[wminfo->total_mmaps] = myproc()->memmaps[i].length;
      wminfo->n_loaded_pages[wminfo->total_mmaps] = 0;
      for (int base = myproc()->memmaps[i].base; base < myproc()->memmaps[i].base + myproc()->memmaps[i].length; base += PGSIZE) {
        pte_t* pte = walkpgdir(myproc()->pgdir, (void*)base, 0);
        if (pte && (*pte & PTE_P)) {
          wminfo->n_loaded_pages[wminfo->total_mmaps]++;
        }
      }
      wminfo->total_mmaps++;
    }
  }
  return 0;
}

int 
sys_getpgdirinfo(void)
{
  struct pgdirinfo *pdinfo;
  if (argptr(0, (char**)&pdinfo, sizeof(struct pgdirinfo)) < 0) {
    return -1;
  }
  pdinfo->n_upages = 0;
  pde_t* pgdir = myproc()->pgdir;
  for (int i = 0; i < 0x100; ++i) {
    if (pgdir[i] & PTE_P) {
      pte_t *pgtab;
      pgtab = (pte_t*)P2V(PTE_ADDR(pgdir[i]));
      for (int j = 0; j < NPTENTRIES; ++j) {
        if (pgtab[j] & PTE_P) {
          if (pdinfo->n_upages < MAX_UPAGE_INFO) {
            pdinfo->va[pdinfo->n_upages] = PGADDR(i, j, 0);
            pdinfo->pa[pdinfo->n_upages] = PTE_ADDR(pgtab[j]);
          }
          pdinfo->n_upages++;
        }
      }
    }
  }
  return 0;
}
