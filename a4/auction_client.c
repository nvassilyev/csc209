#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#define BUF_SIZE 128

#define MAX_AUCTIONS 5
#ifndef VERBOSE
#define VERBOSE 0
#endif

#define ADD 0
#define SHOW 1
#define BID 2
#define QUIT 3

/* Auction struct - this is different than the struct in the server program
 */
struct auction_data {
    int sock_fd;
    char item[BUF_SIZE];
    int current_bid;
};

/* Displays the command options available for the user.
 * The user will type these commands on stdin.
 */

void print_menu() {
    printf("The following operations are available:\n");
    printf("    show\n");
    printf("    add <server address> <port number>\n");
    printf("    bid <item index> <bid value>\n");
    printf("    quit\n");
}

/* Prompt the user for the next command 
 */
void print_prompt() {
    printf("Enter new command: ");
    fflush(stdout);
}


/* Unpack buf which contains the input entered by the user.
 * Return the command that is found as the first word in the line, or -1
 * for an invalid command.
 * If the command has arguments (add and bid), then copy these values to
 * arg1 and arg2.
 */
int parse_command(char *buf, int size, char *arg1, char *arg2) {
    int result = -1;
    char *ptr = NULL;
    if(strncmp(buf, "show", strlen("show")) == 0) {
        return SHOW;
    } else if(strncmp(buf, "quit", strlen("quit")) == 0) {
        return QUIT;
    } else if(strncmp(buf, "add", strlen("add")) == 0) {
        result = ADD;
    } else if(strncmp(buf, "bid", strlen("bid")) == 0) {
        result = BID;
    } 
    ptr = strtok(buf, " "); // first word in buf

    ptr = strtok(NULL, " "); // second word in buf
    if(ptr != NULL) {
        strncpy(arg1, ptr, BUF_SIZE);
    } else {
        return -1;
    }
    ptr = strtok(NULL, " "); // third word in buf
    if(ptr != NULL) {
        strncpy(arg2, ptr, BUF_SIZE);
        return result;
    } else {
        return -1;
    }
    return -1;
}

/* Connect to a server given a hostname and port number.
 * Return the socket for this server
 */
int add_server(char *hostname, int port) {
        // Create the socket FD.
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("client: socket");
        exit(1);
    }
    
    // Set the IP and port of the server to connect to.
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    struct addrinfo *ai;
    
    /* this call declares memory and populates ailist */
    if(getaddrinfo(hostname, NULL, NULL, &ai) != 0) {
        close(sock_fd);
        return -1;
    }
    /* we only make use of the first element in the list */
    server.sin_addr = ((struct sockaddr_in *) ai->ai_addr)->sin_addr;

    // free the memory that was allocated by getaddrinfo for this list
    freeaddrinfo(ai);

    // Connect to the server.
    if (connect(sock_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("client: connect");
        close(sock_fd);
        return -1;
    }
    if(VERBOSE){
        fprintf(stderr, "\nDebug: New server connected on socket %d.  Awaiting item\n", sock_fd);
    }
    return sock_fd;
}
/* ========================= Add helper functions below ========================
 * Please add helper functions below to make it easier for the TAs to find the 
 * work that you have done.  Helper functions that you need to complete are also
 * given below.
 */

/* Print to standard output information about the auction
 */
void print_auctions(struct auction_data *a, int size) {
    printf("Current Auctions:\n");

    /* TODO Print the auction data for each currently connected 
     * server.  Use the follosing format string:
     *     "(%d) %s bid = %d\n", index, item, current bid
     * The array may have some elements where the auction has closed and
     * should not be printed.
     */
    for (int i = 0; i < size; i++){
        if (a[i].sock_fd != -1){
            printf("(%d) %s bid = %d\n", i, a[i].item, a[i].current_bid);
        } 
    }
}

/* Process the input that was sent from the auction server at a[index].
 * If it is the first message from the server, then copy the item name
 * to the item field.  (Note that an item cannot have a space character in it.)
 */
