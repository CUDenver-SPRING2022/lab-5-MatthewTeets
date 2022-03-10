# Lab 5
#### Name: Matthew Teets
#### Date: 3/10/22
#### Class: CSCI 3800

#

### **Description:**
This program uses the combined server client program from lab 4 and allows the user to detect if messages that are being received are **IN RANGE**, **OUT OF RANGE**, or **NOT IN GRID**. This program works with multiple servers, but the user must have at least one terminal window open and connected to the csegrid. The client/server program can run on any gnode just fine. However, when running more than one it is recommended to run them on different gnodes. The config.txt contains the IP addresses of 4 gnode servers and 4 port numbers. The port numbers are command line arguments that must be entered when running the server programs (example below).    

#

### **The program:**
- server_client5.c
  - Promts user for grid size ```"N M"```
  - Reads the config file into a struct
  - Creates the socket/address info using the struct
  - Creates and binds a DGRAM socket to the server address
  - Promts user for a message to send to the server
  - Calculates the distance between the location of the sender and receiver of the message
  - Messages from the client are processed, formatted, and printed to the terminal
  - Displays if message sent is **IN RANGE**, **OUT OF RANGE**, or **NOT IN GRID**
  - Loops until the program is manually terminated using ```Control + C```

#

### **How to run:**
- Connect to CSE-grid
  - Example: ssh username@csegrid.ucdenver.pvt
  - Connect your terminal window(s) to this Linux server
- cd to the directory containing the c program, makefile, and config.txt

**Terminal window used to run server_client4.c**
```
  $ ssh csci-gnode-NUM   /* Use this command to get each terminal to the correct gnode number (i.e., csci-gnode-01) */
  $ make -f Makefile     /* Compiles the c programs */
  $ ./client5 [SERVER-PORT-NUMBER] [LOCATION]    /* Command to run the executable */
```
**Example:**
```
$ ./client5 1101 3          
$ ------------------------            
$ My location: 3       
$ Enter grid size (N M): 5 5 /* Initializes a 5x5 grid */
$         
$ (prints results...)
```
(Repeat this process with different port numbers and locations to connect more servers)
