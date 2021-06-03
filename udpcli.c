/**
 * Swetha Sankar
 * Programming Assignment #2
 * CISC 450
 * udpcli.c: UDP Client Code
 * Client takes in input file name and ack loss ratio as 
 * configuration parameters from the command line 
 * (./client [filepath] [acklossratio])
 * This file contains the driver code for the UDP client.
 * Can be made using gcc -o client udpcli.c (or use make)
 * * */

#include "imports.h"

void printTotals(int total_count, int data_bytes_received, int dups, int ack_success, int ack_drop){
	/* Print totals at the end */
	printf("Total number of data packets received successfully: %d\n", total_count+dups);
	printf("Number of duplicate data packets received successfully: %d\n", dups);
	printf("Number of data packets received successfully, not including duplicates: %d\n", total_count);
	printf("Total number of data bytes received which are delivered to user: %d\n", data_bytes_received);
	printf("Number of ACKs transmitted without loss: %d\n", ack_success);
	printf("Number of ACKs generated but dropped due to loss: %d\n", ack_drop);
	printf("Total number of ACKs generated (with and without loss): %d\n", ack_success+ack_drop);
}


int SimulateACKLoss(double ack_loss_ratio){
	/**
	 * Simulates loss for ACKs using configuration parameter ACK Loss
	 * Ratio. Generates uniformly distributed random # between 0 and 1.
	 * Returns 0 or 1 depending on whether this # is less than the loss 
	 * ratio
	 * */
	double sim = (double) (rand()) / (double) RAND_MAX;
	if(sim < ack_loss_ratio){
		return 1;
	}
	else{
		return 0;
	}
}


int main(int argc, char *argv[]){
	// Modified to take in command line arguments for UDP project
	int socket1; 
	// Packets to send to and receive from the server
	packet *pkt_send = malloc(sizeof(packet));
	packet *pkt_rcv = malloc(sizeof(packet));
	struct sockaddr_in serv_addr;
	char data[DATA]; // character buffer to store data 
	ackpkt *ack_packet = malloc(sizeof(ackpkt));
	//short ack_packet;
	FILE *message; 
	int send1, received, len;
	// initializing variables for totals at the end 
	int current;
	int ack_count = 0, total_count = 0, packet_count = 0, dup_count = 0, data_bytes_received = 0, ack_success = 0, ack_drop = 0;
	
	if(argc !=3){
		error("Usage: ./client [filename] [ack loss rate] \n");
	}

	// Set user configuration parameters 
	
	double ack_loss_ratio = atof(argv[2]);
 	if(ack_loss_ratio < 0 || ack_loss_ratio >= 1){
		// Error checking loss ratio value
		error("Loss ratio must be a real number between 0 and 1\n");
	}

	strcpy(data, argv[1]);
	
	// UDP socket creation
	
	if((socket1 = socket(AF_INET, SOCK_DGRAM, 0))<0){
		error("Socket creation failure");
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	// Set header & data fields of filename packet	
	pkt_send->header[0] = (short) strlen(data);
	pkt_send->header[1] = 0;
	strcpy(pkt_send->data, data);

	// Sending filename
	send1 = sendto(socket1, (packet *) pkt_send, sizeof(packet), MSG_CONFIRM,
			(struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if(send1 < 0){
		error("Error writing filename to server");
	}
	
		
	received = recvfrom(socket1, (packet *) pkt_rcv, sizeof(*pkt_rcv), MSG_WAITALL, (struct sockaddr *)&serv_addr, &len);
	current = 0;

	// Client stores everything received in out.txt (writes to it)
	message = fopen("out.txt", "w");

	// Seed for random number generation in SimulateACKLoss function
	srand(time(0)); 
	
	// Loop through received packets
	while (received > 0){
		// Check for EOT packet
		if(pkt_rcv->header[0]==0){
			printf("End of Transmission Packet with sequence number %d received \n", pkt_rcv->header[1]);
			break;
		}

		// Check for duplicate packet
		if(current == pkt_rcv->header[1]){	
			printf("Duplicate packet %d received with %d data bytes\n", pkt_rcv->header[1], pkt_rcv->header[0]);
			dup_count++;
		}
		

		else{
			// Data packet numbered n is received by client for the first time
			printf("Packet %d received with %d data bytes\n", pkt_rcv->header[1], pkt_rcv->header[0]);
			// Increment packet_count and data_bytes_received for every packet
			total_count++;
			data_bytes_received += pkt_rcv->header[0];

			// Contents of data packet numbered n delivered to user are stored in output
			fprintf(message, "%s", pkt_rcv->data);
			printf("Packet %d delivered to user\n", pkt_rcv->header[1]);
		}
		// Check whether next packet is a dup
		current = pkt_rcv->header[1];

		// Set up ack packet
		ack_packet->seq = pkt_rcv->header[1];
		printf("ACK %d generated for transmission\n", pkt_rcv->header[1]);
		ack_count++;

		if(SimulateACKLoss(ack_loss_ratio) == 0){
			send1 = sendto(socket1, (ackpkt *) ack_packet, sizeof(ackpkt), MSG_CONFIRM, (const struct sockaddr *) &serv_addr, sizeof(serv_addr));
			printf("ACK %d successfully transmitted\n", pkt_rcv->header[1]);
			ack_success++;
		}

		else{
			// If the SimulateACKLoss returns 1 there is loss
			printf("ACK %d lost\n", pkt_rcv->header[1]);
			ack_drop++;
		}
		int check = 0;
    		check = recvfrom(socket1, (packet *) pkt_rcv, sizeof(*pkt_rcv), MSG_WAITALL, (struct sockaddr *) &serv_addr, &len);
	}
	fclose(message);
	
	printTotals(total_count, data_bytes_received, dup_count, ack_success, ack_drop);
	bzero(data, DATA);
	close(socket1);
	return 0;
}

