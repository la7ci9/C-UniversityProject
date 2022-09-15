#include "laciproj.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#define PORT_NO 3333
#define BUFSIZE 1024 
int s;

void SignalHandler(int sig)
{
    if(sig == SIGINT)
    {
        printf("Goodbye..\n");
        exit(0);
    }

    if(sig == SIGUSR1)
    {
        printf("File transfer service is not available..");
    }

    if(sig == SIGALRM)
    {
        printf("Server is not responding");
        exit(1);
    }
}

void help()
{
    printf("**************\n**   HELP   **\n**************\n");
    printf("Commands:\n");
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            printf("--version -> Program version, author\n\n");
        }
        #pragma omp section
        {
            printf("--help -> Show all commands\n\n");
        }
        #pragma omp section
        {
            printf("-send -> Program mode(via file) [default]\n\n");
        }
        #pragma omp section
        {
            printf("-receive -> Program mode(via file)\n\n");
        }
        #pragma omp section
        {
            printf("-file -> Communication type(send mode) [default]\n\n");
        }
        #pragma omp section
        {
            printf("-socket -> Communication type(send mode)\n\n");
        }
    }
    exit(0);
}

void sendViaFile(int *Values, int NumValues)
{
    FILE *file = fopen("Measurement.txt", "w");

    for (int i = 0; i < NumValues; i++)
    {
        fprintf(file, "%d\n", Values[i]);
    }
    fclose(file);

    if (FindPID() == -1)
    {
        printf("can't find host mode working process\n");
        exit(1);
    }
    else
    {
        kill(SIGUSR1, FindPID());
    }
    
}

void receiveViaFile(int sig)
{
    FILE *file = fopen("Measurement.txt","r");
    char *line;
    int i = 0;
    int size = 1;
    int *data = (int *)malloc(sizeof(int));

    while(fgets(line, sizeof(line), file) != NULL) 
    {
        if(i==size)
        {
            size++;
            data =(int*)realloc(data,size*sizeof(int));
        }
        data[i] = atoi(line);
        i++;
    }
    BMPcreator(data,size);
    free(data);
}