void update_auction(char *buf, int size, struct auction_data *a, int index) {
    
    char* token = strtok(buf, " ");
    if (strcmp(token, "Auction") == 0){
        a[index].item[0] = '\0';
        a[index].sock_fd = -1;
        close(a[index].sock_fd);
        return;
    }
    else{
        int size = strlen(token);
        strncpy(a[index].item, token, size);
        a[index].item[size] = '\0';

        token = strtok(NULL, " ");
        
        int bid = atoi(token);
        if (bid >= 0){
            a[index].current_bid = bid;
        }
        else{
            fprintf(stderr, "ERROR malformed bid: %s", buf);
        }
    }
    return;
}


int main(void) {

    char name[BUF_SIZE];

    // Declare and initialize necessary variables
    struct auction_data auctions[MAX_AUCTIONS];

    for (int i = 0; i < MAX_AUCTIONS; i++){
        auctions[i].sock_fd = -1;
        auctions[i].current_bid = 0;
    }
    
    // Get the user to provide a name.
    printf("Please enter a username: ");
    fflush(stdout);
    int num_read = read(STDIN_FILENO, name, BUF_SIZE);
    if(num_read <= 0){
        fprintf(stderr, "ERROR: read from stdin failed\n");
        exit(1);
    }
    print_menu();
    
    fd_set socks;

    while(1) {
        print_prompt();

        FD_ZERO(&socks);
        FD_SET(STDIN_FILENO, &socks);
        int max = 0;
        char buff[BUF_SIZE];

        for (int i = 0; i < MAX_AUCTIONS; i++){
            if (auctions[i].sock_fd != -1){
                if (max < auctions[i].sock_fd){
                    max = auctions[i].sock_fd;
                }    
                FD_SET(auctions[i].sock_fd, &socks);
            }
        }

        if (select(max + 1, &socks, NULL, NULL, NULL) < 0){
            perror("client: select");
            exit(1);
        }
        
        for (int i = 0; i < max + 1; i++){
            if (FD_ISSET(i, &socks)){
                if (i == STDIN_FILENO){
                    //User input
                    fgets(buff, BUF_SIZE, stdin);

                    char p1[BUF_SIZE];
                    char p2[BUF_SIZE];
                    int output = parse_command(buff, BUF_SIZE, p1, p2);

                    int port = 0;
                    int item = 0;
                    switch (output) {
                        case ADD:
                            port = atoi(p2);
                            int sock = add_server(p1, port);

                            int slot = -1;
                            for (int j = 0; j < MAX_AUCTIONS; j++){ 
                                if (auctions[j].sock_fd == -1){
                                    slot = j;
                                    break;
                                }
                            }
                        
                            if (slot != -1) {
                                auctions[slot].sock_fd = sock;
                            } 
                            if (write(sock, name, BUF_SIZE) == 0){
                                perror("client: write");
                                close(auctions[item].sock_fd);
                            }
                            
                            break;
                        case SHOW:
                            print_auctions(auctions, MAX_AUCTIONS);
                            break;
                        case BID:
                            item = atoi(p1);
                            if (write(auctions[item].sock_fd, p2, strlen(p2)) == 0) {
                                perror("client: write");
                                close(auctions[item].sock_fd);
                            } 
                            break;
                        case QUIT:
                            for (int i = 0; i < MAX_AUCTIONS; i++){
                                close(auctions[i].sock_fd);
                            }
                            exit(0);
                            break;
                        }    
                        
                    }
                else{
                    //server input
                    int auction_index = 0;
                    for (int j = 0; j < MAX_AUCTIONS; j++) {
                        // find the auction in the array with sock_fd i
                        if (auctions[i].sock_fd == i){
                            auction_index = j;
                            break;
                            }
                        }
                    if (read(i, buff, BUF_SIZE) < 0){
                        fprintf(stderr, "ERROR: read from stdin failed\n");
                        exit(1);
                        }

                    update_auction(buff, BUF_SIZE, auctions, auction_index);
                }
            }
        }
    }
    return 0; // Shoud never get here
}
