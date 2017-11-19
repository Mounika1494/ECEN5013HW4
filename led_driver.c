#include<stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/socket.h>

typedef enum {SETLEDSTATE,SETPERIOD,SETDUTYCYCLE,GETLEDSTATE,GETPERIOD,GETDUTYCYCLE}command_t;
typedef enum {SUCCESS,ERROR}return_type_t;

typedef struct {
       uint16_t period;
       uint8_t dutycycle;
       char ledstate[7];
}data_t;

typedef struct
{
  command_t command;
  data_t led_data;
}ipc_command_t;

return_type_t write_led_driver(command_t command,data_t data)
{
       char* path = malloc(40);
       strcpy(path,"/sys/ebb/led53/");
       FILE* fp = NULL;
       char* buf = malloc(20);
       switch(command)
       {
              case SETPERIOD:
                            strcat(path,"blinkPeriod");
                            printf("opening file %s\n",path);
                            sprintf(buf,"%d",data.period);
                            printf("Value written %s\n",buf);
                            if((fp = fopen(path,"w+")) != NULL)
                            {
                                   printf("Value written %s\n",buf);
                                   if((fwrite(buf,sizeof(char),strlen(buf),fp))!= -1)
                                   {
                                          printf("Successfully changed the period:%d",data.period);
                                   }
                                   else
                                   {
                                   printf("file write error\n");
                                   fclose(fp);
                                   return ERROR;
                                   }
                                   fclose(fp);
                            }
                            else
                            {
                                   printf("Not able to open file\n");
                                   return ERROR;
                            }
                            
                            break;
                            
              case SETDUTYCYCLE:
                            strcat(path,"blinkDutyCycle");
                            printf("opening file %s\n",path);
                            sprintf(buf,"%d",data.dutycycle);
                            printf("Value written %s\n",buf);
                            if((fp = fopen(path,"w+")) != NULL)
                            {      
                                   printf("Value written %s\n",buf);
                                   if((fwrite(buf,sizeof(char),strlen(buf),fp))!= -1)
                                   {
                                          printf("Successfully changed the dutycycle:%d",data.dutycycle);
                                   }
                                   else
                                   {
                                    fclose(fp);      
                                   printf("file write error\n");
                                   return ERROR;
                                   }
                                   fclose(fp);
                            }
                            else
                            {
                                   printf("Not able to open file\n");
                                   return ERROR;
                            }
                            
                            break;
                            
             case SETLEDSTATE:
                            strcat(path,"led_state");
                            printf("opening file %s\n",path);
                            sprintf(buf,"%s",data.ledstate);
                            printf("Value written %s\n",buf);
                            if((fp = fopen(path,"w+")) != NULL)
                            {
                                   printf("Value written %s\n",buf);
                                   if((fwrite(buf,sizeof(char),strlen(buf),fp))!= -1)
                                   {
                                          printf("Successfully changed the ledstate:%d",data.ledstate);
                                   }
                                   else
                                   {
                                   printf("file write error\n");
                                   fclose(fp);
                                   return ERROR;
                                   }
                                   fclose(fp);
                            }
                            else
                            {
                                   printf("Not able to open file\n");
                                   return ERROR;
                            }
                     
                            break;
       }
      return SUCCESS;
       
}

