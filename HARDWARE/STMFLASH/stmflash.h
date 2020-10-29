#ifndef __STMFLASH_H__
#define __STMFLASH_H__
#include "sys.h"  
#include "lfs.h"



//////////////////////////////////////////////////////////////////////////////////////////////////////
//�û������Լ�����Ҫ����
#define STM32_FLASH_SIZE 512 	 		//��ѡSTM32��FLASH������С(��λΪK)
#define STM32_FLASH_WREN 1              //ʹ��FLASHд��(0��������;1��ʹ��)
//////////////////////////////////////////////////////////////////////////////////////////////////////


#define STM32Flash_ERASE_GRAN              2048
#define STM32Flash_NUM_GRAN                256    //�ܵ�Page����


//FLASH��ʼ��ַ
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH����ʼ��ַ
//FLASH������ֵ

//ʹ��һ��ռ���Ϊϵͳ�ļ���Դ
#define STM32_FLASH_FLLESYS_START_BASE   (0x08000000 + STM32Flash_ERASE_GRAN*128)

uint8_t STMFLASH_ReadHalfWord(u32 faddr);		  //��������  
void STMFLASH_WriteLenByte(u32 WriteAddr,u32 DataToWrite,u16 Len);	//ָ����ַ��ʼд��ָ�����ȵ�����
u32 STMFLASH_ReadLenByte(u32 ReadAddr,u16 Len);						//ָ����ַ��ʼ��ȡָ����������
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);		//��ָ����ַ��ʼд��ָ�����ȵ�����
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);   		//��ָ����ַ��ʼ����ָ�����ȵ�����

//����д��
void Test_Write(u32 WriteAddr,u16 WriteData);

//lfs interface
int stm32flash_readLittlefs(const struct lfs_config *c, lfs_block_t block,
				lfs_off_t off, void *buffer, lfs_size_t size);

int stm32flash_writeLittlefs(const struct lfs_config *c, lfs_block_t block,
				lfs_off_t off,void *buffer, lfs_size_t size);

int stm32flash_eraseLittlefs(const struct lfs_config *c, lfs_block_t block);


int stm32flash_syncLittlefs(const struct lfs_config *c );


#endif

