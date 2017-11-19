/*
Send a file over a socket.

Interface:

    ./executable [<input_path> [<sever_hostname> [<port>]]]

Defaults:

- input_path: input.tmp
- server_hostname: 127.0.0.1
- port: 12345
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>

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

int main(int argc, char **argv) {
    ipc_command_t ipc_command;
    char protoname[] = "tcp";
    struct protoent *protoent;
    char *file_path = "input.tmp";
    char *server_hostname = "192.168.7.2";
    char *server_reply = NULL;
    char *user_input = NULL;
    char buffer[BUFSIZ];
    in_addr_t in_addr;
    in_addr_t server_addr;
    int filefd;
    int sockfd;
    ssize_t i;
    ssize_t read_return;
    struct hostent *hostent;
    struct sockaddr_in sockaddr_in;
    unsigned short server_port = 12345;

    /* Get socket. */
    protoent = getprotobyname(protoname);
    if (protoent == NULL) {
        perror("getprotobyname");
        exit(EXIT_FAILURE);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, protoent->p_proto);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    /* Prepare sockaddr_in. */
    hostent = gethostbyname(server_hostname);
    if (hostent == NULL) {
        fprintf(stderr, "error: gethostbyname(\"%s\")\n", server_hostname);
        exit(EXIT_FAILURE);
    }
    in_addr = inet_addr(inet_ntoa(*(struct in_addr*)*(hostent->h_addr_list)));
    if (in_addr == (in_addr_t)-1) {
        fprintf(stderr, "error: inet_addr(\"%s\")\n", *(hostent->h_addr_list));
        exit(EXIT_FAILURE);
    }
    sockaddr_in.sin_addr.s_addr = in_addr;
    sockaddr_in.sin_family = AF_INET;
    sockaddr_in.sin_port = htons(server_port);
    /* Do the actual connection. */
    if (connect(sockfd, (struct sockaddr*)&sockaddr_in, sizeof(sockaddr_in)) == -1) {
        perror("connect");
        return EXIT_FAILURE;
    }
    printf("Connected\n");
    char option;
    char *value = malloc(10);
    while (1) {
        printf("Commands available....\n");
        printf("SETLEDSTATE  :0\n");
        printf("SETPERIOD    :1\n");
        printf("SETDUTYCYCLE :2\n");
        printf("GETLEDSTATE  :3\n");
        printf("GETPERIOD    :4\n");
        printf("GETDUTYCYCLE :5\n");
        printf("Select the number\n");
        scanf("%s",&option);
        printf("option selected is %d",option);
        if(option <= 50)
        {
          printf("User wants to write to driver\n");
          printf("Enter the value to write\n");
          scanf("%s",value);
          if(option == 48)
          strcpy(ipc_command.led_data.ledstate, value);
          if(option == 49)
          ipc_command.led_data.period = atoi(value);
          if(option == 50)
          ipc_command.led_data.dutycycle = atoi(value);
        }
        ipc_command.command = option-48;
        if(option <= 50)
        {
          printf("sending the write command\n");
        if (write(sockfd, &ipc_command, sizeof(ipc_command)) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        printf("Sent the command\n");
        }
        if(option > 50)
        {
          printf("User wants to read from the driver\n");
        if (write(sockfd, &ipc_command, sizeof(ipc_command)) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        read_return = read(sockfd,&ipc_command,sizeof(ipc_command));
        if(read_return == -1)
        {
          perror("read");
          exit(EXIT_FAILURE);
        }
        printf("recieved from the server data %d %d %s\n",ipc_command.led_data.dutycycle,ipc_command.led_data.period,ipc_command.led_data.ledstate);
        }
    }
    free(user_input);
    free(server_reply);
    close(filefd);
    exit(EXIT_SUCCESS);
}
