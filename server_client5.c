// STUDENT: Matthew Teets
// CLASS: CSCI 3800
// ASSIGNMENT: Lab5

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#define STDIN 0
#define VERSION 1

// Structure that holds info of config.txt file
struct _configure {
    int port_num;
    char ip[14];
};

// Structure that holds info pertaining to the message being sent
struct _msg{
    int version;
    int location;
    char command[20];
    char msg[100];
};

int findCoordinates(int choice, int *row, int *column, int ROWS, int COLUMNS);
int distance(int c1, int c2, int r1, int r2);
int sendData(char *buffer, int sd, struct sockaddr_in server_address); // This function allows the client to send data to the server
void parseMe(char *line, struct _msg *message); // Parses through the sent message and tokenizes the colon delimited values.

// ====================================================================//
// =============================== MAIN ===============================//
// ====================================================================//

int main(int argc, char * argv[]) {
    
    // Struct info for servers 1 - 4
    struct _configure s1 [100];
    
    // Opens config file
    FILE *f = fopen("config.txt", "r");
    char line[250];
    
    // Error checking if file exists
    if (f == NULL)
    {
        printf("Error: No file \n");
    }
    
    // Reads config file into struct members for each server
    int MAXPARTNERS = 0; // Variable for the number of servers being connected
    while (fgets(line, sizeof(line), f)) { // returns NULL when no more data
        sscanf(line, "%s %d", s1[MAXPARTNERS].ip, &s1[MAXPARTNERS].port_num);
        MAXPARTNERS++;
    }
    
    fclose(f); // Closes file when we have what we need
    
    // Variables used throughout the program
    char buffer[100]; // Variable for the message entered by the user
    char bufferSend[100]; // Variable for the message being sent to the server(s)
    char bufferRecv[100]; // Variable for the message being received from the client(s)
    char *ptr;
    int rc;
    int location;
    int flags = 0;
    
    struct sockaddr_in from_address;
    socklen_t fromLength = sizeof(struct sockaddr);
    fromLength = sizeof(struct sockaddr_in);
    fd_set socketFDS; // Set of socket discriptors
    int maxSD; // Tells the OS how many sockets are set
    struct _msg message;
    
    // Checks if number of cmd line arguments is correct
    if (argc < 3) {
        printf("client4 <portnumber> <location>\n");
        exit(1);
    }
    
    int sd; // Socket descriptor
    
    struct sockaddr_in server_address; // Provides address info for current server
    struct sockaddr_in partner_address; // Provides address into for other servers
    
    // Bind requirements
    int portNumber = atoi(argv[1]);
    fromLength = sizeof(struct sockaddr_in);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portNumber);
    server_address.sin_addr.s_addr = INADDR_ANY;
    
    sd = socket(AF_INET, SOCK_DGRAM, 0); // Creates the socket
    rc = bind(sd, (struct sockaddr *)&server_address, sizeof(server_address)); // Binds the socket to the local address info
    
    // Checks if the bind was successful
    if (rc < 0) {
        perror("bind");
        exit(1);
    }
    
    int ROWS, COLUMNS;
    int choice_r;
    int choice_c;

    printf("-------------------------------\n");
    
    // Displays location to the console
    location = atoi(argv[2]);
    printf("My location: %d\n", location);
    
    // User enters the size of the matrix
    printf("Enter grid size (N M): ");
    scanf("%d %d%*c", &ROWS, &COLUMNS);
    
    printf("\n-------------------------------\n");
    printf("\n");
    
    // forever loop
    for(;;)
    {
        fflush(stdin); // Used to clear the output buffer
        printf("Enter message to send: \n"); // Prompts user for msg
        
        // Sets all characters to '\0'
        memset(buffer, '\0', 100);
        memset(bufferSend, '\0', 100);
        memset(bufferRecv, '\0', 100);
        
        FD_ZERO(&socketFDS); // Initializes set to contain no file descriptors
        FD_SET(sd, &socketFDS); // Sets the value for sd in the set
        FD_SET(STDIN, &socketFDS); // Sets the value for STDIN in the set
        
        // Checks descriptor sizes
        if (STDIN > sd) {
            maxSD = STDIN;
        }else{
            maxSD = sd;
        }
        
        // Sets which file descriptor is ready to be read
        rc = select (maxSD+1, &socketFDS, NULL, NULL, NULL);
        
        // Received data from the client
        if (FD_ISSET(STDIN, &socketFDS)) {
            memset(buffer, '\0', 100);
            ptr = fgets(buffer, sizeof(buffer), stdin);
            ptr = ptr;
            
            // Creates message in discussed protocol format
            buffer[strlen(buffer)-1] = '\0';
            sprintf(bufferSend, "%d:INFO:%d:%s", VERSION, location, buffer);
            printf("Sending: '%s'\n\n", buffer);
            
            // Calls 'sendData' function to send message(s) to the server(s)
            for (int i = 0; i < MAXPARTNERS; i++){
                partner_address.sin_family = AF_INET; // Sets the address family for the transport address
                partner_address.sin_port = htons(s1[i].port_num); // Indexes to the 'port_num's of the struct array
                partner_address.sin_addr.s_addr = inet_addr(s1[i].ip); // Indexes to the 'ip's of the struct array
                sendData(bufferSend, sd, partner_address); // Calls the 'sendData' funtion
            }
        }
        
        // Received data from the server
        if (FD_ISSET(sd, &socketFDS)) {
            printf("\n<<<<<<<<<< Message incoming... >>>>>>>>>>\n");
            
            rc = recvfrom(sd, bufferRecv, sizeof(bufferRecv), flags, (struct sockaddr *)&from_address, &fromLength);
            parseMe(bufferRecv, &message); // Calls the 'parseMe' function
            
            // Prints and formats all data recieved
            printf("\nRECEIVED - version %d : location %d : command is %s : message is '%s'\n",
                   message.version, message.location, message.command, message.msg);
            
            //============================== LOCATION STUFF ==============================//
            
            // Find the coordinates of the user location
            int ret_1 = findCoordinates(location, &choice_r, &choice_c, ROWS, COLUMNS);
            if (-1 == ret_1) // If true jump to label 'end_msg'
            {
                goto end_msg;
            }
            
            // Displays the user coordinates
            printf("User coords: (%d , %d)\n", choice_r, choice_c);
            int x1, y1;
            x1 = choice_r;
            y1 = choice_c;
            
            // Find the coordinates of the message location
            int ret_2 = findCoordinates(message.location, &choice_r, &choice_c, ROWS, COLUMNS);
            if (-1 == ret_2) // If true jump to label 'end_msg'
            {
                goto end_msg;
            }
            
            // Displays the message coordinates
            printf("Message coords: (%d , %d)\n", choice_r, choice_c);
            int x2, y2;
            x2 = choice_r;
            y2 = choice_c;
            
            // Calculates the euclidean distance between user location and message location.
            int distanceVal = distance(x1, x2, y1, y2);
            // Displays the distance between the two points
            printf("Distance between (%d, %d) and (%d, %d): %u\n", x1, y1, x2, y2, distanceVal);
            
            if (distanceVal > 2) { // If the distance between the two coords is greater than 2 OUT OF RANGE
                printf("\nOUT OF RANGE\n\n");
            }else{ // Else is IN RANGE
                printf("\nIN RANGE\n\n");
            }
            
        // goto label to jump to end of the message
        end_msg:
            printf("<<<<<<<<<< End of Message... >>>>>>>>>>\n\n");
        }
    }
    close(sd); // Closes the client socket
    return 0;
}

