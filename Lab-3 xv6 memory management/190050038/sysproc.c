#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


// Return the number of virtual/logical pages in uvm.
// Equals sz divided by PGSIZE. Counting the stack guard page as well.
int
sys_numvp(void)
{
  return numvp();
}

// Return the number of physical pages in uvm.
// Counting the number of present bit set PTEs in pgtable
int
sys_numpp(void)
{
  return numpp();
}


// Only logical pages are alloted. Physical memory is alloted later on-demand.
// The size argument has to be a positive multiple of PGSIZE, else 0 is returned.
// In case of success, the virtual address of the first new page is returned. This is equal to the previous sz of the process.
int
sys_mmap(void){
  int size;
  if (argint(0, &size) < 0){
      return -1;
  }

  return (int)mmap(size);
}