void sendViaSocket(int *Values, int NumValues)
{
    /************************ Declarations **********************/
    int bytes; // received/sent bytes
    int flag;  // transmission flag
    char on;   // sockopt option
    int sendbuffer[1];
    int buffer[1];          // datagram buffer area
    unsigned int server_size;  // length of the sockaddr_in server
    struct sockaddr_in server; // address of server

    /************************ Initialization ********************/
    on = 1;
    flag = 0;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(PORT_NO);
    server_size = sizeof server;

    /************************ Creating socket *******************/
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
    {
        fprintf(stderr, " %s: Socket creation error.\n", "chart");
        exit(2);
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

    /************************ Sending data **********************/
    printf(" Message send: ");
    sendbuffer[0] = NumValues;
    bytes = sendto(s, sendbuffer, sizeof(int) + 1, flag, (struct sockaddr *)&server, server_size);
    if (bytes <= 0)
    {
        fprintf(stderr, " %s: Sending error.\n", "chart");
        exit(3);
    }
    printf(" %i bytes have been sent to server.\n", bytes - 1);
    signal(SIGALRM,SignalHandler);
    alarm(1);
    /************************ Receive data **********************/
    bytes = recvfrom(s, buffer, sizeof(int) + 1, flag, (struct sockaddr *)&server, &server_size);
    if (bytes < 0 && sendbuffer[0] == buffer[0])
    {
        fprintf(stderr, " %s: Receiving error.\n", "chart");
        exit(4);
    }
    printf(" Server's (%s:%d) acknowledgement:\n  %ls\n",
    inet_ntoa(server.sin_addr), ntohs(server.sin_port), buffer);
    alarm(0);

    /************************Second sending data **********************/
    printf(" Message to send: ");
    bytes = sendto(s, Values, NumValues * sizeof(int) + 1 , flag, (struct sockaddr *)&server, server_size);
    if (bytes <= 0)
    {
        fprintf(stderr, " %s: Sending error.\n", "chart");
        exit(3);
    }
    printf(" %i bytes have been sent to server.\n", bytes - 1);

    /************************Second receive data **********************/
    bytes = recvfrom(s, buffer, sizeof(int) + 1, flag, (struct sockaddr *)&server, &server_size);
    if (bytes < 0 && (sizeof(int) * NumValues) == buffer[0])
    {
        fprintf(stderr, " %s: Receiving error.\n", "chart");
        exit(4);
    }
    printf(" Server's (%s:%d) acknowledgement:\n  %ls\n",inet_ntoa(server.sin_addr), ntohs(server.sin_port), buffer);

    

    close(s);
}

//Nincs kész! :(
void receiveViaSocket()
{
    /************************ Declarations **********************/
    int bytes;                 // received/sent bytes
    int err;                   // error code
    int flag;                  // transmission flag
    char on;                   // sockopt option
    int buffer[BUFSIZE];          // datagram buffer area
    unsigned int server_size;  // length of the sockaddr_in server
    unsigned int client_size;  // length of the sockaddr_in client
    struct sockaddr_in server; // address of server
    struct sockaddr_in client; // address of client

    /************************ Initialization ********************/
    on = 1;
    flag = 0;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT_NO);
    server_size = sizeof server;
    client_size = sizeof client;
    int *Array;

    unsigned long int counter = 1;
    /************************ Creating socket *******************/
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
    {
        fprintf(stderr, " %s: Socket creation error.\n", "chart");
        exit(2);
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

    /************************ Binding socket ********************/
    err = bind(s, (struct sockaddr *)&server, server_size);
    if (err < 0)
    {
        fprintf(stderr, " %s: Binding error.\n", "chart");
        exit(3);
    }

    while (1)
    { 
        /************************ Receive data **********************/
        printf("\n Waiting for a message...\n");
        bytes = recvfrom(s, buffer, BUFSIZE , flag, (struct sockaddr *)&client, &client_size);
        if (bytes < 0)
        {
            fprintf(stderr, " %s: Receiving error.\n", "chart");
            exit(4);
        }
        printf(" %d bytes have been received from the client (%s:%d).\n ",bytes - 1, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        /************************ Sending data **********************/

        //BEFEJEZNI 

    }
}

int Measurement(int **Values)
{
    struct tm* local;
    time_t t = time(NULL);
    local = localtime(&t); 
    int min_in_sec = (local->tm_min * 60 ) - ((local->tm_min / 15) * 15 )* 60;
	int actualtime = min_in_sec + local->tm_sec;
    int maxElements = 0;
    if (actualtime < 100)
    {
        maxElements = 100;
    } 
    else
    {
        maxElements = actualtime;
    }

    srand(time(NULL));
    int *array;

    array = malloc(maxElements * sizeof(int));
    array[0] = 0;

    for (int i = 1; i < maxElements; i++)
    {
        float randNum = (float) rand()/(float)RAND_MAX;
        if (randNum < 0.428571)
        {
            array[i] = array[i-1] + 1;
        }
        else if (randNum > 0.428571 && randNum < 0.78340971) // 1-((1-0.428571)-(11/31))
        {
            array[i] = array[i-1] - 1;
        }
        else
        {
            array[i] = array[i-1];
        }
        //printf("%d\n",array[i]);
    }
    *Values = array;
    //BMPcreator(*Values,maxElements);
    
    return maxElements;
}

//3.feladat segítség
typedef struct BitMapFileHeader{
	const unsigned short Reserve;
	const unsigned short FileType;// = 0x424D; //BM
	unsigned int   FileSize;
	const unsigned short Reserve1;// = 0x00;
	const unsigned short Reserve2;// = 0x00;
	const unsigned int   DateOffSet;//  = 0x3E;
} BMPFileHeader;

typedef struct BitMapInfoHeader{
	const unsigned int   ThisSize;// = 0x28;
	int   ImgWidth;
	int   ImgHeight;
	const unsigned short PlainNum;// = 0x01;
	const unsigned short BitPerPx;// = 0x01;
	const unsigned int   CompressType;// = 0x00;
	unsigned int   ImgDateSize;
	const unsigned int   HorizontalDpi;// = 0x00;
	const unsigned int   VerticalDpi;// = 0x00;
	const unsigned int   ColorIndex;// = 0x02;
	const unsigned int   PrincipalIndex;// = 0x00;
} BMPInfoHeader;

typedef struct ColorPalette{
	const unsigned char Red;
	const unsigned char Green;
	const unsigned char Blue;
	const unsigned char Reserve;
} ColorPalette;
//3.feladat segítség


void BMPcreator(int *Values, int NumValues)
{
    int matrix[NumValues][NumValues];
    memset(matrix, 0, sizeof(matrix));
    for (int i = 0; i < NumValues; i++)
    {
        matrix[(NumValues/2)-Values[i]][i] = 1;
    }
    /*for (int i = 0; i < NumValues; i++)
    {
        for (int j = 0; j < NumValues; j++)
        {
            printf("%d ",matrix[i][j]);
        }
        printf("\n");
    }*/

    BMPInfoHeader bmpIH = {
        0x28,  //ThisSize
		NumValues,  //ImgWidth
		NumValues,  //ImgHeight
		0x01,  //PlainNum
		0x01,  //BitPerPx
		0x00,  //CompressType
		0x54,  //ImgDateSize
		0x00,  //HorizontalDpi
		0x00,  //VerticalDpi
		0x00,  //ColorIndex
		0x00   //PrincipalIndex
    };
    BMPFileHeader bmpFH = {
        0x00,
		0x4D42, //signature
		0x00,   //fileSize
		0x00,   //Reserve 1
		0x00,   //Reserve 2
		0x36   //PixelArrayOffSet
	};
    
    ColorPalette  colP[2] = {
		{ 0x17, 0xB1, 0xFB },   //Yellow
		{ 0x00, 0x00, 0x00 }    //Black
	};
    
	unsigned int BytePerImgLine = (NumValues +31) *8;

    if (NumValues % 8 != 0)
    {
       bmpIH.ImgDateSize = (NumValues/8 + 1) * 8 * BytePerImgLine;
	  

    }
    else
    {
        bmpIH.ImgDateSize = NumValues * BytePerImgLine;
    }
    
    bmpFH.FileSize =bmpFH.DateOffSet + bmpIH.ImgDateSize;
    FILE *file = fopen("chart.bmp","w");
	fwrite((char*)&bmpFH + 2, sizeof(BMPFileHeader)-2, 1, file);



	fwrite(&bmpIH, sizeof(BMPInfoHeader), 1, file);
	fwrite(colP, 1, 8, file);
    

    for(int row = NumValues-1; row >= 0; row--)
    {
        if (NumValues % 32 ==0)
        {
            for (int i = 0; i < NumValues; i= i+8)
            {
                int bits = bintoint(&matrix[row][i],8);
                fwrite(&bits,1,1,file);
            }
        }
        else
        {
            for (int i = 0; i < ((NumValues/32)+1)*32; i= i+8)
            {
                int bits = bintoint(&matrix[row][i],8);
                fwrite(&bits,1,1,file);
            }
        }
        
        
        
    }

	fclose(file);
}

int bintoint(int *array,int size)
{
   int result = 0;
    int num = 1;
    for (int i = size; i > 0; i--)
    {
        result += array[i] * num;
        num *= 2;
    }
    
    return result;
}

//Nem sikerült megoldani :(
int FindPID()
{
    DIR *d;
    struct dirent *dir;
    char start[5] = "/proc";
    d = opendir(start);
    char filePID[256];
    while ((dir = readdir(d)) != NULL)
    {
        if(isdigit(dir->d_name[0]))
        {
           // printf("%s\n",dir->d_name);
            //char currentDir[512] ="/proc/";
            //char *name = strcat(currentDir, strcat(dir->d_name, "/status"));

           // printf("%s\n",name);
/*
            FILE *file;
            file = fopen(name,"r");

            //          VALAMIÉRT SEGMENTATION FAULTRA FUT, JAVITANI KELL

            char buff[256];
            char *firstLine = fgets(buff, sizeof(buff),file);

            */
            //printf("%s\n",firstLine);

        
            //fclose(file);
        }
    }
    //closedir(d);
    
    return 0;
}