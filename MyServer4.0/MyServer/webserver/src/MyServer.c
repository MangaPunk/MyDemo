//
// Created by Manganese on 2018/10/11.
//


#include "../include/MyServer.h"

void read_config_file(const char filename[], struct ConfigData * confData){
    //static const char filename[] = "../config.txt";
    FILE *file = fopen ( filename, "r" );
    if ( file != NULL )
    {
        char line [ MAXLINE ]; /* or other suitable maximum line size */
        while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
        {
            //fputs ( line, stdout ); /* write the line */
            char tmp[MAXLINE];
            if (strstr(line, "ROOT_PATH") != NULL){
                sscanf(line, "%s %s", tmp, confData->root_path);
            }
            else if (strstr(line, "DEFAULT_PORT") != NULL){
                sscanf(line, "%s %s", tmp, confData->default_port);
            }
            else if (strstr(line, "CONCURRENCY_STRATEGY") != NULL){
                sscanf(line, "%s %s", tmp, confData->default_concurrency_strategy);
            }
        }
        fclose ( file );
    }
    else
    {
        perror ( filename ); /* why didn't the file open? */
    }
}

void rio_readinitb(rio_t *rp, int fd) {
    rp->rio_fd=fd;
    rp->rio_cnt=0;
    rp->rio_bufptr = rp->rio_buf;

}

ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n) {
    int cnt = n;

    while (rp->rio_cnt <= 0){
        rp->rio_cnt=read(rp->rio_fd,rp->rio_buf, sizeof(rp->rio_buf));
        if(rp->rio_cnt<0){
            if(errno != EINTR) /*interrupted by sig handler return  */
                return -1;
        }
        else if (rp->rio_cnt ==0) /*EOF*/
            return 0;
        else
            rp->rio_bufptr = rp->rio_buf; /*reset buffer ptr to the start position */
    }

    if(rp->rio_cnt < cnt){
        cnt = rp->rio_cnt;
    }
    memcpy(usrbuf,rp->rio_bufptr,cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;

}


ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlenth) {
    int n, rc;
    char c, *bufp = usrbuf;

    for(n = 1; n < maxlenth; n++){
        if(rc=rio_read(rp, &c, 1)==1){
            *bufp++ = c;   /*add char  to user buffer one by one  */
            if (c == '\n'){
                n++;
                break;
            }
        }
        else if(rc == 0){
                if ( n==1 ) /*EOF,no data read*/
                    return 0;
                else
                    break;/*EOF, some data was read*/
        } else
                return -1;  /*ERROR*/
    }
    *bufp = 0;
    return n-1;
}

int open_connection(char *port) {
    struct addrinfo hints, *listp, *p; 
    int listenfd, optval=1;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG | AI_NUMERICSERV;
    getaddrinfo(NULL, port, &hints, &listp);

    for(p = listp; p; p = p->ai_next){
        if(listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol) < 0)
            continue;   /*Socket failed*/

        /*Eliminates "Address already in use" error from bind*/
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

        if((bind(listenfd, p->ai_addr, p->ai_addrlen)) == 0)
            break;  /*Success*/
        close(listenfd);
    }

    /*Clean up*/
    freeaddrinfo(listp);
    if(!p) {
        printf("(error : %s )\n","no address worked" );
        return -1;
    }
    // Listen to connections
    if((listen(listenfd, 5)) < 0){
        close(listenfd);
        printf("(error : %s )\n", "failed to listen");
        return -1;
    }

    return listenfd;

}

void handle_request(int fd, char *root) {
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE],version[MAXLINE];
    char filename[MAXLINE];
    rio_t rio;
    int type;

    rio_readinitb(&rio,fd);
    rio_readlineb(&rio,buf,MAXLINE);
    printf("request header:\n");    /*   test code .......  */
    printf("%s",buf);
    

    if(request_format_checking(fd,buf, method, uri,  version)){
        printf("( error : %s )\n", "incorrect format of request. ");
        return;
    }

    read_requesthdrs(&rio);
    type = method_type(method);
   printf("==============================\n");
    if(!type){
        error_respond(fd, 1,method, "501", "Not implemented", 
            "Webserver dose not implement this method");
        return;
    }
  
    strncpy(filename,root,100);
   strncat(filename,uri,MAXLINE);  
   if (uri[strlen(uri)-1]=='/'){
            strncat(filename,"index.html",MAXLINE);
        }

        printf("********Filename :  %s**********\n", filename);
          /*------*/
         /* URI validation */
    if(strstr(uri,"/..")){
        error_respond(fd,type,uri, "403", "Forbidden", "Webserver dosen't have the right to access parent directory");
        return;
    }

    if(stat(filename,&sbuf)<0){
        error_respond(fd,type,filename,"404","Not found","Webserver couldn't find the file");
        return;
    }
    if(!(S_ISREG(sbuf.st_mode)) ){
        error_respond(fd,type,filename, "403", "Forbidden", "Webserver couldn't read the file");
        return;
    }

    serve_file(fd,type,filename,sbuf.st_size);
    printf("======finish request handling======\n");
}


