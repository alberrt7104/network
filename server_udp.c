#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#define SERVER_PORT 5432
#define MAX_LINE 256

int main(int argc, char * argv[])
{
    char *fname;
    char buf[MAX_LINE];
    int ack;
    struct sockaddr_in sin;
    int len;
    int s, i;
    struct timeval tv;
    char seq_num = 1; 
    FILE *fp;
    char message[MAX_LINE];
    struct package{
    int seq;    
    char buf[MAX_LINE];
    };
    int check=0;
    int getseq;
    int slen;
    struct package pac;
    struct package re;
    if (argc==2) {
        fname = argv[1];
    }

    else {
        fprintf(stderr, "usage: ./server_udp filename\n");
        exit(1);
    }


    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(SERVER_PORT);

    /* setup passive open */
    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("simplex-talk: socket");
        exit(1);
    }
    if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
        perror("simplex-talk: bind");
        exit(1);
    }

    socklen_t sock_len = sizeof sin;

    fp = fopen(fname, "w");
    if (fp==NULL){
        printf("Can't open file\n");
        exit(1);
    }
    
    while(1){
       // recvfrom(s, &pac, sizeof(pac), 0, (struct sockaddr *)&sin, &sock_len);
        len = recvfrom(s, &pac, sizeof(pac), 0, (struct sockaddr *)&sin, &sock_len);
         
	    strcpy(re.buf,pac.buf);
            re.seq=pac.seq+1;
            slen = strlen(pac.buf); 
            pac.buf[slen] ='\0';
            sendto(s, &re, sizeof(re)+1, 0, (struct sockaddr *)&sin, sock_len);
       
          //message[0]=pac.seq+1;
          //sendto(s, &message[0], sizeof(char), 0, (struct sockaddr *)&sin, sock_len);
          if (pac.buf[0] == 0x02){
                printf("Transmission Complete\n");
                break;
            }
          else if(check==pac.seq){
          check=check+1;
          fputs((char *) pac.buf, fp);
        }
        
        
      

    }
    fclose(fp);
    close(s);
}
