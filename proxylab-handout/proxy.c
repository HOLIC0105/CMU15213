#include <stdio.h>
#include <string.h>
#include "csapp.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define	MAXLINE	 8192 

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

typedef char string[MAXLINE];

typedef struct{
    string host;
    string port;
    string path;
}url_t;

void * doit(void * tmpfd);
int  parse_url(char *uri, url_t * url_info);
int parse_header(rio_t* rio, string header_info, string host);


int main(int argc, char *argv[])
{
    
    Signal(SIGPIPE, SIG_IGN);

    int listenfd, * connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if(argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
	    exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    pthread_t tpid;
    while(1) {
        clientlen = sizeof(clientaddr);
        connfd = (int *) malloc (sizeof(int));
        *connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); 
        Getnameinfo((SA *) &clientaddr, clientlen, 
                    hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        pthread_create(&tpid, NULL, doit, connfd);           
    }
    return 0;
}

void * doit(void * tmpfd) {
    pthread_detach(pthread_self());
    int client_fd = *((int *) tmpfd);
    free(tmpfd);
    rio_t client_rio;
    char buf[MAXLINE << 2];
    string method, url, version;
    Rio_readinitb(&client_rio,  client_fd);
    if(!rio_readlineb(&client_rio, buf, MAXLINE)) {
        fprintf(stderr, "Parse request line error: %s\n", strerror(errno));
        close(client_fd);
        return NULL;
    }
    if(sscanf(buf, "%s %s %s", method, url, version) != 3) {
        fprintf(stderr, "Parse request line error: %s\n", strerror(errno));
        close(client_fd);
        return NULL;
    }
    if(strcasecmp(method, "GET")) {
        fprintf(stderr, "Proxy does not implement the method");
        close(client_fd);
        return NULL;
    }
    url_t url_info;
    if(!parse_url(url, &url_info)) {
        string header_info;
        parse_header(&client_rio, header_info, url_info.host);
        int server_fd = open_clientfd(url_info.host, url_info.port);
        rio_t server_rio;
        if(server_fd < 0) {
            fprintf(stderr, "Open connect to %s:%s error\n", url_info.host, url_info.port);
            return NULL;
        }
        rio_readinitb(&server_rio, server_fd);
        sprintf(buf, "GET %s HTTP/1.0\r\n%s", url_info.path, header_info);
        if (rio_writen(server_fd, buf, strlen(buf)) != strlen(buf)) {
            fprintf(stderr, "Send request line and header error\n");
            close(server_fd);
            return NULL;
        }
        int readlen;
        while ((readlen = rio_readnb(&server_rio, buf, MAXLINE))) {
            if (readlen < 0) {
                fprintf(stderr, "Read server response error\n");
                close(server_fd);
                return NULL;
            }
            if (rio_writen(client_fd, buf, readlen) != readlen) {
                fprintf(stderr, "Send response to client error\n");
                close(server_fd);
                return NULL;
            }
        }
        close(server_fd);
    }
    close(client_fd);
    return NULL;
}
int parse_url(char * url, url_t * url_info) 
{
    if(strncasecmp(url, "http://", 7)) {
        fprintf(stderr, "Not http protocol: %s\n", url);
        return -1;
    }
    char * host = url + 7;
    char * port = strchr(host, ':');
    char * path = strchr(host, '/');
    if(port == NULL) {
        *path = '\0';
        strcpy(url_info->host, host);
        strcpy(url_info->port, "80");
        *path = '/';
        strcpy(url_info->path, path);
    } else {
        *port = '\0';
        strcpy(url_info->host, host);
        *path = '\0';
        strcpy(url_info->port, port + 1);
        *path = '/';
        strcpy(url_info->path, path);
    }
    return 0;
}
int parse_header(rio_t* rio, string header_info, string host) {
    string buf;
    int flag = 0;
    while(1) {
        rio_readlineb(rio, buf, MAXLINE);
        if(!strcmp(buf, "\r\n")) break;
        if(!strncasecmp(buf, "Host:", 5)) flag = 1;
        if(strncasecmp(buf, "Connection:", 11)) continue;
        if(strncasecmp(buf, "Proxy-Connection:", 17)) continue;
        if(strncasecmp(buf, "User-Agent:", 11)) continue;
        strcat(header_info, buf);
    }
    if(!flag) {
        sprintf(buf, "Host: %s\r\n", host);
        strcpy(header_info, buf);
    }
    strcat(header_info, "Connection: close\r\n");
    strcat(header_info, "Proxy-Connection: close\r\n");
    strcat(header_info, user_agent_hdr);
    strcat(header_info, "\r\n");
    return 0;
}