return_type_t read_led_driver(command_t command,data_t *data)
{
       char* path = malloc(40);
       strcpy(path,"/sys/ebb/led53/");
       FILE* fp = NULL;
       char* buf = malloc(20);
       switch(command)
       {
              case GETPERIOD:
                            strcat(path,"blinkPeriod");
                            if((fp = fopen(path,"r")) != NULL)
                            {
                                   if((fread(buf,sizeof(char),20,fp)) > 0)
                                   {
                                          data->period = atoi(buf);
                                          printf("Successfully read the period:%s",buf);
                                   }
                                   else
                                   {
                                   printf("file write error\n");
                                   fclose(fp);
                                   return ERROR;
                                   }
                                   fclose(fp);
                            }
                            else
                            {
                                   printf("Not able to open file\n");
                                   return ERROR;
                            }
                            break;
                            
              case GETDUTYCYCLE:
                            bzero(buf,20);
                            strcat(path,"blinkDutyCycle");
                            if((fp = fopen(path,"r")) != NULL)
                            {
                                   if((fread(buf,sizeof(char),20,fp)) > 0)
                                   {
                                          data->dutycycle = atoi(buf);
                                          printf("Successfully read the dutycycle:%s",buf);
                                   }
                                   else
                                   {
                                   printf("file write error\n");
                                   fclose(fp);
                                   return ERROR;
                                   }
                                   fclose(fp);
                            }
                            else
                            {
                                   printf("Not able to open file\n");
                                   return ERROR;
                            }
                            break;
                            
              case GETLEDSTATE:
                            bzero(buf,20);
                            strcat(path,"led_state");
                            if((fp = fopen(path,"r")) != NULL)
                            {
                                   if((fread(buf,sizeof(char),20,fp)) > 0)
                                   {
                                          strcpy(data->ledstate,buf);
                                          printf("Successfully changed the ledstate:%s",buf);
                                   }
                                   else
                                   {
                                   printf("file write error\n");
                                   fclose(fp);
                                   return ERROR;
                                   }
                                   fclose(fp);
                            }
                            else
                            {
                                   printf("Not able to open file\n");
                                   return ERROR;
                            }
                            break;
                            
       }
       free(path);
       free(buf);
       
       
}



int main(int argc,char **argv)
{
    char buffer[30];
    ipc_command_t ipc_command;
    char protoname[] = "tcp";
    int client_sockfd;
    int enable = 1;
    int filefd;
    int i;
    int server_sockfd;
    socklen_t client_len;
    ssize_t read_return;
    struct protoent *protoent;
    struct sockaddr_in client_address, server_address;
    unsigned short server_port = 12345u;

    if (argc == 1) {
            server_port = strtol(argv[1], NULL, 10);
        }

    /* Create a socket and listen to it.. */
    protoent = getprotobyname(protoname);
    if (protoent == NULL) {
        perror("getprotobyname");
        exit(EXIT_FAILURE);
    }
    server_sockfd = socket(AF_INET,SOCK_STREAM,protoent->p_proto);
    if (server_sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(server_port);
    if (bind(server_sockfd,(struct sockaddr*)&server_address,sizeof(server_address)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_sockfd, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "listening on port %d\n", server_port);

    while (1) {
        client_len = sizeof(client_address);
        puts("waiting for client");
        client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_address,&client_len);
        printf("accepted the connection from client\n");
        do{
            read_return = read(client_sockfd, &ipc_command, sizeof(ipc_command));
            if (read_return == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }
            printf("recieved from client %d and %d\n",ipc_command.command,ipc_command.led_data.period);
            if(ipc_command.command <= 2)
            write_led_driver(ipc_command.command,ipc_command.led_data);
            if(ipc_command.command > 2)
            {
            read_led_driver(ipc_command.command,&ipc_command.led_data);
            if (write(client_sockfd, &ipc_command, sizeof(ipc_command)) == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            }
        }while(read_return >0);
        close(client_sockfd);
     /*  data_t data;
       data_t readdata;
       read_led_driver(GETPERIOD,&readdata);
       read_led_driver(GETDUTYCYCLE,&readdata);
       read_led_driver(GETLEDSTATE,&readdata);
       printf("period is %d\n",readdata.period);
       printf("dutycycle is %d\n",readdata.dutycycle);
       printf("led state is %s\n",readdata.ledstate);
       data.period = 500;
       write_led_driver(SETPERIOD,data);
       data.dutycycle = 30;
       write_led_driver(SETDUTYCYCLE,data);
       strcpy(data.ledstate,"led on");
       write_led_driver(SETLEDSTATE,data);*/
    }
}