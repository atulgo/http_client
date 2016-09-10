/*======================================================================*/

#include "http_get_libsocket.h"

/*======================================================================*/

regex_t 	__GLBL__rgx_cont_len , __GLBL__rgx_xfer_encoding ;
char* 	__GLBL__reply_buf ;

/*======================================================================*/

void fail(char*s)
{
	perror(s);
	regfree(&__GLBL__rgx_cont_len) ;
	regfree(&__GLBL__rgx_xfer_encoding) ;
	if(__GLBL__reply_buf)
		free( __GLBL__reply_buf) ;
	exit(0);
}

/*======================================================================*/

char* parse_url(char*url , char*host,char*port)
{
	char*hostbeg=strstr(url,"http://") ;
	if(hostbeg==NULL)
		FAIL("Invalid url. \nURL must start with 'http://' ")
	hostbeg+=strlen("http://") ;
	size_t hostlen = strcspn(hostbeg , ":/") ;
	if(hostlen==0)				// url entered was the string "http://"
		FAIL("Invalid url ")
	strncpy(host,hostbeg,hostlen) ;
	host[hostlen]=0;				//got to manually add terminating NULL
	char*portbeg	= hostbeg+hostlen  ;
	
	char*path = portbeg ;
	if( portbeg = strchr( portbeg , ':' ) )
	{
		unsigned long portnum = strtoul( portbeg+1 , &path , 10) ;
		snprintf(port , PORTLEN , "%u" , portnum) ;
	}
	if(*path==0)
	{
		*path='/';
		*(path+1) = 0 ;
	}
	return path;
}

/*======================================================================*/

int send_http_request(char*url)
{
	char host[BUFLEN] , port[PORTLEN]="80" , *path;
	path = parse_url(url,host,port);
	char  request[2048];
	snprintf
	(
		request, sizeof(request) , 
		"GET %s HTTP/1.1"			CRLF
		"Host: %s"				CRLF
		"User-Agent: "USER_AGENT""	CRLF
		"Connection: close"			CRLFCRLF
		, path , host
	);
	int sock = create_inet_stream_socket(host , port, LIBSOCKET_IPv4,0);
	if ( sock < 0 )
		FAIL("create_inet_stream_socket failed :")
	int ret = write(sock,request,strlen(request));
	if ( ret < 0 )
		FAIL("write to sock failed ")
	return sock ;
}

/*======================================================================*/

void write_entire_file(char*fname , char*fbin , int fbinlen)
{
	FILE*fptr = fopen( fname  , "wb" ) ;
	if(fptr==NULL)
		FAIL("fopen failed ")
	int res =fwrite( fbin , sizeof(char ), fbinlen , fptr ) ;
	if(res==0)
	{
		fclose(fptr);	
		FAIL("fwrite failed ")
	}	
	fclose(fptr);	
}

/*======================================================================*/

int save_webpage_to_file(char*dest_path , char*fbin , int fbinlen , char*url)
{
	char fname[BUFLEN] ;
	strcpy( fname , dest_path) ;
	int nn=0 , offset = strlen(dest_path) ;
	for(nn=0 ; url[nn]!=0 ; nn++)
	{
		if(url[nn]=='/')
			fname[nn+offset]='_';
		else
			fname[nn+offset]=url[nn] ;
	}
	fname[nn+offset]=0;
	DEBUG_PUTS(fname)
	write_entire_file( fname , fbin , fbinlen );
}

/*======================================================================*/

// int write_entire_file(char*dest_path , char*fbin , int fbinlen , char*url)
// {
// 	char fname[BUFLEN] ;
// 	strcpy( fname , dest_path) ;
// 	int nn=0 , offset = strlen(dest_path) ;
// 	for(nn=0 ; url[nn]!=0 ; nn++)
// 	{
// 		if(url[nn]=='/')
// 			fname[nn+offset]='_';
// 		else
// 			fname[nn+offset]=url[nn] ;
// 	}
// 	fname[nn+offset]=0;
// 	DEBUG_PUTS(fname)
// 	FILE*fptr = fopen( fname  , "wb" ) ;
// 
// 	if(fptr==NULL)
// 		FAIL("fopen failed ")
// 	
// 	int res =fwrite( fbin , sizeof(char ), fbinlen , fptr ) ;
// 	if(res==0)
// 	{
// 		fclose(fptr);	
// 		FAIL("fwrite failed ")
// 	}
// 	
// 	fclose(fptr);	
// 	return 0;
// }

/*======================================================================*/

int is_chunked_reply_complete(char*body , int bodylen)
{
	char*curr=body , *first_crlf;
	long chunklen=0 ;
	int ntot = bodylen , two_crlf_len = (2*strlen(CRLF)) ;

	for(;;)
	{
		chunklen = strtol(curr,&first_crlf,16) ;
		int n = (first_crlf-curr) + two_crlf_len + chunklen  ;
		if( ntot < n)
			return 0 ;
		if((chunklen==0) && (ntot==n))
			return 1;
		ntot-=n ;
		curr+=n;
	}
}

/*======================================================================*/

