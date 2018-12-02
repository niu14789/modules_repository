/** 
*/  
#include "stdio.h"
#include "fs.h"

int main(void);

FS_REGISTER_ENTRANCE(main,__FS_NORMAL);

int main(void)
{	
  system_initialization();

	return (int)inode_sched_getfiles();
}
