int request_format_checking(int fd,char *req, char *method, char *uri, char *version){
     char protocal[10];
     int res, type;
     res=sscanf(req,"%s %s %s",method,uri,version);
     type = method_type(method);
     if( res != 3){
            error_respond(fd,type,req,"400","Bad request",
            "the request sent by the client was syntactically incorrect");
            return 1;
     }
     
     memset(protocal, 0, sizeof(protocal));
     snprintf(protocal,6,"%s\n",version);
     if(strcmp(protocal,"HTTP/")){
         error_respond(fd,type,version,"400","Bad request",
            "the HTTP version of the request sent by client was syntactically incorrect");
           return 1;
     }
     else if(!(strstr("OPTIONSHEADGETPOSTPUTDELETETRACECONNECT",method))){
         error_respond(fd,type,method,"400","Bad request",
            "the method of the request  sent by  client was  incorrect");
            return 1;
     }
     return 0;
}

void error_respond(int fd,int type,char *cause,char *errnum, char *shortmsg, char *longmsg) {        /* imcomplete so far !!!!!  */
    char buf[MAXLINE];
    char body[MAXLINE], temp[MAXLINE];
    memset(body, 0, sizeof(buf));

/*body*/
    strncat(body, "<html><title>Error</title>",MAXLINE);
    strncat(body, "<body bgcolor=""ffffff"">\r\n",MAXLINE);
    snprintf(temp,MAXLINE,"%s: %s\r\n", errnum, shortmsg );
    strncat(body, temp,MAXLINE);
    snprintf(temp,MAXLINE,"<p>%s: %s\r\n",  longmsg , cause);
    strncat(body,temp,MAXLINE);
    strncat(body, "<hr><em>My WebServer</em>\r\n",MAXLINE);

 /*print the HTTP response  */
    snprintf(buf,MAXLINE,"HTTP/1.0 %s %s\r\n",errnum,shortmsg);
    rio_writen(fd, buf, strlen(buf));
    snprintf(buf,MAXLINE,"Content-type: text/html\r\n");
    rio_writen(fd,buf,strlen(buf));
    snprintf(buf,MAXLINE,"Content-length: %d\r\n\r\n", (int)strlen(body));
    rio_writen(fd,buf,strlen(buf));
    
    if(type==1){
    rio_writen(fd,body, strlen(body));
   }
    }

    // void head_error_respond(int fd,char *errnum, char *shortmsg){
    //     char buf[MAXLINE];

    // snprintf(buf,MAXLINE,"http/1.0 %s %s\r\n",errnum,shortmsg);
    // rio_writen(fd, buf, strlen(buf));
    // }

void read_requesthdrs(rio_t *rp) {
    char buf[MAXLINE];
    
    rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf, "\r\n")){
            rio_readlineb(rp, buf, MAXLINE);
            printf("%s",buf);
    }

    return;
}

int method_type(char *mtd) {
    int tp=0;
    if(strcasecmp(mtd,"GET")==0){
        tp=1;
    }
    else if(strcasecmp(mtd,"HEAD")==0){
        tp=2;
    }
    return tp;
}

void serve_file(int fd, int type,char *filename, int filesize) {
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXLINE];
    char content_length[MAXLINE], content_type[MAXLINE];
    memset(buf, 0, sizeof(buf));
    
    /*headers*/
    file_type(filename,filetype);
    snprintf(content_length,MAXLINE,"Content-length: %d\r\n",filesize);
    snprintf(content_type,MAXLINE,"Content-type: %s\r\n\r\n",filetype);
    strncat(buf,"HTTP/1.0 200 OK\r\n",MAXLINE);
    strncat(buf,"Server: My Web Server\r\n",MAXLINE);
    strncat(buf,"Connection: close\r\n",MAXLINE);
    strncat(buf,content_length,MAXLINE);
    strncat(buf,content_type,MAXLINE);
    printf("Response headers:\n");
    printf("%s",buf);
    if(rio_writen(fd,buf,strlen(buf))== -1){
        error_respond(fd,type,filename, "500", "Internal Server Error", 
            "Failed to send the file resource");
        return;
    }
    if(type==1){
           /*body*/
    srcfd = open(filename,O_RDONLY,0);
    srcp = mmap(0,filesize,PROT_READ, MAP_PRIVATE,srcfd,0);
    rio_writen(fd,srcp,filesize);
    close(srcfd);
    munmap(srcp,filesize);    
    }
    
    
}

void file_type(char *filename, char *filetype) {
    if (strstr(filename,".html"))
        strcpy(filetype,"text/html");
    else if (strstr(filename,".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpeg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n) {
    size_t  nleft = n;
    ssize_t  nwritten;
    char *bufp = usrbuf;

    while(nleft>0){
        if ((nwritten = write(fd,bufp,nleft))<=0){
            if (errno == EINTR)
                nwritten=0;

            else
                return -1;
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}


void sigchld_handler(int sig){
    while (waitpid(-1,0,WNOHANG)>0)
        ;
    return;
}


void create_daemon(void){
    int  fd_0,fd_1,fd_2;
    pid_t  pid;
    struct sigaction sa;

    umask (0);

    pid = fork();
    if(pid == -1){
        perror("FORK ERROR");
        exit(-1);
    }
        
    else if(pid > 0)
        exit(0);

    if(setsid() == -1){
        perror("SETSID ERROR");
        exit(-1);
    }

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGHUP, &sa, NULL) <0){
        perror("CAN NOT IGNORE SIGHUP");
        exit(-1);
    }

     if((pid = fork())== -1){
        perror("FORK ERROR");
        exit(-1);
    }

    else if (pid > 0 )
        exit(0);


    fd_0 = open("/dev/null", O_RDWR);
    fd_1 = dup(0);
    fd_2 = dup(0);

    return;
}
