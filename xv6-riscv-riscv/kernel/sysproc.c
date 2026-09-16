#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"


extern struct spinlock wait_lock;
extern struct proc proc[NPROC];


uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


uint64 sys_setpriority(void) {    //CW request
    int priority;

    // Retrieve priority value from arguments and check for errors
    argint(0, &priority);
    if ( priority< 0) {
        return -1;  //ERROR
    }

    // Validate the priority range
    if (priority < 1 || priority > 20) {
        return -1;  // ERROR
    }
    
    struct proc *current_process = myproc();    // Get the current process
    acquire(&current_process->lock);  //safe access 

    current_process->priority = priority;   //new priority

    release(&current_process->lock);  // Release the lock 

    return 0; //SUCCESS
}


int sys_getpinfo(void) {
    struct pstat *user_stat;

    
    argaddr(0, (uint64*)&user_stat);
    if( user_stat < 0) { //Velidation
        return -1; // ERROR
    }

    struct pstat kernel_stat;
    memset(&kernel_stat,0, sizeof(struct pstat)); // Initialize

    acquire(&wait_lock); // Acquire global lock for process table access
    for(int i = 0; i < NPROC; i++) {
       struct proc *pa = &proc[i]; // Access process from the process table

        acquire(&pa->lock); // Lock 
        if(pa->state != UNUSED) {  //if process is active ...
          
            kernel_stat.use[i] = 1;
            kernel_stat.pid[i] = pa->pid;
            kernel_stat.ppid[i] = pa->parent ? pa->parent->pid : -1;
            kernel_stat.priority[i] = pa->priority;
            kernel_stat.state[i] = pa->state;
            kernel_stat.size[i] = pa->sz;
            safestrcpy(kernel_stat.name[i], pa->name, sizeof(pa->name));
        }
        release(&pa->lock); // release lock
    }
    release(&wait_lock); 

    // Copy to space
    if(copyout(myproc()->pagetable, (uint64)user_stat, (char *)&kernel_stat, sizeof(struct pstat)) < 0) {
        return -1; // ERROR
    }

    return 0; // Success
}
