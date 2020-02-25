#include <stdio.h>
#include <stdlib.h>

int main(){
    printf("GET http://www.google.com:80/index.html/ HTTP/1.0\r\nContent-Length:80\r\nIf-Modified-Since: Sat, 29 Oct 1994 19:43:31 GMT\r\n\r\n");
    return 0;
}