/*************************************************************************
	> File Name: sm3.h
	> Author:siminxu
	> Mail: 
	> Created Time: 2020年12月02日 星期三 09时24分01秒
 ************************************************************************/
#ifndef SM3_H
#define SM3_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void out_hex();
void intToString(unsigned char *out_hash);
int SM3(unsigned char *msg, unsigned int msglen, unsigned char *out_hash);

#endif
