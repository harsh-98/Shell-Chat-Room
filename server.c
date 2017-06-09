#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h> 
#include<pthread.h> //for threading , link with lpthread
#include<errno.h>


int    no_of_conn=0 ,ini_conn=0;
char password_entry1[100],password_entry2[100];
void *connection_handler(void *);
char * password_entry(int );
struct user { 
	char *username;
	char password[100];
	char *handle;
};
struct user user_array[10] ;

int main(int argc , char *argv[])
{
    int socket_desc , new_socket , c , *new_sock;
    struct sockaddr_in server , client;
    char *message;
    char *endptr;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(strtol(argv[1],&endptr,0));
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("bind failed");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while(new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) 
    {
        puts("Connection accepted");
         
        //Reply to the client
        message = "Collected to the chat-room\n";
        write(new_socket , message , strlen(message));
      	
      	pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = new_socket;
         
       	if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
        
    }
     
    if (new_socket<0)
    {
        perror("accept failed");
        return 1;
    }
     
    return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
	//read and write 
	int read_size;
    char *message , client_message[2000];
    char username_entry[50];
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    ini_conn=(ini_conn<sock)? sock: ini_conn;
    struct user user_handled;
    short pass=0;

    // username 
    message="Enter your name\n";
    write(sock , message , strlen(message));
    recv(sock , username_entry , 50 , 0);
    user_handled.username=username_entry;
	
	for(int count=0;count<no_of_conn;count++){
		printf("%s\n",user_array[count].username );
		printf("%d\n",strcmp(user_array[count].username,username_entry));
        if(user_array[count].username!=NULL && strcmp(user_array[count].username,username_entry)==0){
            pass=1;
            user_handled=user_array[count];
            break;
        } else if (user_array[count].username==NULL){
            pass=0;
            break;
        }

    }
    if (pass==1){
    message="Member by this name exists enter the password--\n";
    write(sock , message , strlen(message));
    recv(sock , client_message , 100 , 0);
     printf("%s\n","b" );
    if (strcmp(user_handled.password,client_message)){
    	message="Wrong password\n";
    write(sock , message , strlen(message));
    	exit(EXIT_FAILURE);
    }
    memset(client_message, 0, sizeof(client_message));
    
    }
    else if (pass==0){
    
    strcpy(user_handled.password,password_entry(sock));

    user_array[no_of_conn]=user_handled;
    
    }

    printf("%s\n","a" );

	//printf("%s\n", user_array[no_of_conn].username);
    	 printf("%s\n","c" );
	no_of_conn++;

    //Send some messages to the client
    message = "you are now connect our local chat-room .\n We hope you enjoy your visit\n";
    write(sock , message , strlen(message));
     
    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
        //Send the message back to client
    	int a=0; //Send the message back to client
        while  (a<no_of_conn)
        {
        write(ini_conn-a , client_message , strlen(client_message));
        a++;
    	}
        memset(client_message, 0, sizeof(client_message)); 
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
         
    //Free the socket pointer
    free(socket_desc);
     
    return 0;
}

char* password_entry(int fil_dsc){
	
	char *message;
	message="Enter your password (password must be between 10-100 characters)\n";
    write(fil_dsc , message , strlen(message));
    recv(fil_dsc , password_entry1 , 100 , 0);
   // memset(message, 0, sizeof(message));
    message="Re-enter the password--\n";
    write(fil_dsc , message , strlen(message));
    recv(fil_dsc , password_entry2 , 100 , 0);
    printf("%s\n", password_entry2);
    printf("%s\n", password_entry1);
    printf("%d",password_entry1!=password_entry2);
    if (strcmp(password_entry1,password_entry2))
    {	printf("%s\n","saa" );
	memset(password_entry2, 0, sizeof(password_entry2));
    memset(password_entry1, 0, sizeof(password_entry1));
    	return password_entry(fil_dsc);

    }
    else {
    	return password_entry1;
    }
}

