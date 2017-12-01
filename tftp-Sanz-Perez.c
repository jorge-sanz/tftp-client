// Practica tema 7, Sanz Perez Jorge

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define BUFFER_SIZE 516

int v_mode = 0; /* 1 for v mode active, which means that the program will print its trace in standard output. */

/* error: wrapper for perror */
void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

/* checkProgramAguments */
void checkProgramArguments(int argc, char *argv[])
{
    if (argc == 5)
    {
        /* if there are 5 arguments and the 5th one is -v then v mode is active */
        if (strcmp(argv[4], "-v") == 0)
        {
            v_mode = 1;
            printf("V mode is active: trace will be shown during the execution.\n");
        }
    }
    /* exit failure. program should have 4 arguments at least. */
    else if (argc < 4)
    {
        fprintf(stderr,
                "Usage: %s server-ip {-r|-w} file [-v]\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    int udp_socket;                      /* udp socket */
    int port;                            /* server port */
    struct sockaddr_in server;           /* server information */
    struct servent *application;         /* tftp application information */
    int end;                             /* flag to mark if we are done with file transfer */
    int is_first_datagram = 1;           /* flag to mark if we are dealing with the first datagram */
    int block;                           /* datagram block number. tracking purposes */
    int received_block;                  /* server received datagram */
    char datagram[BUFFER_SIZE];          /* datagrams */
    int datagram_size;                   /* datagram size */
    FILE *file;                          /* file */
    int opcode;                          /* operation code */
    int errcode;                         /* error code */
    char errstring[512];                 /* error description */
    int received_bytes;                  /* number of bytes received from server */
    int sending_size;                    /* sending data size */
    unsigned char *sending_block_number; /* block numbering for writing mode */

    printf("-- Welcome to TFTP client --\n");

    checkProgramArguments(argc, argv);

    /* create UDP socket */
    if ((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        error("ERROR opening UDP socket");
    }

    /* get tftp port number */
    application = getservbyname("tftp", "udp");
    if (!application)
    {
        error("ERROR unknown application tftp");
    }
    else
    {
        port = application->s_port;
    }

    /* set up server */
    server.sin_family = AF_INET;                 /* Internet Domain    */
    server.sin_port = port;                      /* Server Port        */
    server.sin_addr.s_addr = inet_addr(argv[1]); /* Server's Address   */

    /* read mode */
    if (strcmp(argv[2], "-r") == 0)
    {
        /* read loop */
        while (!end)
        {
            if (is_first_datagram == 1)
            {
                if (v_mode)
                {
                    printf("%s file read request sent to %s tftp server\n", argv[3], argv[1]);
                }

                /* block number 0 */
                block = 0;

                /* create read request (RRQ) datagram */
                /* define opcode (two first bytes) */
                datagram[0] = 0;
                datagram[1] = 1;
                strcpy(datagram + 2, argv[3]);                     /* define file name */
                strcpy(datagram + (strlen(argv[3]) + 3), "octet"); /* define octet mode */

                if ((file = fopen(argv[3], "w")) == NULL)
                {
                    error("ERROR creating read mode file");
                }
            }
            else /* not the first datagram */
            {
                if (v_mode)
                {
                    if (block == 1)
                    {
                        printf("It's first block (number of block 1)\n");
                    }
                    else
                    {
                        printf("Block number %d\n", block);
                    }
                    printf("Sending block %d ACK\n", block);
                }

                received_block = (unsigned char)datagram[3] + ((unsigned char)datagram[2] << 8);
                if (received_block != block)
                {
                    error("ERROR with reiceived block. unexpected block");
                }

                /* preparing ACK datagram */
                datagram[0] = 0;
                datagram[1] = 4;
            }

            datagram_size = 2 + strlen(argv[3]) + 1 + strlen("octet") + 1; /* defining datagram size by adding component sizes */

            /* send datagram to server: request or ACK message */
            if (sendto(udp_socket, datagram, sizeof(char) * datagram_size, 0, (struct sockaddr *)&server, sizeof(server)) < 0)
            {
                error("ERROR sendto");
            }

            /* get server response */
            socklen_t server_length = sizeof(server);
            if ((received_bytes = recvfrom(udp_socket, datagram, BUFFER_SIZE, 0, (struct sockaddr *)&server, &server_length)) < 0)
            {
                error("ERROR in recvfrom");
            }

            block++;
            if (v_mode)
            {
                printf("Block received from tftp server\n");
            }

            /* check if it's an error package */
            opcode = datagram[1];
            if (opcode == 5)
            {
                errcode = datagram[3];
                strcat(errstring, datagram + 4);
                printf("Errcode %d: %s\n", errcode, errstring);
                fclose(file);
                remove(argv[3]);
                exit(EXIT_FAILURE);
            }

            /* write in file */
            fwrite(datagram + 4, sizeof(char), received_bytes - 4, file);

            /* close file and finish program if it's the last block */
            if (received_bytes < 516)
            {
                if (v_mode)
                {
                    printf("Block %d is the last one. We close the file.\n", block);
                }

                if (fclose(file) == EOF)
                {
                    error("ERROR in fclose");
                }
                end = 1;
            }
            is_first_datagram = 0;
        }
    }
    /* write mode */
    else if (strcmp(argv[2], "-w") == 0)
    {
        /* write loop */
        while (end == 0)
        {
            /* check if it's first datagram */
            if (is_first_datagram)
            {
                if (v_mode)
                {
                    printf("%s file write request sent to %s tftp server\n", argv[3], argv[1]);
                }

                /* open local file for read */
                if ((file = fopen(argv[3], "r")) == NULL)
                {
                    error("ERROR in read file opening");
                }

                block = 0;

                /* create write request (WRQ) datagram */
                /* define opcode (two first bytes) */
                datagram[0] = 0;
                datagram[1] = 2;
                strcpy(datagram + 2, argv[3]);
                strcpy(datagram + (strlen(argv[3])) + 3, "octet");
                datagram_size = 2 + strlen(argv[3]) + 1 + strlen("octet") + 1;
            }
            /* deal with non first datagrams */
            else
            {
                if (v_mode)
                {
                    if (block == 1)
                    {
                        printf("It's first block (number of block 1)\n");
                    }
                    else
                    {
                        printf("Block number %d\n", block);
                    }
                    printf("Sending block %d ACK\n", block);
                }

                /* get to send data size */
                if (!feof(file))
                {
                    /* calculate sending datagram size: 4 bytes for opcode and block number, the rest of them for data */
                    sending_size = fread(datagram + 4, sizeof(char), 512, file);
                }
                else
                {
                    sending_size = 0;
                }

                /* declaring datagram as data datagram */
                datagram[1] = 3;

                /* add block number in datagram to be sent */
                sending_block_number = (unsigned char *)&block;
                datagram[2] = sending_block_number[1];
                datagram[3] = sending_block_number[0];

                datagram_size = 2 + 2 + sending_size;

                /* check if received block is correct */
                received_block = (unsigned char)datagram[3] + ((unsigned char)datagram[2] << 8);
                if (received_block != block)
                {
                    error("ERROR with reiceived block. unexpected block");
                }
            }

            /* send read data datagram */
            sending_size = sendto(udp_socket, datagram, sizeof(char) * datagram_size, 0, (struct sockaddr *)&server, sizeof(server));
            if (sending_size < 0)
            {
                error("ERROR in sendto");
            }

            /* receive server ACK */
            socklen_t server_length = sizeof(server);
            if ((received_bytes = recvfrom(udp_socket, datagram, BUFFER_SIZE, 0, (struct sockaddr *)&server, &server_length)) < 0)
            {
                error("ERROR in recvfrom");
            }

            if (v_mode)
            {
                printf("ACK received from TFTP server %s\n", argv[1]);
            }

            /* Check if client has received an error datagram */
            opcode = datagram[1];
            if (opcode == 5)
            {
                errcode = datagram[3];
                strcat(errstring, datagram + 4);
                printf("Errcode %d: %s\n", errcode, errstring);
                exit(EXIT_FAILURE);
            }

            /* closing file in case that we have sent last datagram */
            if (!is_first_datagram && (sending_size < BUFFER_SIZE))
            {
                if (v_mode)
                {
                    printf("Block %d is the last one. We close the file.\n", block);
                }

                if (fclose(file) == EOF)
                {
                    error("ERROR in fclose");
                }
                end = 1;
            }

            block++;
            is_first_datagram = 0;
        }
    }
    /* deal with other kind of operations */
    else
    {
        fprintf(stderr,
                "Usage: %s server-ip {-r|-w} file [-v]\n",
                argv[0]);
        close(udp_socket);
        exit(EXIT_FAILURE);
    }

    close(udp_socket);
    return 0;
}
