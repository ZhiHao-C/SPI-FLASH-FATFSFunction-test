#include "stm32f10x.h"                  // Device header
#include "bps_led.h"
#include "bps_usart.h"
#include <stdio.h>
#include <string.h>
#include "bps_flash.h"
#include "ff.h"	

uint32_t n;
FATFS fs;//����һ���������f_mountʹ��
FATFS *pfs;
FIL fp;//����һ���ļ����
DIR dir;//���f_opendirʹ��
FILINFO fno;//���f_statʹ��

DWORD fre_clust, fre_sect, tot_sect;
uint8_t WriteData[]="\n��ԭ���ļ������һ������\n";
UINT bw;
UINT br;
char ReadData[4096];
char fpath[100];                  /* ���浱ǰɨ��·�� */
FRESULT res;




static FRESULT scan_files (char* path) 
{ 
//  FRESULT res; 		//�����ڵݹ���̱��޸ĵı���������ȫ�ֱ���	
//  FILINFO fno; 
//  DIR dir; 
  int i;            
  char *fn;        // �ļ���	
	
#if _USE_LFN 
  /* ���ļ���֧�� */
  /* ����������Ҫ2���ֽڱ���һ�����֡�*/
  static char lfn[_MAX_LFN*2 + 1]; 	
  fno.lfname = lfn; 
  fno.lfsize = sizeof(lfn); 
#endif 
  //��Ŀ¼
  res = f_opendir(&dir, path); 
  if (res == FR_OK) 
	{ 
    i = strlen(path); 
    for (;;) 
		{ 
      //��ȡĿ¼�µ����ݣ��ٶ����Զ�����һ���ļ�
      res = f_readdir(&dir, &fno); 								
      //Ϊ��ʱ��ʾ������Ŀ��ȡ��ϣ�����
      if (res != FR_OK || fno.fname[0] == 0) break; 	
#if _USE_LFN 
      fn = *fno.lfname ? fno.lfname : fno.fname; 
#else 
      fn = fno.fname; 
#endif 
      //���ʾ��ǰĿ¼������			
      if (*fn == '.') continue; 	
      //Ŀ¼���ݹ��ȡ      
      if (fno.fattrib & AM_DIR)         
			{ 			
        //�ϳ�����Ŀ¼��        
        sprintf(&path[i], "/%s", fn); 					
        //�ݹ����         
        res = scan_files(path);	
        path[i] = 0;         
        //��ʧ�ܣ�����ѭ��        
        if (res != FR_OK) 
					break; 
      } 
			else 
			{ 
				printf("%s/%s\r\n", path, fn);								//����ļ���	
        /* ������������ȡ�ض���ʽ���ļ�·�� */        
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
	
	printf("******** ����һ��SPI FLASH �ļ�ϵͳʵ�� *******\r\n");
	//����ϵͳ
	res=f_mount(&fs, "1:", 0); //������������ִ�в���ִ�к����f_mkfs�����ȵȺ���
	if(res!=FR_OK)
	{
		printf("����ϵͳʧ�ܣ��������¹���\r\n");
		f_mkfs ("1:", 0, 0);
		while(f_mount(&fs, "1:", 0)!=0);
		printf("�ļ�ϵͳ���سɹ������Խ��в���\r\n");
	}
	else
	{
		printf("�ļ�ϵͳ���سɹ������Խ��в���\r\n");
	}
	
	//�ռ���Ϣ
	res = f_getfree("1:", &fre_clust, &pfs);
	tot_sect = (pfs->n_fatent - 2) * pfs->csize;//�����ܹ�������
	fre_sect = fre_clust * pfs->csize;//����ʣ��������
	//һ������Ϊ4Kb�ֽ�
	printf("�豸�ܿռ�����%d Kb\n ʣ��ռ�����%d Kb\n",tot_sect*4,fre_sect*4);
	
	printf("\n******** �ļ���λ�͸�ʽ��д�빦�ܲ��� ********\r\n");
	res=f_open(&fp,"1:FatFs��д�����ļ�.txt",FA_OPEN_ALWAYS|FA_READ|FA_WRITE);
	if(res==FR_OK)
	{
		//���˵�һ�������� ��printf�����÷�����һ��
		f_printf(&fp,"\n��ԭ���ļ������һ������\n");		
		f_printf(&fp,"���豸�ܿռ䣺%10lu KB��\n�����ÿռ䣺  %10lu KB��\n", tot_sect *4, fre_sect *4);

		f_lseek(&fp,0);//���ƫ�Ƶ�0λ��
		res=f_read(&fp,ReadData,f_size(&fp),&br);
		if(res==FR_OK)
		{
			printf("�ļ�����: \n%s\n",ReadData);
		}
	}
	f_close(&fp);
	
	printf("\n********** Ŀ¼���������������ܲ��� **********\r\n");
	res=f_opendir(&dir,"1:TestDir");//��Ŀ¼��Ŀ¼�����ļ��У�
	if(res!=FR_OK)//��ʧ�ܴ���һ��
	{
		printf("\nû�ҵ����ļ����Ѿ�����һ��\r\n");
		res=f_mkdir("1:TestDir");//����һ��Ŀ¼���ļ��У�
	}
	else//���޸�
	{
		printf("\n�ҵ��˸��ļ���\r\n");
		f_closedir(&dir);//�ر��ļ���
		res=f_unlink("1:TestDir/testdir.txt");//ɾ���ļ�
		//Ŀ����Ϊ��дһ���µ�
	}
	if(res==FR_OK)
	{
		//���������ƶ�����ļ���TestDirĿ¼��
		f_rename("1:FatFs��д�����ļ�.txt","1:TestDir/testdir.txt");//ʹ��ʱ�ļ����ܱ���
	printf("�ɹ�������FatFs��д�����ļ�.txtΪtestdir.txt�����ƶ�����ļ���TestDirĿ¼��");		
	}
	else
	{
		printf("��Ŀ¼�ļ�ʧ�ܻ򸲸��ļ�ʧ��");
	}
	
	
	
	
  printf("\n*************** �ļ���Ϣ��ȡ���� **************\r\n");
	res=f_stat("1:TestDir/testdir.txt",&fno);//��ȡ��Ϣ�������ֽڣ�ʱ����ȣ�ʹ��ʱ�ļ����ܱ���
	if(res==FR_OK)
	{
		
    printf("��testdir.txt���ļ���Ϣ��\n");
    printf("���ļ���С: %ld(�ֽ�)\n", fno.fsize);
    printf("��ʱ���: %u/%02u/%02u, %02u:%02u\n",
           (fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31,fno.ftime >> 11, fno.ftime >> 5 & 63);
    printf("������: %c%c%c%c%c\n\n",
           (fno.fattrib & AM_DIR) ? 'D' : '-',      // ��һ��Ŀ¼
           (fno.fattrib & AM_RDO) ? 'R' : '-',      // ֻ���ļ�
           (fno.fattrib & AM_HID) ? 'H' : '-',      // �����ļ�
           (fno.fattrib & AM_SYS) ? 'S' : '-',      // ϵͳ�ļ�
           (fno.fattrib & AM_ARC) ? 'A' : '-');     // �����ļ�
	}

	

	strcpy(fpath,"1:");
  scan_files(fpath);
  
  
	/* ����ʹ���ļ�ϵͳ��ȡ�������ļ�ϵͳ */
	f_mount(NULL,"1:",1);

	
	while(1)
	{}
}


////���մ����жϺ���
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



