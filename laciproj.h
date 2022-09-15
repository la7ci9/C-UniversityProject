#ifndef LACIPROJ_H_
#define LACIPROJ

void help();

void sendViaFile();

void receiveViaFile();

void sendViaSocket();

void receiveViaSocket();

void arguments(int argc, char* argv[]);

int Measurement(int **Values);

void BMPcreator(int *Values, int NumValues);

int bintoint(int *array,int size);

int FindPID();

#endif