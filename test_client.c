#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero

 
#define SERVER_PORT 2000
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512
struct Recv_Sensor
{
	float yaw;
	float pitch;
	float roll;
	float q0;
	float q1;
	float q2;
	float q3;	
	float temperature;
	float angle;
	float light;
	int   gas;
	int   gear_flat;
}; 
struct Recv_Sensor recv_struct;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Please input the IP address of the server \n");
        exit(1);
    }
 
    //����һ��socket��ַ�ṹclient_addr,����ͻ���internet��ַ, �˿�
    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof(client_addr)); //��һ���ڴ���������ȫ������Ϊ0
    client_addr.sin_family = AF_INET; //internetЭ����
    client_addr.sin_addr.s_addr = htons(INADDR_ANY); //INADDR_ANY��ʾ�Զ���ȡ������ַ
    client_addr.sin_port = htons(0); //0��ʾ��ϵͳ�Զ�����һ�����ж˿�
    //��������internet����Э��(TCP)socket,��client_socket����ͻ���socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }
    //�ѿͻ�����socket�Ϳͻ�����socket��ַ�ṹ��ϵ����
    if (bind(client_socket, (struct sockaddr*) &client_addr,
            sizeof(client_addr)))
    {
        printf("Client Bind Port Failed!\n");
        exit(1);
    }
 
    //����һ��socket��ַ�ṹserver_addr,�����������internet��ַ, �˿�
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (inet_aton(argv[1], &server_addr.sin_addr) == 0) //��������IP��ַ���Գ���Ĳ���
    {
        printf("Server IP Address Error! \n");
        exit(1);
    }
 
    server_addr.sin_port = htons(SERVER_PORT);
    socklen_t server_addr_length = sizeof(server_addr);
    // ���������������,���ӳɹ���client_socket�����˿ͻ����ͷ�������һ��socket����
    if (connect(client_socket, (struct sockaddr*) &server_addr,
            server_addr_length) < 0)
    {
        printf("Can Not Connect To %s!\n", argv[1]);
        exit(1);
    }
	int n;
	unsigned int count=0;
	char tmp[100];
    while(1)
	{
		count++;
		memset(tmp,0,sizeof(tmp)); //���ڴ� 
		n = recv(client_socket, tmp, sizeof(recv_struct), 0);
		if (n<0)
		{
			printf("Recv failed:\n");
			break;
		}
		memcpy(&recv_struct,tmp,sizeof(recv_struct));

		printf("%.1f  ",recv_struct.yaw);
		printf("%.1f  ",recv_struct.pitch);
		printf("%.1f  ",recv_struct.roll);
		printf("%.1f  ",recv_struct.q0);
		printf("%.1f  ",recv_struct.q1);
		printf("%.1f  ",recv_struct.q2);
		printf("%.1f\n",recv_struct.q3);
		printf("wendu     %.1f\n",recv_struct.temperature);
		printf("jiaodu    %.1f\n",recv_struct.angle);
		printf("guangzhao %.1f\n",recv_struct.light);
		printf("gas       %d\n",recv_struct.gas);
		printf("gear      %d\n",recv_struct.gear_flat);
		printf("times      %d\n",count);
		/*//
		if(count==100)
		{
		    unsigned char data[3]={0x01,0xaa,0xab};
			n = send(client_socket, data, 3, 0);
			if (n<0)
			{
				printf("Recv failed:\n");
				break;
			}
		}
		if(count==130)
		{
		    unsigned char data[3]={0x01,0xcc,0xcd};
			n = send(client_socket, data, 3, 0);
			if (n<0)
			{
				printf("Recv failed:\n");
				break;
			}
		}
		if(count==160)
		{
		    unsigned char data[3]={0x01,0xdd,0xde};
			n = send(client_socket, data, 3, 0);
			if (n<0)
			{
				printf("Recv failed:\n");
				break;
			}
		}
		if(count==190)
		{
		    unsigned char data[3]={0x01,0xbb,0xbc};
			n = send(client_socket, data, 3, 0);
			if (n<0)
			{
				printf("Recv failed:\n");
				break;
			}		
		
		}
		usleep(30000);
		if(count==220)
		{
		    unsigned char data[3]={0x01,0xee,0xef};
			n = send(client_socket, data, 3, 0);
			if (n<0)
			{
				printf("Recv failed:\n");
				break;
			}		
		
		}
				unsigned char data[3]={0x03,0x33,0x36};
			n = send(client_socket, data, 3, 0);
			if (n<0)
			{
				printf("Recv failed:\n");
				break;
			}
		if(500==count)
		{   i=count;
		    unsigned char data[3]={0x03,0x11,0x14};
			n = send(client_socket, data, 3, 0);
			if (n<0)
			{
				printf("Recv failed:\n");
				break;
			}
		}	
		static i=0;
		if(30==(count-i))
		{   i=count;
		    unsigned char data[3]={0x03,0x11,0x14};
			n = send(client_socket, data, 3, 0);
			if (n<0)
			{
				printf("Recv failed:\n");
				break;
			}
		}//*/
		if(count==100)
		{
			unsigned char data[3]={0x03,0x33,0x36};
			n = send(client_socket, data, 3, 0);
			if (n<0)
			{
				printf("Recv failed:\n");
				break;
			}
		
		}


	}

    close(client_socket);
    return 0;
}