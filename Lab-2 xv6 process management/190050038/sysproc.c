#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "processInfo.h"

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



// prints hello onto console
// returns 0 after successfullt printing
int
sys_hello(void)
{
  // how to print onto console?
  cprintf("Hello\n");
  return 0;
}




// prints name and hello onto console
// returns 0 after successfullt printing
int
sys_helloYou(void)
{
  // how to print onto console?
  char* name;
  if(argstr(0, &name) < 0)
    return -1;

  cprintf("%s\n", name);
  return 0;
}


// returns the total number of active processes
int
sys_getNumProc(void){
  return getNumProc();
}

// returns the max pid among all currently active process
int
sys_getMaxPid(void){
  return getMaxPid();
}


// returns process info for a process with given PID
int
sys_getProcInfo(void){
  // these 2 are user arguments na!!!
  int pid;
  struct processInfo* pInfo;

  if (argint(0, &pid) < 0 || argptr(1, (void *)&pInfo, sizeof(*pInfo)) < 0)
    return -1;

  return getProcInfo(pid, pInfo);

}

// Pass a function pointer. The function should terminate with welcomeDone() syscall
int
sys_welcomeFunction(void){
  void* func;
  int n;
  struct proc* curproc = myproc();

  if (argint(0, &n) < 0)
    return -1;
  if (n >= curproc->sz)
    return -1;
    
  func = (void*) n;

  return welcomeFunction(func);
}

// Call this syscall at the end of user welcome function
int
sys_welcomeDone(void)
{
  return welcomeDone();
}
