#include "stmflash.h"
#include "stdio.h"

//��ȡָ����һ���ֽ�
//faddr:����ַ(�˵�ַ����Ϊ2�ı���!!)
//����ֵ:��Ӧ����.
uint8_t STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(uint8_t*)faddr; 
}

#if STM32_FLASH_WREN	//���ʹ����д   
//������д��
//WriteAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��   
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0;i<NumToWrite;i++)
	{
		FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//��ַ����2.
	}  
} 
//��ָ����ַ��ʼд��ָ�����ȵ�����
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
//pBuffer:����ָ��
//NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else 
#define STM_SECTOR_SIZE	2048
#endif		 
u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//�����2K�ֽ�
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
{
	u32 secpos;	   //������ַ
	u16 secoff;	   //������ƫ�Ƶ�ַ(16λ�ּ���)
	u16 secremain; //������ʣ���ַ(16λ�ּ���)	   
 	u16 i;    
	u32 offaddr;   //ȥ��0X08000000��ĵ�ַ
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return;//�Ƿ���ַ
	FLASH_Unlock();						//����
	offaddr=WriteAddr-STM32_FLASH_BASE;		//ʵ��ƫ�Ƶ�ַ.
	secpos=offaddr/STM_SECTOR_SIZE;			//������ַ  0~127 for STM32F103RBT6
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		//�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
	secremain=STM_SECTOR_SIZE/2-secoff;		//����ʣ��ռ��С   
	if(NumToWrite<=secremain)secremain=NumToWrite;//�����ڸ�������Χ
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//������������������
		for(i=0;i<secremain;i++)//У������
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;//��Ҫ����  	  
		}
		if(i<secremain)//��Ҫ����
		{
			FLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//�����������
			for(i=0;i<secremain;i++)//����
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//д����������  
		}else STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		if(NumToWrite==secremain)break;//д�������
		else//д��δ����
		{
			secpos++;				//������ַ��1
			secoff=0;				//ƫ��λ��Ϊ0 	 
		   	pBuffer+=secremain;  	//ָ��ƫ��
			WriteAddr+=secremain;	//д��ַƫ��	   
		   	NumToWrite-=secremain;	//�ֽ�(16λ)���ݼ�
			if(NumToWrite>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;//��һ����������д����
			else secremain=NumToWrite;//��һ����������д����
		}	 
	};	
	FLASH_Lock();//����
}
#endif

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
		ReadAddr+=2;//ƫ��2���ֽ�.	
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//WriteAddr:��ʼ��ַ
//WriteData:Ҫд�������
void Test_Write(u32 WriteAddr,u16 WriteData)   	
{
	STMFLASH_Write(WriteAddr,&WriteData,1);//д��һ���� 
}




/*
 * @brief littlefs read interface
 * @param [in] c lfs_config���ݽṹ
 * @param [in] block Ҫ���Ŀ�
 * @param [in] off �ڵ�ǰ���ƫ��
 * @param [out] buffer ��ȡ��������
 * @param [in] size Ҫ��ȡ���ֽ���
 * @return 0 �ɹ� <0 ����
 * @note littlefs һ��������ڿ�Խ��洢�����
 */
int stm32flash_readLittlefs(const struct lfs_config *c, lfs_block_t block,
				lfs_off_t off, void *buffer, lfs_size_t size)
{
	uint32_t addr2Read = 0;
	
	if(block >= (W25Q128_NUM_GRAN/2)) //error
	{
		return LFS_ERR_IO;
	}
	
	//��ȡ��ȡ�ĳ�ʼ��ַ
	addr2Read = STM32_FLASH_FLLESYS_START_BASE + block*W25Q128_ERASE_GRAN + off;
	
	//���ݶ�ȡ
	for(int i =0;i < size;i++,addr2Read++)
	{
		buffer[i] = *(uint8_t*)addr2Read; 
	}
	
	
	return LFS_ERR_OK;
				
}



/*
 * @brief littlefs write interface
 * @param [in] c lfs_config���ݽṹ
 * @param [in] block Ҫ���Ŀ�
 * @param [in] off �ڵ�ǰ���ƫ��
 * @param [out] buffer ��ȡ��������
 * @param [in] size Ҫ��ȡ���ֽ���
 * @return 0 �ɹ� <0 ����
 * @note littlefs һ��������ڿ�Խ��洢�����
 */
int stm32flash_writeLittlefs(const struct lfs_config *c, lfs_block_t block,
				lfs_off_t off,void *buffer, lfs_size_t size)
{
	
	if(block >= W25Q128_NUM_GRAN) //error
	{
		return LFS_ERR_IO;
	}
	
	W25QXX_Write_NoCheck(buffer,block*W25Q128_ERASE_GRAN + off, size);
	
	return LFS_ERR_OK;
				
}



/*
 * @brief littlefs ����һ����
 * @param [in] c lfs_config���ݽṹ
 * @param [in] block Ҫ�����Ŀ�
 * @return 0 �ɹ� <0 ����
 */
int stm32flash_eraseLittlefs(const struct lfs_config *c, lfs_block_t block)
{
	
	if(block >= W25Q128_NUM_GRAN) //error
	{
		return LFS_ERR_IO;
	}
	
	//��������
	W25QXX_Erase_Sector(block);
	return  LFS_ERR_OK;

}

int stm32flash_syncLittlefs(const struct lfs_config *c )
{
	return  LFS_ERR_OK;

}

