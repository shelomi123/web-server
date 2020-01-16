//#include "common.h"
#include <strings.h>

char *mime_type;

void respond(int fd, char *header, void *body, int length){
	char responce[length+100];
	int responce_length = sprintf(responce,"%s\n""Connection: close\n""Content-length: %d\n""Content-type: %s\n""\n",header,length,mime_type);
	
	memcpy(responce + responce_length, body,length);
	send(fd, responce,responce_length + length, 0);
}

void handleFile(int fd, char *fileName){
	char *source;
	FILE *file = fopen(fileName, "r");
	size_t bufferSize;

	if(file != NULL){
		if(fseek(file,0L,SEEK_END) == 0){
			bufferSize = ftell(file);
			if(bufferSize > 10000000){
				char *data = "File size too big";
				respond(fd, "HTTP/1.1 500 Internal Server Error", data, strlen(data));
				return;
			}
			source = malloc(sizeof(char) * (bufferSize + 1));
			fseek(file, 0L, SEEK_SET);
			fread(source, sizeof(char), bufferSize, file);
			respond(fd, "HTTP/1.1 OK", source, bufferSize);
		}
		free(source);
		fclose(file);
	}else{
		char *error = "file not found";
        	mime_type = html;
        	respond(fd, "HTTP/1.1 404 NOT FOUND", error, strlen(error));
	}
}

int main(int argc, char **argv)
{
	int listenfd, connfd, n;
	struct sockaddr_in servaddr;
	//uint8_t buff[MAXLINE+1];
	//uint8_t recvline[MAXLINE+1];
	char buffer[1024] = {0}; 
    	char requestType[4];       
    	char requestpath[1024]; 

	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("socket error");

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERVER_PORT);

	if((bind(listenfd, (SA *) &servaddr, sizeof(servaddr))) < 0)
		printf("bind error");

	if((listen(listenfd, 10)) < 0)
		printf("listen error");

	for( ; ; ) {
		struct sockaddr_in addr;
		socklen_t addr_len;

		printf("Waiting for a connection on port %d\n", SERVER_PORT);
		fflush(stdout);

		if((connfd = accept(listenfd, (SA *)&servaddr, (socklen_t*)&servaddr)) == -1){
			printf("accept error");
		}

		read( connfd, buffer, 1024);

		sscanf(buffer, "%s %s", requestType, requestpath);
		printf("request Type: %s \n", buffer);

		for(int i=0; i< 100; i++){
			requestpath[i] = tolower(requestpath[i]);
		}

		char *mime = strrchr(requestpath, '.')+1;
        	char *name = strtok(requestpath, "/");

		if(mime)
			mime_type = mime;
		else
			mime_type = NULL;

		if(!strcmp(requestType, "GET") && !strcmp(requestpath, "/")) {
			char *data = "Get request received: Displaying root page";
			mime_type = html;
			respond(connfd,"HTTP/1.1 200 OK", data, strlen(data));
		}else if(!strcmp(requestType, "POST") && !strcmp(requestpath, "/")){
			char *data = "Post request received";
			mime_type = html;
			respond(connfd,"HTTP/1.1 200 OK",data,strlen(data));
		}else{
			handleFile(connfd,name);
		}

		close(connfd);
		
		//memset(recvline, 0, MAXLINE);

		/*while( (n = read(connfd, recvline, MAXLINE-1) ) > 0)
		{
			//fprintf(stdout, "\n%s\n\n%s",bin2hex(recvline,n),recvline);
			if(recvline[n-1] == '\n'){
				break;
			}
			memset(recvline, 0 , MAXLINE);
		}
		
		if(n < 0)
			printf("read error");
		snprintf((char*)buff, sizeof(buff), "HTTP/1.1 200 OK\r\n\r\nHello");
		write(connfd, (char*)buff, strlen((char*)buff));
		close(connfd);*/
	}

	return 0;
}
