/* Swetha Sankar
 * Programming Assignment #2
 * CISC 450
 * imports.h: Includes necessary includes for udpserv.c and udpcli.c
 * May 2021
 * */

#include <stdio.h>
#include <math.h> 
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "printError.c"
#define HOST "127.0.0.1" // Internet loopback protocol
#define PORT 8080 // Standard testing port
#define HEADER 2 // Header fields
#define DATA 80 //Packet data size


// general packet struct 
typedef struct pack{
	char data[DATA];
	short header[HEADER];
} packet;

// ack packet struct
typedef struct ackpack{
	short seq;
} ackpkt;