int is_http_reply_complete(char*reply , long reply_len )
{
	regex_t*prgx_cl=&__GLBL__rgx_cont_len ; 
	regex_t*prgx_te=&__GLBL__rgx_xfer_encoding ;
	
	char *body = strstr(reply , CRLFCRLF) ;
	if(body==NULL)
		return 0 ;
	else
		body+=strlen(CRLFCRLF) ;

	long hdrlen = body - reply ;
	//char*pchar=reply;
	regmatch_t match;
	
	int res = regexec(prgx_cl , reply , 1 , &match , 0 ) ;
	if(res==0)
	{
		long cont_len = strtol( (reply+match.rm_eo) , NULL , 10);
		if( reply_len == ( hdrlen+cont_len))
			return 1;
	}
	else
	{
		res = regexec(prgx_te , reply , 0 , NULL , 0 ) ;
		if((res==0) && (is_chunked_reply_complete(body , reply_len-hdrlen)==1) )
			return 1 ;
	}
	return 0;
}

/*======================================================================*/

int http_status_redirect(char*url)
{
	char *pchar=__GLBL__reply_buf ;

	for(	;(*pchar) && (*pchar!=' ') ; pchar++ ) {}

	long status = strtol(pchar,NULL,10) ;
	if((status == 301) || (status == 302)|| (status == 303)|| (status == 307)|| (status == 308))
	{
		
		for(
			pchar = strstr(pchar , CRLF)
			; strcmp(pchar,CRLFCRLF)
			; pchar++
		)
		{
			*pchar = tolower(*pchar) ;
		}
		if( pchar=strstr(__GLBL__reply_buf , "location"))
			pchar+=strlen("location");

		for(	;(*pchar==':') || (*pchar==' ') ; pchar++ ) {}
		int len = strcspn(pchar,"\r\n") ;
		strncpy(url , pchar,len);
		url[len]=0;
		DEBUG_PUTS("HTTP status redirect .... ")
		return 1 ;
	}
}


/*======================================================================*/

void receive_http_reply(int sock )
{
	int reply_size , nrecv , reply_len ;

	for( 
		reply_size = REPLY_INIT_LEN , reply_len = 0 
		; nrecv= read( sock , __GLBL__reply_buf + reply_len , reply_size - reply_len ) 
		;
	)
	{
		if(nrecv<0)
			FAIL("read() failed ")

		reply_len+=nrecv ;

		if( reply_size < (reply_len+1) )
		{
			DO_REALLOC( __GLBL__reply_buf , 2*reply_len)
			DEBUG_PUTS("REALLOCating __GLBL__reply_buf .... ")
			reply_size = 2*reply_len;
		}

		if(is_http_reply_complete( __GLBL__reply_buf , reply_len )==1)
			break;
	}
	__GLBL__reply_buf[reply_len]=0 ;

}

/*======================================================================*/

void usage(char*progname)
{
	printf("\n	Fetch a Url\n\nUSAGE :: %s [ <url> | -h | --help | -? ] \n\n" , progname) ;
	exit(USAGE);
}

/*======================================================================*/

void init(int argc , char**argv ,char*url)
{
	__GLBL__reply_buf	= NULL ;

	if( 0!=(regcomp( &__GLBL__rgx_cont_len , CONTLEN , REG_EXTENDED|REG_ICASE )))
		exit(ERR_REGCOMP_FAILED_CONTLEN );

	if( 0!= (regcomp( &__GLBL__rgx_xfer_encoding , TE_CHUNKED , REG_EXTENDED|REG_ICASE )))
		exit(ERR_REGCOMP_FAILED_TECHUNKED );

	if(argc==2)
	{
		if(
			( strcmp(argv[1] , "--help")==0 ) 
			|| ( strcmp(argv[1] , "-h")==0 ) 
			|| ( strcmp(argv[1] , "-?")==0 ) 
		)
			usage(argv[0]) ;
		else
		{
			if(strlen(argv[1]) >= BUFLEN)
				FAIL( "ERR_URL_TOO_LONG ")
			strcpy( url , argv[1]  );
			DEBUG_PUTS(url)
		}
	}
	DO_MALLOC( __GLBL__reply_buf , REPLY_INIT_LEN )
}

/*======================================================================*/

void deinit()
{
	fail("") ;
}

/*======================================================================*/

int main(int argc,char**argv)
{
	char url[BUFLEN] = "http://api.ipify.org/";
	init( argc , argv , url ) ;
	do
	{
		DEBUG_PRN("URL == %s \n",url)
		int sock = send_http_request(url) ;
		receive_http_reply(sock) ;
		int ret = destroy_inet_socket(sock);
		if ( ret < 0 )
			FAIL("destroy_inet_socket failed ... ")

	}while(http_status_redirect(url) == 1) ;
	DEBUG_PUTS("fetched page successfully ...")
	save_webpage_to_file("./data/" , __GLBL__reply_buf , strlen( __GLBL__reply_buf ) , url );
	deinit();
	return 0;
}

/*======================================================================*/
/*======================================================================*/
/*======================================================================*/
/*======================================================================*/
/*======================================================================*/

// 	ERR_URL_TOO_LONG
// 	,ERR_REGCOMP_FAILED_CONTLEN
// 	,ERR_REGCOMP_FAILED_TECHUNKED

/*======================================================================*/
