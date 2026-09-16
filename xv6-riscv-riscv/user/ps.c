#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/spinlock.h"
#include "kernel/param.h"
#include "kernel/riscv.h"

#include "kernel/proc.h"







int main(void) {

  struct  pstat st;    // Hold information
  // avoid other data
   memset(&st, 0, sizeof(st));

  
  // Get Information
  if (getpinfo(&st) < 0) {
    printf("ERROR\n"); 
    return -1;  //ERROR
  }
   // All process
  for (int p = 0; p < NPROC; p++) {
   
    if (st.use[p]) {    // process is in use
      
      printf("%d\t%d\t%d\t%s\t\t%d\t%s\n",
             st.pid[p], st.ppid[p], st.priority[p], st.state[p], st.size[p], st.name[p]);
    }
  }

  return 0;   // SUCESS
}