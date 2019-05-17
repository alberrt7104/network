#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 5432
#define MAX_LINE 80

int main(int argc, char * argv[])
{
    FILE *fp;
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host;
    char *fname;
    char buf[MAX_LINE];
    int s;
    int slen;
    
    char message[MAX_LINE];
    message[0]=10;
    
    struct timeval tv;
    tv.tv_sec=0;
    tv.tv_usec=1000;
    
    int count=0;
    int arriveack=0;
    int data=0;
    struct package{
    int seq;
    char buf[MAX_LINE];

    };
    struct package pac;
    struct package sw[10]; //slisewindow
    struct package re;
    if (argc==3) {
        host = argv[1];
        fname= argv[2];
    }
    else {
        fprintf(stderr, "Usage: ./client_udp host filename\n");
        exit(1);
    }
    /* translate host name into peer¡¦s IP address */
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "Unknown host: %s\n", host);
        exit(1);
    }

    fp = fopen(fname, "r");
    if (fp==NULL){
        fprintf(stderr, "Can't open file: %s\n", fname);
        exit(1);
    }

    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(SERVER_PORT);

    /* active open */
    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket");
        exit(1);
    }

    socklen_t sock_len= sizeof sin;
    
    /* main loop: get and send lines of text */
    while(fgets(pac.buf, 80, fp) != NULL){
        slen = strlen(pac.buf);
                
        pac.buf[slen] ='\0';
        strcpy(sw[data].buf,pac.buf);
        sw[data].seq=count;
        pac.seq=count;
        //printf("sw seq:%d \n",sw[data].seq);
        //printf("pac seq:%d \n",pac.seq);
       // printf("%s",pac.buf);
        //printf("%s",sw[data].buf);
        sendto(s,&pac, sizeof(pac)+1, 0, (struct sockaddr *)&sin, sock_len);
        
     

          if(count>8){
            if (setsockopt(s,SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
              perror("PError");
            }
              recvfrom(s, &re, sizeof(re), 0, (struct sockaddr *)&sin, &sock_len); 
            //recvfrom(s, &message[0], sizeof(char), 0, (struct sockaddr *)&sin, &sock_len);
           // printf("now: %d \n",arriveack);
            //printf("ack: %d \n",re.seq);
            //printf("pu : %s \n",re.buf);
            if(arriveack+1==re.seq){
            arriveack=arriveack+1;  
            //printf("arrive: %d \n",arriveack); 
            } 
            else{
              while(arriveack+1!=re.seq){
                for(int i=0;i<10;i++){
                sw[i].buf[slen] ='\0';
                sendto(s,&sw[i], sizeof(sw[i])+1, 0, (struct sockaddr *)&sin, sock_len);    
                   // printf("resend \n");
                }
                if (setsockopt(s,SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
              perror("PError");
              }
               recvfrom(s, &re, sizeof(re), 0, (struct sockaddr *)&sin, &sock_len); 
             // recvfrom(s, &message[0], sizeof(char), 0, (struct sockaddr *)&sin, &sock_len);
             // printf("re now: %d \n",arriveack);
             // printf("re ack: %d \n",re.seq);
              //printf("re pu : %s \n",re.buf);
             }

             arriveack=arriveack+1;
               }
         }

        if(sendto(s,&pac, sizeof(pac), 0, (struct sockaddr *)&sin, sock_len)<0){
            perror("SendTo Error\n");
            exit(1);
        }
        count=count+1;
        data=data+1;
        if(data==10) data=0;

    }
    //if (setsockopt(s,SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
    //perror("PError");
//}
    //ack=recvfrom(s, &message[0], sizeof(char), 0, (struct sockaddr *)&sin, &sock_len);
    //printf("%d ",message[0]);
    //printf("1234");
    *pac.buf = 0x02; 
         
  
    
        if(sendto(s, &pac,sizeof(pac) , 0, (struct sockaddr *)&sin, sock_len)<0){
        perror("SendTo Error\n");
        exit(1);
    }
    fclose(fp);
}
