/*************************************************************************
	> File Name: sm3.c
	> Author:siminxu
	> Mail: 
	> Created Time: 2020年12月02日 星期三 09时24分55秒
 ************************************************************************/
#include "sm3.h"

static unsigned char message_buffer[64] = {0};    //存放消息分组
static unsigned int hash[8] = {0};                //用来存放哈希值
static unsigned int T[64] = {0};                  //加密中的常量

void out_hex()                       //输出处理后的哈希值
{
	unsigned int i = 0;
	for (i = 0; i < 8; i++)
	{
		printf("%08x ", hash[i]);
	}
	printf("\n");
}

void intToString(unsigned char *out_hash)        
{
	int i=0;
	for (i = 0; i < 8; i++)
	{
		out_hash[i * 4] = (unsigned char)((hash[i] >> 24) & 0xFF);
		out_hash[i * 4 + 1] = (unsigned char)((hash[i] >> 16) & 0xFF);
		out_hash[i * 4 + 2] = (unsigned char)((hash[i] >> 8) & 0xFF);
		out_hash[i * 4 + 3] = (unsigned char)((hash[i]) & 0xFF);
	}
	for (i = 0; i < 32; i++)
	printf("%c", out_hash[i]);
	printf("\n");

}

//循环左移
unsigned int rotate_left(unsigned int a, unsigned int k)
{
	k = k % 32;
	return ((a << k) & 0xFFFFFFFF) | ((a & 0xFFFFFFFF) >> (32 - k));
}

//常量  Ti = 79cc4519 (0-15)  7a879d8a (16-63)
int init_T()
{
	int i = 0;
	for (i = 0; i < 16; i++)
	{
		T[i] = 0x79cc4519;
	}
	for (i = 16; i < 64; i++)
	{
		T[i] = 0x7a879d8a;
	}
	return 1;
}

/*FF为布尔函数，通过改变j，改变表达式
*FF(X, Y, Z) = X ^ Y ^ Z  0 - 15

FF(X, Y, Z) = (X  & Y) | (X & Z) | (Y & Z) 16-63*/
unsigned int FF(X, Y, Z, j)
{
	unsigned int ret = 0;
	if (0 <= j && j < 16)
	{
		ret = X ^ Y ^ Z;
	}
	else if (16 <= j && j < 64)
	{
		ret = (X & Y) | (X & Z) | (Y & Z);
	}
	return ret;
}
//GG
unsigned int GG(X, Y, Z, j)
{
	unsigned int ret = 0;
	if (0 <= j && j < 16)
	{
		ret = X ^ Y ^ Z;
	}
	else if (16 <= j && j < 64)
	{
		ret = (X & Y) | ((~X) & Z);
	}
	return ret;
}

//置换函数：调用循环左移rotate_left()函数
#define P_0(X) X ^ (rotate_left(X, 9)) ^ (rotate_left(X, 17))
#define P_1(X) X ^ (rotate_left(X, 15)) ^ (rotate_left(X, 23))


/*
 *迭代压缩函数CF
 *A,B,C,D,E,F,G,H为字寄存器，SS1,SS2,TT1,TT2为中间值
 *压缩函数V(i + 1) = CF(V(i), B(i)); 
 *ABCDEFGH <-V(i)
 *
 *
 */
int CF(unsigned char *arr)
{
	unsigned int W[68];
	unsigned int W_1[64];
	unsigned int j;
	unsigned int A, B, C, D, E, F, G, H;
	unsigned int SS1, SS2, TT1, TT2;

    /*
     * 将消息分组B(i)划分为16个字W0，W1...W15
     */
	for (j = 0; j < 16; j++)
	{
		W[j] = arr[j * 4 + 0] << 24 | arr[j * 4 + 1] << 16 | arr[j * 4 + 2] << 8 | arr[j * 4 + 3];
	}
	for (j = 16; j < 68; j++)
	{
		W[j] = P_1(W[j - 16] ^ W[j - 9] ^ (rotate_left(W[j - 3], 15))) ^ (rotate_left(W[j - 13], 7)) ^ W[j - 6];
	}
	for (j = 0; j < 64; j++)
	{
		W_1[j] = W[j] ^ W[j + 4];
	}
	A = hash[0];
	B = hash[1];
	C = hash[2];
	D = hash[3];
	E = hash[4];
	F = hash[5];
	G = hash[6];
	H = hash[7];
    /*
     * 压缩函数
     *
     */
	for (j = 0; j < 64; j++)
	{
		SS1 = rotate_left(((rotate_left(A, 12)) + E + (rotate_left(T[j], j))) & 0xFFFFFFFF, 7);
		SS2 = SS1 ^ (rotate_left(A, 12));
		TT1 = (FF(A, B, C, j) + D + SS2 + W_1[j]) & 0xFFFFFFFF;
		TT2 = (GG(E, F, G, j) + H + SS1 + W[j]) & 0xFFFFFFFF;
		D = C;
		C = rotate_left(B, 9);
		B = A;
		A = TT1;
		H = G;
		G = rotate_left(F, 19);
		F = E;
		E = P_0(TT2);

	}
	
	hash[0] = (A ^ hash[0]);
	hash[1] = (B ^ hash[1]);
	hash[2] = (C ^ hash[2]);
	hash[3] = (D ^ hash[3]);
	hash[4] = (E ^ hash[4]);
	hash[5] = (F ^ hash[5]);
	hash[6] = (G ^ hash[6]);
	hash[7] = (H ^ hash[7]);
	return 1;
}

//给定初始向量IV,确定压缩寄存器的初态
void SM3_Init()
{
	init_T();
	hash[0] = 0x7380166f;
	hash[1] = 0x4914b2b9;
	hash[2] = 0x172442d7;
	hash[3] = 0xda8a0600;
	hash[4] = 0xa96f30bc;
	hash[5] = 0x163138aa;
	hash[6] = 0xe38dee4d;
	hash[7] = 0xb0fb0e4e;
}
/*
 * 消息填充
 * 讲bit“1”添加到消息的末尾，添加K个0，k满足l+1+k=448 mod 512。
 * 然后添加一个64为比特串，该比特串为l的长度二进制表示
 * 填充后的消息比特长度为512的倍数
 */
void Block(unsigned char *msg,unsigned int msglen){
	int i;
	int left = 0;
	unsigned long long total = 0;
		
	for(i = 0; i < msglen/64; i++){
		memcpy(message_buffer, msg + i * 64, 64);
		CF(message_buffer);
	}
	
	total = msglen * 8;
	left = msglen % 64;
	memset(&message_buffer[left], 0, 64 - left);	
	memcpy(message_buffer, msg + i * 64, left);
	message_buffer[left] = 0x80;

    /*
     * 对填充后的消息进行比特分组
     */
	if(left <= 55){
		for (i = 0; i < 8; i++)
			message_buffer[56 + i] = (total >> ((8 - 1 - i) * 8)) & 0xFF;
		CF(message_buffer);
	}else{
		CF(message_buffer);
		memset(message_buffer, 0, 64);
		for (i = 0; i < 8; i++)
			message_buffer[56 + i] = (total >> ((8 - 1 - i) * 8)) & 0xFF;
		CF(message_buffer);
	}
	
}

int SM3(unsigned char *msg, unsigned int msglen, unsigned char *out_hash)
{
	SM3_Init();
	Block(msg,msglen);
	out_hex();
	return 1;
}

int main(int argc, char *argv[])
{
	unsigned char Hash[32]={0};
	char str[512];
    fgets(str, 512, stdin);
	int len;
	len=strlen(str);
	if(!SM3(str, len - 1, Hash))
		printf("1 false\n");
	return 0;
}
