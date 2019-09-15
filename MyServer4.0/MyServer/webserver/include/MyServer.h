//
// Created by Manganese on 2018/10/11.
//

#ifndef LAB_2_MYSERVER_H
#define LAB_2_MYSERVER_H



#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <limits.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>



#define	MAXLINE	 8192  /* Max text line length */
#define MAXURI	100 /* Max uri length */

#define RIO_BUFSIZE    8192
typedef struct {
    int rio_fd;                /* Descriptor for this internal buf */
    int rio_cnt;               /* Unread bytes in internal buf */
    char *rio_bufptr;          /* Next unread byte in internal buf */
    char rio_buf[RIO_BUFSIZE]; /* Internal buffer */
} rio_t;

typedef struct sockaddr SA;

struct ConfigData{
    char root_path[MAXLINE];
    char default_port[MAXLINE];
    char default_concurrency_strategy[MAXLINE];
};

void read_config_file(const char filename[], struct ConfigData * confData);

/* Rio (Robust I/O) package */

void rio_readinitb(rio_t *rp, int fd);
static ssize_t	rio_read(rio_t *rp, char *usrbuf, size_t n);
ssize_t	rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);



/* main handle functions */
int open_connection(char *port);
void handle_request(int fd, char *root);

/*show error */
void error_respond(int fd,int type,char *cause,char *errnum, char *shortmsg, char *longmsg);
// void head_error_respond(int fd,char *errnum, char *shortmsg);
int request_format_checking(int fd, char *req, char *method, char *uri, char *version);

/*signal handling*/
void sigchld_handler(int sig);

void read_requesthdrs(rio_t *rp);
int method_type(char *mtd);


void serve_file(int fd,int type,char *filename, int filesize);
void file_type(char *filename, char *filetype);

void create_daemon(void);

#endif //LAB_2_MYSERVER_H
