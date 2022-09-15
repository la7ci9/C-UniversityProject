#include <stdio.h>
#include "laciproj.h"
#include <stdlib.h>
#include <string.h>
#include <signal.h>

void signal_catch(int sig)
{
    receiveViaFile();
}

void receive_fun()
{
    signal(SIGUSR1,signal_catch); // Register signal handler
    for(int i=1;;i++)
    {    
        //Infinite loop
    }
}

int main(int argc, char* argv[]){

    if(strcmp(argv[0],"./chart")!=0)
    {
        
        printf("Please, use >chart< name to compile\n");
        exit(0);
    }
    if (argc==1 || argc > 3)
    {
        help();
    }
    
    int *start;
    int numbers;
    //Measurement(&start);
    if (argc==2)
    {
        if(strcmp(argv[1],"--version")==0)
        {
            printf("Version: 0.1.0\nAuthor: Kiss Laszlo\n");
            exit(0);
        }
        else if(strcmp(argv[1],"--help")==0)
        {
            help();
        }
        else if(strcmp(argv[1],"-send")==0)
        {
            numbers = Measurement(&start);
            sendViaFile(start,numbers);
            free(start);
            exit(0);
        }
        else if(strcmp(argv[1],"-receive")==0)
        {
            receive_fun();
        }
        else if(strcmp(argv[1],"-file")==0)
        {
            sendViaFile();
        }
        else if (strcmp(argv[1],"-socket")==0)
        {
            sendViaSocket();
        }
        else
        {
            help();
        }
    }
    if (argc==3)
    {
        if ( ((strcmp(argv[1], "-send")==0) && (strcmp(argv[2], "-file")==0)) || ((strcmp(argv[1], "-file")==0) && (strcmp(argv[2], "-send")==0)) )
        {
            numbers = Measurement(&start);
            sendViaFile(start,numbers);
            free(start);
            exit(0);
        }
        else if ( ((strcmp(argv[1], "-send")==0) && (strcmp(argv[2], "-socket")==0)) || ((strcmp(argv[1], "-socket")==0) && (strcmp(argv[2], "-send")==0)) )
        {
            sendViaSocket();
        }
        else if ( ((strcmp(argv[1], "-receive")==0) && (strcmp(argv[2], "-file")==0)) || ((strcmp(argv[1], "-file")==0) && (strcmp(argv[2], "-receive")==0)) )
        {
           receive_fun();
        }
        else if ( ((strcmp(argv[1], "-receive")==0) && (strcmp(argv[2], "-socket")==0)) || ((strcmp(argv[1], "-socket")==0) && (strcmp(argv[2], "-receive")==0)) )
        {
            receiveViaSocket();
        }
        else
        {
            help();
        }
    }
    return 0;
}