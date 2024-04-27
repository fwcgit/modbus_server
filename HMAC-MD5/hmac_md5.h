/********************************************************************************
* @file    	  	hmac_md5.h
* @function   	用于计算阿里云物联网平台登录密码
*
* @company		深圳市飞思创电子科技有限公司
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


