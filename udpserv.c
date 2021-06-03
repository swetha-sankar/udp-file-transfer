/**
 * Swetha Sankar
 * Programming Assignment #2
 * CISC 450
 * udpserv.c: Driver code for UDP server. Copies filepath from client, reads file, and sends it to client's out.txt file.
 * */

#include "imports.h"

void printTotals(int transmit_count, int data_count, int total_count, int drop_count, int success_count,int ack_count, int timeout_count){
	/**
	 * Print totals at end
	 * */
	printf("Number of data packets generated for transmission (initial transmission only): %d\n", transmit_count);
  	printf("Total number of data bytes generated for transmission, initial transmission only: %d\n", data_count);
	printf("Total number of data packets generated for retransmission: %d\n", total_count);
	printf("Number of data packets dropped due to loss: %d\n", drop_count);
	printf("Number of data packets transmitted successfully: %d\n", success_count);
	printf("Number of ACKs received: %d\n", ack_count);
	printf("Count of how many times timeout expired: %d\n", timeout_count);
	
}


int SimulateLoss(double packet_loss_ratio){
	/**
	 * Simulate loss in server's transmission of data packets to client.
	 * Uses configuration parameter packet loss ratio (user specified)
	 * */
	double sim = ((double) rand() / (double) RAND_MAX);
	if(sim < packet_loss_ratio){
		return 1;
	}
	else{
		return 0;
	}
}


int main(int argc, char** argv) {
  /**
   * Driver function for UDP server
   * */
  int socket1, clilen; 
  char data[DATA];
  char input[DATA];
  FILE * file1;
  // ackpkt, packet structs defined in imports.h
  ackpkt *ack_packet = malloc(sizeof(ackpkt));
  packet *pkt_rcv = malloc(sizeof(packet));
  packet *pkt_send = malloc(sizeof(packet));
  struct sockaddr_in serv_addr, cli_addr; 
  int sent, received, eot; 
  int packet_count = 1; //sequence number
  int success_count = 0, data_count = 0, drop_count = 0, ack_count = 0, timeout_count = 0, transmit_count = 0, retransmit_count = 0, total_count = 0;
  struct timeval tv; // used for timeout

  if(argc != 3){
	  // Error checking before setting configuration parameters
	  error("Usage ./server [timeout] [packet_loss_ratio]\n");
  }
  // Set configuration parameters with command line arguments
  int timeout = atoi(argv[1]);
  if(timeout < 1 || timeout > 10){
	// Error checking timeout input value
	  error("Incorrect timeout value\n");
  }

  double packet_loss_ratio = atof(argv[2]);
  if(packet_loss_ratio < 0 || packet_loss_ratio > 1){
	  // Error checking packet loss ratio input value
	  error("Incorrect packet_loss_ratio value\n");
  }

  // UDP socket creation
  if((socket1 = socket(AF_INET, SOCK_DGRAM, 0))<0){
	  error("Socket creation failure");
  }

  // Initialize server 
  memset(&serv_addr, 0, sizeof(serv_addr));
  memset(&cli_addr, 0, sizeof(cli_addr));
  int len = sizeof(cli_addr);

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(PORT);
  
  // Bind to local port 
  if(bind(socket1, (const struct sockaddr *)&serv_addr, sizeof(serv_addr))<0){
	  error("Binding error");
  }
  
  received = recvfrom(socket1, (packet *) pkt_rcv, sizeof(*pkt_rcv), MSG_WAITALL,(struct sockaddr*)&cli_addr, &len);

  if (received <0){
    error("Error reading from socket");
  }
 
   // Set timeout according to user input
  tv.tv_usec = pow(10, (double) timeout);
  //  Timeout value = 10^n microseconds (per instructions)

  tv.tv_sec = 0;
  
  if(setsockopt(socket1, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))<0){
          error("Timeout error");
  }

  // Copy & read file
  
  memcpy(input, pkt_rcv->data, pkt_rcv->header[0]);
  
  file1 = fopen(input, "r");
  
  if (file1 == NULL) {
	error("Error opening file\n");
  }
  
  
  // Seed for random number generation in above function
  srand(time(0));
  
  while (fgets(data, DATA, file1) != NULL) {
	// Must iterate through the file and send to client's out.txt
	// Check configuration paramters and send as necessary
    
    pkt_send->header[0] = strlen(data);
    pkt_send->header[1] = packet_count; 
    strcpy(pkt_send->data, data);
    data_count += strlen(data); 
    transmit_count++;
    
    printf("Packet %d generated for transmission with %zu data bytes\n", packet_count, strlen(data));

    if(SimulateLoss(packet_loss_ratio)== 0){
	    // No packet loss
	    int stop = 0;
	    while(!stop){
		    // Increment total count of packets and send packet
		    total_count++;
		    sent = sendto(socket1, (packet *) pkt_send, sizeof(packet), MSG_CONFIRM, 
				    (struct sockaddr *) &cli_addr, len);
		    if (sent<0){ 
			    error("Error sending packet\n");
		    }
		    // Have to reset this value to see if ack received
		    received = 0;
		    received = recvfrom(socket1, (ackpkt *) ack_packet, sizeof(ackpkt), MSG_WAITALL, (struct sockaddr *) &cli_addr, &len);
		    // Use the received value in order to see whether the ACK was properly delivered

		    if(received != -1){
			    // ACK received
			    ack_count++;
			    printf("ACK %d received\n", ack_packet->seq);
	            }
		    if (ack_packet->seq!=packet_count) { 
			    // if the sequence number of the ack does not match the current sequence number of the packet then we need to handle timeout
	  	    	if (received==-1) {
				// Expired timeout
	    			timeout_count++;
				printf("Timeout expired for packet numbered %d\n", packet_count);
	  		}
	  		printf("Packet %d generated for re-transmission with %zu data bytes\n", packet_count, strlen(data));
		    } 
		    else if (ack_packet->seq==packet_count) {
	  		// Successful transmission
			success_count++;
			printf("Packet %d successfully transmitted with %d data bytes\n", packet_count, pkt_send->header[0]);
			stop = 1; 
			// The ACK has been successfully received so we can break
		}
      	}
    } 
    else {
      // Account for packet loss
      printf("Packet %d lost\n", packet_count);
      drop_count++;
    }
    /** Each packet transmitted by the server alternates between 0 and 1 for each packet*/			    
    if(packet_count == 0){
	    packet_count = 1;
    }
    else if(packet_count == 1){
	    packet_count = 0;
    }
  }

  // EOT packet
  data[0] = 0;
  strcpy(pkt_send->data, data);
  pkt_send->header[0] = 0;
  pkt_send->header[1] = packet_count;  
  eot = sendto(socket1, (packet *) pkt_send, sizeof(packet), 0, (struct sockaddr *)&cli_addr, len);
  if(eot < 0){
	  error("Error writing EOT to socket");
  }	
  printf("End of Transmission Packet with sequence number %d transmitted\n", packet_count);
  printTotals(transmit_count, data_count, total_count,drop_count, success_count, 
		  ack_count, timeout_count);

  // Close server socket
  close(socket1);
  return 0;
  
}