// ===============================================================================//
// ================================== FUNCTIONS ==================================//
// ===============================================================================//

int sendData(char *buffer, int sd, struct sockaddr_in server_address) {
    /*
       All information being sent out to the server :
            sd                                   ->  socket descriptor
            buffer                               ->  data being sent
            strlen(buffer)                       ->  how many bytes of data being sent
            0                                    ->  flags
            (struct sockaddr *) &server_address  ->  TO: address of the server
            sizeof(server_address)               ->  size of the data structure being sent
     */
    
    int rc = 0;
    rc = sendto(sd, buffer, strlen(buffer), 0, (struct sockaddr *) &server_address, sizeof(server_address));
    
    // Checks if sendto was successfully filled
    if(rc <= 0)
    {
        printf("ERROR: No bytes were sent/received... \n");
        exit(1);
    }
    return(0);
}

// =========================================================================//

void parseMe(char *line, struct _msg *message){
    int version;
    char command[20];
    int location;
    char *ptr;
    char msg[100];
    
    //printf("Parsing %s\n", line);
    version = atoi(strtok(line, ":"));
    
    ptr = strtok(NULL, ":");
    //printf("Command %s\n", ptr);
    sprintf(command, "%s", ptr);
    
    ptr = strtok(NULL, ":");
    location = atoi(ptr);
    //printf("location is %d\n", location);
    
    ptr = strtok(NULL, ":");
    //printf("Message is '%s'\n", ptr);
    sprintf(msg, "%s", ptr);

    // printf("Version - %d\nCommand - %s\nLocation - %d\nMessage - '%s'\n", version, command, location, msg);
    
    // Push into the message structure
    message -> version = version;
    message -> location = location;
    sprintf(message -> command, "%s", command);
    sprintf(message -> msg, "%s", msg);
}

// =========================================================================//

int findCoordinates(int choice, int *row, int *column, int ROWS, int COLUMNS) {
  // ROWS and COLUMNS are the size of the grid
  // row and column are returned as location of the ‘choice’
  // choice is the cell you are in
    
    *row = (choice - 1) / COLUMNS + 1;
    *column = (choice - 1) % COLUMNS + 1;
    
    if (*row > ROWS) {
        printf ("\nNOT IN GRID...\n\n");
        return(-1);
    }else{
        //printf ("Your row/column: ( %d / %d ) \n",*row, *column);
    }

    return(1);
    
}

//=========================================================================//

//Function to find the euclidean distance between 2 points
int distance(int c1, int c2, int r1, int r2) {
    return (int)sqrt(pow(c2 - c1, 2) + pow(r2 - r1, 2));
}

//=========================================================================//
