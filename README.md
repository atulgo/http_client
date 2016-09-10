# http_client

Basic HTTP-1 client using libsocket

Usage :  
       ./http_get_libsocket [ <url> | -h | --help | -? ] 

==> if no command line arguments are provided the program fetches "http://api.ipify.org/"

Dependencies : libsocket - https://github.com/dermesser/libsocket
  
Compilation : gcc -o http_get_libsocket http_get_libsocket.c -lsocket


