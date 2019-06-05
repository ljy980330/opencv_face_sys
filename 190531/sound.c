//标准输入输出头文件
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

//打开文件需要的头文件
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//打开串口需要的头文件
#include <termios.h>
#include <unistd.h>

//语音模块的头文件
#include "SYN6288.h"

//字符编码转换
#include <iconv.h>


//声明串口初始化函数
int set_opt(int,int,int,char,int);
static void UART_SendData(int fd,unsigned char *buffer,int count);

//全局变量
char *uartPath = "/dev/ttySAC3";
int fd;

//字符编码转换
static int charset_convert(const char *from_charset, const char *to_charset,
                           char *in_buf, size_t in_left, char *out_buf, size_t out_left)
{
    iconv_t icd = (iconv_t)-1;
    size_t sRet = -1;
    char *pIn = in_buf;
    char *pOut = out_buf;
    size_t outLen = out_left;
 
    if (NULL == from_charset || NULL == to_charset || NULL == in_buf || 0 >= in_left || NULL == out_buf || 0 >= out_left)
    {
        return -1;
    }
 
    icd = iconv_open(to_charset, from_charset);
    if ((iconv_t)-1 == icd)
    {
        return -1;
    }
 
    sRet = iconv(icd, &pIn, &in_left, &pOut, &out_left);
    if ((size_t)-1 == sRet)
    {
        iconv_close(icd);
        return -1;
    }
 
    out_buf[outLen - out_left] = 0;
    iconv_close(icd);
    return (int)(outLen - out_left);
}


static int charset_convert_UTF8_TO_GB2312(char *in_buf, size_t in_left, char *out_buf, size_t out_left)
{
    return charset_convert("UTF-8", "GB2312", in_buf, in_left, out_buf, out_left);
}


int main(int argc, char **argv)
{
	unsigned char *buffer = (unsigned char *)malloc(sizeof(unsigned char) * 200);
	unsigned char *buffer_gb2312;
	int bufSize;
	int i, ret;

	if(argc != 2)
	{
		printf("argument number error!!\n");
		printf("%s \"the text you want to say\"\n", argv[0]);
		return -1;
	}

	if((fd = open(uartPath,O_RDWR|O_NOCTTY)) < 0)
	{
		printf("can not open %s\n",uartPath);
		return -1;
	}

	//printf("text: %s\n", argv[1]);
	
	set_opt(fd,9600,8,'N',1);

	bufSize = strlen(argv[1]);
	buffer_gb2312 = (unsigned char *)malloc(sizeof(unsigned char) * bufSize);

    ret = charset_convert_UTF8_TO_GB2312(argv[1], (size_t)bufSize, buffer_gb2312, (size_t)bufSize);
    if (-1 == ret)
    {
    	printf("failed convert UTF-8 to GB2312\n");
        return -1;
    }	

	bufSize = SYN_FrameInfo(0, buffer_gb2312, buffer);

	UART_SendData(fd, buffer, bufSize);

	free(buffer);
	free(buffer_gb2312);
	
	close(fd);
	
	return 0;
}


//串口发送一个命令包
static void UART_SendData(int fd,unsigned char *buffer,int count)
{
	int j;

	write(fd, buffer, count);

	//打印发出的数据至屏幕
	printf("send : ");
	//清空输出流缓存区
	fflush(stdout);
	for(j=0;j<count;j++)
	{
		printf("%02X ",buffer[j]);
		fflush(stdout);
	}
	printf("\n");
}


/*******************************************************************************************
	函数名：set_opt();
	函数作用：串口初始化
	参数说明： 参数1：fd----open函数返回的文件句柄
			   参数2 	：nSpeed----波特率设置（2400，4800，9600，115200）
			   参数3：nBits----数据位设置（7，8）
			   参数4：nEvent----校验位设置（'O'，'E'，'N'）
			   参数5：nStop----停止位（1，2）			   
*******************************************************************************************/
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop){
	struct termios newtio,oldtio;
	//将旧的参数配置文件赋给fd指向的文件
	if(tcgetattr(fd,&oldtio) != 0){
		perror("SetupSerial 1");
		return -1;
	}
	//清空结构体newtio中的数据
	bzero(&newtio,sizeof(newtio));
	newtio.c_cflag |= CLOCAL|CREAD;
	newtio.c_cflag &= ~CSIZE;

	//配置数据位
	switch (nBits)
		{
		case 7:{
			newtio.c_cflag |= CS7;
			break;}
		case 8:{
			newtio.c_cflag |= CS8;
			break;}
		}
	
	//配置校验位
	switch (nEvent)
		{
		case 'O':{
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |= PARODD;
			newtio.c_iflag |= (INPCK | ISTRIP);
			break;}
		case 'E':{
			newtio.c_iflag |= (INPCK | ISTRIP);
			newtio.c_cflag |= PARENB;
			newtio.c_cflag &= ~PARODD;
			break;}
		case 'N':{
				newtio.c_cflag &= ~PARENB;
				break;}
		}

	//配置波特率
	switch (nSpeed)
		{
		case 2400:{
			cfsetispeed(&newtio,B2400);
			cfsetospeed(&newtio,B2400);
			break;}
		case 4800:{
			cfsetispeed(&newtio,B4800);
			cfsetospeed(&newtio,B4800);
			break;}
		case 9600:{
			cfsetispeed(&newtio,B9600);
			cfsetospeed(&newtio,B9600);
			break;}
		case 115200:{
			cfsetispeed(&newtio,B115200);
			cfsetospeed(&newtio,B115200);
			break;}
		case 460800:{
			cfsetispeed(&newtio,B460800);
			cfsetospeed(&newtio,B460800);
			break;}
		default:{
			cfsetispeed(&newtio,B9600);
			cfsetospeed(&newtio,B9600);
			break;}
		}

	//配置停止位
	if(nStop == 1){
		newtio.c_cflag &= ~CSTOPB;
	}
	else if(nStop == 2){
		newtio.c_cflag |= CSTOPB;
	}
	//清空数据
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 0;

/*******************************************************************************************
	函数名：tcflush();
	函数作用：清空串口缓存器BUFFER中的数据
	参数说明： 参数1：fd----open函数返回的文件句柄
			   参数2：TCIFLUSH----清除正受到的数据，且不会读出来
			   		  TCOFLUSH----清除正写入的数据，且不会发送至终端
			          TCIOFLUSH----清除所有正在发生的IO数据
	函数返回值：成功返回0，失败返回-1			   
*******************************************************************************************/
	tcflush(fd,TCIFLUSH);

/*******************************************************************************************
	函数名：tcsetattr();
	函数作用：设置串口参数函数
	参数说明： 参数1：fd----open函数返回的文件句柄
			   参数2：TCSANOW----不等数据传输完毕就立刻改变属性
			   		  TCSADRAIN----等待所有数据传输结束才改变属性
			          TCSAFLUSH----清空输入输出缓冲区才改变属性
			   参数3：newtio----在旧的参数基础上修改后的参数
	函数返回值：成功返回0，失败返回-1			   
*******************************************************************************************/	
	if((tcsetattr(fd,TCSANOW,&newtio)) != 0){
		perror("com set error");
		return -1;
	}
	return 0;
}

