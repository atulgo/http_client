/*==================================================================================*/

#ifndef HTTP_GET_LIBSOCKET_H
#define HTTP_GET_LIBSOCKET_H

/*======================================================================*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdint.h>
#include<regex.h>
# include <libinetsocket.h>

/*======================================================================*/

#define LFLF 			"\n\n"
#define CRLF			"\r\n"
#define CRLFCRLF 		"\r\n\r\n"
#define HDR_SEP 		"\r\n"
#define CHUNKED		"chunked"
#define TE_CHUNKED	"\r?\ntransfer-encoding *: *chunked *\r?\n"
#define CONTLEN		"\r?\ncontent-length *: *"
#define USER_AGENT	"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.86 Safari/537.36"

/*==================================================================================*/

#define REPLY_INIT_LEN	1024*128
#define BUFLEN		1024
#define PORTLEN		7
/*==================================================================================*/

/*THESE Errors occur before any memory allocations and regex compilations .... and therefore cant be handled by FAIL macro.. */
enum
{
	ERR_URL_TOO_LONG
	,ERR_REGCOMP_FAILED_CONTLEN
	,ERR_REGCOMP_FAILED_TECHUNKED
	,USAGE
};

/*==================================================================================*/

#define ARR_LEN(__arr)					(sizeof(__arr)/sizeof(__arr[0]))

/*==================================================================================*/

#define DEBUG_PRN( __FMT__ , __VAR__ )	printf( " :: %s :: %d :: " __FMT__ " \n" , __FILE__ , __LINE__ , __VAR__ ) ;

/*==================================================================================*/

#define DEBUG_PUTS( __STR__ )			printf( " :: %s :: %d :: %s \n", __FILE__ , __LINE__  , __STR__) ;

/*==================================================================================*/

#define BYTES_TO_UINT32(__buf,__myuint32)	\
do										\
{										\
	__myuint32=							\
		((__buf[0] << 24 ) & 0xff000000 ) |	\
		((__buf[1] << 16 ) & 0x00ff0000 ) |	\
		((__buf[2] << 8 ) & 0x0000ff00) |		\
		(__buf[3] & 0x000000ff);				\
}while(0);

/*==================================================================================*/

#define UINT32_TO_BYTES(__myuint32,__buf)	\
do										\
{ 										\
	__buf[0]=(uint8_t)(__myuint32>>24) ;		\
	__buf[1]=(uint8_t)(__myuint32>>16) ;		\
	__buf[2]=(uint8_t)(__myuint32>>8) ;		\
	__buf[3]=(uint8_t)(__myuint32) ;			\
}while(0);

/*==================================================================================*/

#define FAIL( __STR__)						\
do										\
{ 										\
	uint8_t __buf[1024] ; 					\
	snprintf( 	__buf , sizeof(__buf) 			\
			, " :: %s :: %d :: %s"				\
			, __FILE__ , __LINE__ , __STR__ 	\
			) ;							\
	fail(__buf);							\
}while(0);

/*==================================================================================*/

#define DO_REALLOC(ptr , sz)			\
do									\
{									\
	char*nptr=realloc(ptr,sz);			\
	if(nptr==NULL)					\
	{								\
		free(ptr);						\
		FAIL("realloc error :")			\
	}								\
	ptr = nptr;						\
}while(0);

/*==================================================================================*/

#define DO_MALLOC(ptr , sz)				\
do									\
{									\
	ptr=malloc(sz);					\
	if(ptr==NULL)						\
		FAIL("malloc error :")			\
}while(0);

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

#endif	// HTTP_GET_LIBSOCKET_H

/*==================================================================================*/

