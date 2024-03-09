#include "wmap.h"
#include "defs.h"
#include "mmu.h"
#include "param.h"
#include "memlayout.h"
#include "proc.h"
#include "types.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"


int 
sys_wmap(void) 
{
  uint addr;
  int length;
  int flags;
  if (argint(0, (int*)&addr) < 0 || argint(1, &length) < 0 || argint(2, &flags) < 0) {
    return FAILED;
  }

  struct file *f = 0;
  if (!(flags & MAP_ANONYMOUS)) {
    if (argfd(3, 0, &f) < 0) {
      return FAILED;
    }
  }

  struct proc* p = myproc();
  if (!(flags & MAP_FIXED)) {
    addr = 0;
    // add init addr
    for (uint st = 0x60000000; st + length < 0x80000000; st += PGSIZE) {
      int found = 0;
      for (int i = 0; i < MAX_MEMMAPS; ++i) {
        if (p->memmaps[i] && ( 
          (p->memmaps[i]->base <= st && st < p->memmaps[i]->base + p->memmaps[i]->length) ||
          (p->memmaps[i]->base <= st + length - 1 && st + length - 1 < p->memmaps[i]->base + p->memmaps[i]->length) ||
          (st <= p->memmaps[i]->base && p->memmaps[i]->base < st + length) ||
          (st <= p->memmaps[i]->base + p->memmaps[i]->length - 1 && p->memmaps[i]->base + p->memmaps[i]->length - 1 < st + length)
        )) {
          found = 1;
          break;
        }
      }
      if (found == 0) {
        addr = st;
        break;
      }
    } 
    if (addr == 0) {
      return FAILED;
    }
  }

  for (int i = 0; i < MAX_MEMMAPS; ++i) {
    if (p->memmaps[i] && p->memmaps[i]->base <= addr && addr < p->memmaps[i]->base + p->memmaps[i]->length) {
      return FAILED;
    }
  }

  for (int i = 0; i < MAX_MEMMAPS; ++i) {
    if (!p->memmaps[i]) {
      p->memmaps[i] = (struct memmap*)kalloc(); 
      if (flags & MAP_ANONYMOUS) {
        p->memmaps[i]->f = 0;
      } else {
        p->memmaps[i]->f = f;
      }
      p->memmaps[i]->base = addr;
      p->memmaps[i]->length = length;
      p->memmaps[i]->flags = flags;
      p->memmaps[i]->ref = 1;
      break;
    }
  }
  return (int)addr;
}

int real_wunmap(uint addr) {
  for (int i = 0; i < MAX_MEMMAPS; ++i) {
    if (myproc()->memmaps[i] && myproc()->memmaps[i]->base == addr) {
      int status = --myproc()->memmaps[i]->ref;
      if (!status && myproc()->memmaps[i]->f) {
        // map file back
      } 
      uint a = PGROUNDDOWN(addr);
      uint end = PGROUNDDOWN(addr + myproc()->memmaps[i]->length - 1);
      for (uint base = a; base <= end; base += PGSIZE) {
        pte_t *pte = walkpgdir(myproc()->pgdir, (void*)base, 0);
        if (pte && (*pte & PTE_P)) {
          if (!status) {
            if (myproc()->memmaps[i]->f && (myproc()->memmaps[i]->flags & MAP_SHARED)) {
              struct file *f = myproc()->memmaps[i]->f;
              f->off = base - myproc()->memmaps[i]->base;
              uint size = PGSIZE;
              if (base == end) {
                size = myproc()->memmaps[i]->base + myproc()->memmaps[i]->length - base;
              }
              filewrite(f, (char*)base, size);
            }
            kfree((char*)P2V(PTE_ADDR(*pte)));
          }
          *pte = 0;
        }
      }
      if (!status) {
        kfree((char*)myproc()->memmaps[i]);
      }
      myproc()->memmaps[i] = 0;
      return 0;
    }
  }
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
    if (myproc()->memmaps[i]) {
      wminfo->addr[wminfo->total_mmaps] = myproc()->memmaps[i]->base;
      wminfo->length[wminfo->total_mmaps] = myproc()->memmaps[i]->length;
      wminfo->n_loaded_pages[wminfo->total_mmaps] = 0;
      for (int base = myproc()->memmaps[i]->base; base < myproc()->memmaps[i]->base + myproc()->memmaps[i]->length; base += PGSIZE) {
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
  for (int i = 0; i < 0x200; ++i) {
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
