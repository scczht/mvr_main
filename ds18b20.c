#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/ioctl.h>

//==================================================================
//��������  get_temperature(char *dev)
//���ߣ�    zht
//���ڣ�    2015/03/31
//���ܣ�    ��ȡDS18B20�¶�
//�������: /dev/ds18b20
//����ֵ��  �����¶�ֵ����ȷ��С������λ
//�޸ļ�¼��
//ע��      ����������ͨIOģ�����ʱ�򣬲�Ҫ�ɿ�����������
//          �����ή��ϵͳ���ܣ�Ҳû��Ҫ��
//==================================================================
float get_temperature(char *dev)
{
	float temp;
	int fd;
	int max_trying=10;
	char integer_value = 0;
	float decimal_value = 0;    
	char result[9]; 
	int i,tmp=0;
	fd = open(dev, 0);
	if (fd < 0)  
	{
		perror("open temperature error,please make sure ds18b20 drive is OK����\n");
		exit(1);
	}
	while(max_trying--)
	{
		if(read(fd, &result, sizeof(result))<0)
		{
			perror("read temperature ERROR\n");
			exit(1);
		}
	for(i=0;i<sizeof(result);i++)
	tmp+=result[i];
	if (0!=tmp)
		break; 
	}
	close(fd);
	if(0==max_trying)
	{
		printf("WARNING: the temperature maybe not corrct!!\n");
		return 999.0;
	}
	integer_value = ((result[0] & 0xf0) >> 4) | ((result[1] & 0x0f) << 4);
	decimal_value = 0.5*((result[0] & 0x08)>> 3) + 0.25*((result[0]&0x04)>> 2) + 0.125*((result[0]&0x02)>> 1);	
	if(integer_value>0)
		return ((float)integer_value + decimal_value);
	else
		return ((float)integer_value - decimal_value);
}