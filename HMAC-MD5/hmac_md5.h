/********************************************************************************
* @file    	  	hmac_md5.h
* @function   	���ڼ��㰢����������ƽ̨��¼����
*
* @company		�����з�˼�����ӿƼ����޹�˾
* @website		www.freestrong.com
* @tel			0755-86528386
* @Author		freestrong
* @date			2021/12/24
********************************************************************************/

#ifndef __HMAC_MD5_H
#define __HMAC_MD5_H

#include <stdio.h>
#include <stdlib.h> 
#include "md5.h"
#include <string.h>

#define LENGTH_MD5_RESULT 16
#define LENGTH_BLOCK 64

void hmac_md5(unsigned char* out, unsigned char* data, int dlen, unsigned char* key, int klen);


#endif


