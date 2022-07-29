#include "stm32f10x.h"                  // Device header
#include "bps_led.h"
#include "bps_usart.h"
#include <stdio.h>
#include <string.h>
#include "bps_flash.h"
#include "ff.h"	

uint32_t n;
FATFS fs;//定义一个变量配合f_mount使用
FATFS *pfs;
FIL fp;//创建一个文件句柄
DIR dir;//配合f_opendir使用
FILINFO fno;//配合f_stat使用

DWORD fre_clust, fre_sect, tot_sect;
uint8_t WriteData[]="\n在原来文件新添加一行内容\n";
UINT bw;
UINT br;
char ReadData[4096];
char fpath[100];                  /* 保存当前扫描路径 */
FRESULT res;




static FRESULT scan_files (char* path) 
{ 
//  FRESULT res; 		//部分在递归过程被修改的变量，不用全局变量	
//  FILINFO fno; 
//  DIR dir; 
  int i;            
  char *fn;        // 文件名	
	
#if _USE_LFN 
  /* 长文件名支持 */
  /* 简体中文需要2个字节保存一个“字”*/
  static char lfn[_MAX_LFN*2 + 1]; 	
  fno.lfname = lfn; 
  fno.lfsize = sizeof(lfn); 
#endif 
  //打开目录
  res = f_opendir(&dir, path); 
  if (res == FR_OK) 
	{ 
    i = strlen(path); 
    for (;;) 
		{ 
      //读取目录下的内容，再读会自动读下一个文件
      res = f_readdir(&dir, &fno); 								
      //为空时表示所有项目读取完毕，跳出
      if (res != FR_OK || fno.fname[0] == 0) break; 	
#if _USE_LFN 
      fn = *fno.lfname ? fno.lfname : fno.fname; 
#else 
      fn = fno.fname; 
#endif 
      //点表示当前目录，跳过			
      if (*fn == '.') continue; 	
      //目录，递归读取      
      if (fno.fattrib & AM_DIR)         
			{ 			
        //合成完整目录名        
        sprintf(&path[i], "/%s", fn); 					
        //递归遍历         
        res = scan_files(path);	
        path[i] = 0;         
        //打开失败，跳出循环        
        if (res != FR_OK) 
					break; 
      } 
			else 
			{ 
				printf("%s/%s\r\n", path, fn);								//输出文件名	
        /* 可以在这里提取特定格式的文件路径 */        
      }//else
    } //for
  } 
  return res; 
}





int main()
{
	FRESULT res=FR_DISK_ERR;
	USART_Config();
	
//	
//	SPI_FLASH_Init();
//	for(i=0;i<20;i++)
//	{
//		Sector_erase(i*4096);
//		WaitnoBUSY();
//	}
	
	printf("******** 这是一个SPI FLASH 文件系统实验 *******\r\n");
	//挂载系统
	res=f_mount(&fs, "1:", 0); //这个程序必须先执行才能执行后面的f_mkfs（）等等函数
	if(res!=FR_OK)
	{
		printf("挂载系统失败，申请重新挂载\r\n");
		f_mkfs ("1:", 0, 0);
		while(f_mount(&fs, "1:", 0)!=0);
		printf("文件系统挂载成功，可以进行测试\r\n");
	}
	else
	{
		printf("文件系统挂载成功，可以进行测试\r\n");
	}
	
	//空间信息
	res = f_getfree("1:", &fre_clust, &pfs);
	tot_sect = (pfs->n_fatent - 2) * pfs->csize;//计算总共扇区数
	fre_sect = fre_clust * pfs->csize;//计算剩余扇区数
	//一个扇区为4Kb字节
	printf("设备总空间数：%d Kb\n 剩余空间数：%d Kb\n",tot_sect*4,fre_sect*4);
	
	printf("\n******** 文件定位和格式化写入功能测试 ********\r\n");
	res=f_open(&fp,"1:FatFs读写测试文件.txt",FA_OPEN_ALWAYS|FA_READ|FA_WRITE);
	if(res==FR_OK)
	{
		//除了第一个参数外 和printf函数用法基本一致
		f_printf(&fp,"\n在原来文件新添加一行内容\n");		
		f_printf(&fp,"》设备总空间：%10lu KB。\n》可用空间：  %10lu KB。\n", tot_sect *4, fre_sect *4);

		f_lseek(&fp,0);//光标偏移到0位置
		res=f_read(&fp,ReadData,f_size(&fp),&br);
		if(res==FR_OK)
		{
			printf("文件内容: \n%s\n",ReadData);
		}
	}
	f_close(&fp);
	
	printf("\n********** 目录创建和重命名功能测试 **********\r\n");
	res=f_opendir(&dir,"1:TestDir");//打开目录（目录就是文件夹）
	if(res!=FR_OK)//打开失败创建一个
	{
		printf("\n没找到该文件夹已经创建一个\r\n");
		res=f_mkdir("1:TestDir");//创建一个目录（文件夹）
	}
	else//待修改
	{
		printf("\n找到了该文件夹\r\n");
		f_closedir(&dir);//关闭文件夹
		res=f_unlink("1:TestDir/testdir.txt");//删除文件
		//目的是为了写一个新的
	}
	if(res==FR_OK)
	{
		//重命名并移动这个文件到TestDir目录里
		f_rename("1:FatFs读写测试文件.txt","1:TestDir/testdir.txt");//使用时文件不能被打开
	printf("成功重命名FatFs读写测试文件.txt为testdir.txt。并移动这个文件到TestDir目录里");		
	}
	else
	{
		printf("打开目录文件失败或覆盖文件失败");
	}
	
	
	
	
  printf("\n*************** 文件信息获取测试 **************\r\n");
	res=f_stat("1:TestDir/testdir.txt",&fno);//获取信息包括（字节，时间戳等）使用时文件不能被打开
	if(res==FR_OK)
	{
		
    printf("“testdir.txt”文件信息：\n");
    printf("》文件大小: %ld(字节)\n", fno.fsize);
    printf("》时间戳: %u/%02u/%02u, %02u:%02u\n",
           (fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31,fno.ftime >> 11, fno.ftime >> 5 & 63);
    printf("》属性: %c%c%c%c%c\n\n",
           (fno.fattrib & AM_DIR) ? 'D' : '-',      // 是一个目录
           (fno.fattrib & AM_RDO) ? 'R' : '-',      // 只读文件
           (fno.fattrib & AM_HID) ? 'H' : '-',      // 隐藏文件
           (fno.fattrib & AM_SYS) ? 'S' : '-',      // 系统文件
           (fno.fattrib & AM_ARC) ? 'A' : '-');     // 档案文件
	}

	

	strcpy(fpath,"1:");
  scan_files(fpath);
  
  
	/* 不再使用文件系统，取消挂载文件系统 */
	f_mount(NULL,"1:",1);

	
	while(1)
	{}
}


////接收触发中断函数
//void DEBUG_USART_IRQHandler(void)
//{
//	uint8_t temp;
//	if((USART1->SR&0x20)==0x20)
//	{
//		temp=USART_ReceiveData(USART1);
//		USART_SendData(USART1,temp);
//		while (!USART_GetFlagStatus(USART1,USART_FLAG_TXE));
//	}
//}	



