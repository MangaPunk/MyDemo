//
// Created by Manganese on 2018/10/11.
//

#include "../include/MyServer.h"



int main(int argc, char **argv){
    int listenfd, connfd;
    socklen_t clientlen;
    char hostname[MAXLINE], port[MAXLINE];
    struct sockaddr_storage clientaddr;
    struct ConfigData confData;
    read_config_file(".lab3-config.txt", &confData);
     char *portn = confData.default_port;
    
    printf("root_path: %s\n", confData.root_path);
    printf("default_port: %s\n", confData.default_port);
    printf("default_concurrency_strategy: %s\n", confData.default_concurrency_strategy);


    //---------------

    struct sockaddr_in servaddr;
    char buff[MAXLINE];
    int n;

    /* check command-line args */
      if(argc == 3 && !(strcmp(argv[1],"-p"))){
          snprintf(portn,8,"%s",argv[2]);
     }
     else if (argc ==2 ){
        if(!(strcmp(argv[1],"-h"))){
            printf("========Usage========\r\n");
            printf("%s  -p  <port> : %s \n", argv[0],"bind the server to a port");
            exit(0);
        }
        else if(!(strcmp(argv[1],"-d"))){
            create_daemon();
        }
     }
      

    signal(SIGCHLD, sigchld_handler);
    //listenfd = open_connection(portn);

	if((listenfd=socket(AF_INET,SOCK_STREAM,0))==-1){

		printf("create socket failed");
		exit(0);
}
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port = htons(8008);
	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))==-1){
		printf("bind error\n");
		exit(0);}

	if(listen(listenfd,10)==-1){
		printf("listen error\n");
		exit(0);}

    
    if(listenfd<0){
        printf(">>>>>>>>>invalid port : %s<<<<<<<<<<<\n", portn);
        return;
    }
    while(1){
        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd, (SA *) &clientaddr, &clientlen);
        if(fork() ==0){
        close(listenfd);
        getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf ("Accepted connection from %s : %s\n", hostname, port);
        handle_request(connfd,confData.root_path);
        close(connfd);
        exit(0);
        }

        close(connfd);
    }

}

