#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h> 
#include<pthread.h> //for threading , link with lpthread
#include<errno.h>
#include<stdbool.h>

#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"

int     no_of_conn=0 ,ini_conn=0;
char    password_entry1[100],password_entry2[100];
int     no_of_users=0;

void    *connection_handler(void *);
char    * password_entry(int );
char    *substring_bool(char *,char*);
void    remove_newline(char *);
int connection_array[100]={0};

struct user { 
    char *username;
    char password[100];
    char handle[50];
    int user_no;
    int conn_array[20];
    int pos;
};

struct user user_array[20] ;

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
        //printf("Could not create socket");
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
    char *message , client_message[2000],cli_mes_final[2100],user_prompt[200];
    char username_entry[50];

    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    connection_array[sock]=1;
    //printf("%d\n",sock );
    ini_conn=(ini_conn<sock)? sock: ini_conn;
    struct user user_handled;
    short pass=0;

    // username 
    message=KGRN "Enter your name\n" RESET KWHT;
    write(sock , message , strlen(message));
    recv(sock , username_entry , 50 , 0);

    remove_newline(username_entry);
    user_handled.username=username_entry;
    strcpy(user_handled.handle,username_entry);
    for(int count=0;count<no_of_users;count++)
    {
        ////printf("%s\n",user_array[count].username );
        ////printf("%d\n",strcmp(user_array[count].username,username_entry));
        if(user_array[count].username!=NULL && strcmp(user_array[count].username,username_entry)==0)
        {
            pass=1;
            user_array[count].conn_array[++user_array[count].pos]=sock;
            user_handled=user_array[count];
            break;
        } else if (user_array[count].username==NULL)
        {
            pass=0;
            break;
        }
    }

    if (pass==1)
    {
        message=KGRN "Member by this name exists enter the password--\n" RESET KWHT;
        write(sock , message , strlen(message));
        recv(sock , client_message , 100 , 0);
        
        if (strcmp(user_handled.password,client_message))
        {
            message=KRED "Wrong password\n" RESET ;
            write(sock , message , strlen(message));
            connection_array[sock]=0;
            close(sock);
            return 0;
        }
        memset(client_message, 0, sizeof(client_message));
    }
    else if (pass==0)
    {
        strcpy(user_handled.password,password_entry(sock));
        user_handled.user_no=no_of_users;
        //printf("%d\n", user_handled.user_no);
        // //printf("%zu\n",sizeof user_handled.conn_array );
        user_handled.pos=0;
        user_handled.conn_array[0]=sock;
        user_array[no_of_users]=user_handled;
        //printf("%s\n",user_array[no_of_users].username);
        no_of_users++;
    }
    printf("harsh");
    no_of_conn++;
    int new_user=0;
    ////printf("%s\n", user_array[no_of_conn].username);
     // new user is connected !!
    strcat(cli_mes_final,KYEL );
    strcat(cli_mes_final,"( Bot )");
    strcat(cli_mes_final,RESET );
    strcat(cli_mes_final,KCYN " : NEW USER @");
    strcat(cli_mes_final,KGRN );
    strcat(cli_mes_final,user_handled.username);
    strcat(cli_mes_final,RESET KCYN);
    strcat(cli_mes_final," joined the chat-room\n" RESET KWHT);

    while  (new_user<no_of_conn)
    {
        if(ini_conn-new_user!=sock&&connection_array[ini_conn-new_user]==1)
            write(ini_conn-new_user , cli_mes_final , strlen(cli_mes_final));
        new_user++;
    }
    memset(cli_mes_final, 0, sizeof(client_message));
    //Send some messages to the client
    message =KGRN "you are now connect our local chat-room.\nWe hope you enjoy your visit\n" RESET KWHT;
    write(sock , message , strlen(message));
    
    //Receive a message from client
    strcat(user_prompt,KBLU "( ");
    strcat(user_prompt,user_handled.handle);
    strcat(user_prompt," ):" KWHT);
    //write(sock , user_prompt , strlen(user_prompt));
    
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
       // printf("%d",read_size);
       //write(sock , user_prompt , strlen(user_prompt));
        if (client_message[0]!=0x0d&&client_message[0]!='\n')
        {
            int a=0,dm=0;
            char *handling_ptr;
            if(handling_ptr=substring_bool(client_message,"handle "))
            {
                remove_newline(handling_ptr);
                //printf("%d\n",user_handled.user_no );
                strcpy(user_handled.handle,handling_ptr);   

                user_array[user_handled.user_no]=user_handled;
            }
            else if (handling_ptr=substring_bool(client_message,"admin#"))
            {
                dm=2;
                while  (a<no_of_users)
                {
                    char *handling_ptr1;
                    //printf("%s\n",user_array[a].handle );
                    if(handling_ptr1=substring_bool(handling_ptr,user_array[a].handle))
                    {
                        printf("hi");
                        if(handling_ptr1=substring_bool(handling_ptr1," disconnect")){
                            for(int j=0;j<=user_array[a].pos;j++)
                                {connection_array[user_array[a].conn_array[j]]=0;
                                    close(user_array[a].conn_array[j]);
                                    printf("%d\n",user_array[a].conn_array[j] );
                                }
                            }
                        }
                        a++;
                    }
                }
                else if ((handling_ptr=substring_bool(client_message,"@"))&&dm!=2)
                {
                //printf("%s\n", handling_ptr);
                    while  (a<no_of_users)
                    {
                        char *handling_ptr1;
                    //printf("%s\n",user_array[a].handle );
                        if(handling_ptr1=substring_bool(handling_ptr,user_array[a].handle) )
                        {
                            remove_newline(handling_ptr1);
                            strcat(cli_mes_final,KBLU );
                            strcat(cli_mes_final,"( ");
                        ////printf("%zu\n",strlen(user_array[user_handled.user_no].handle) );
                            if(strlen(user_array[user_handled.user_no].handle))
                            {
                               strcat(cli_mes_final, user_array[user_handled.user_no].handle );
                           }
                           else
                           {
                               strcat(cli_mes_final,user_handled.username);
                           }
                        //direct message            
                           strcat(cli_mes_final," )");
                           strcat(cli_mes_final, RESET);
                           strcat(cli_mes_final,KMAG "[DM]: " RESET KCYN );
                           strcat(cli_mes_final,handling_ptr1 );
                           strcat(cli_mes_final,"\n" RESET KWHT);
                        //printf("%d\n",ini_conn );
                        //printf( "%d\n",ini_conn+a);
                           int t;
                           for(int j=0;j<=user_array[a].pos&&connection_array[t=user_array[a].conn_array[j]]==1;j++)
                               write(t, cli_mes_final , strlen(cli_mes_final));
                           memset(cli_mes_final, 0, sizeof(cli_mes_final)); 
                           memset(client_message, 0, sizeof(client_message));
                       }
                       a++;
                   }
                   dm=1;
               }
               else if(handling_ptr=substring_bool(client_message,"close")){
                 if(handling_ptr[0]==0x0d||handling_ptr[0]=='\n'){ 
                    printf("%d\n",sock);
                    connection_array[sock]=0;
                    close(sock);
                    return 0;
                }
            }
            else
            {
                remove_newline(client_message);
            }
            
            a=0;
             //Send the message back to client
            strcat(cli_mes_final,KCYN "( ");
            //printf("%zu\n",strlen(user_array[user_handled.user_no].handle));

            if(strlen(user_array[user_handled.user_no].handle))
            {
               strcat(cli_mes_final,user_array[user_handled.user_no].handle);
           }
           else
           {
               strcat(cli_mes_final,user_handled.username);
           }

           strcat(cli_mes_final," )" RESET KYEL ": ");
           strcat(cli_mes_final,client_message);
           strcat(cli_mes_final,"\n" RESET KWHT);

           if(dm==0)
            while (a<no_of_conn)
            {
                if(ini_conn-a!=sock&&connection_array[ini_conn-a]==1)
                    write(ini_conn-a , cli_mes_final , strlen(cli_mes_final));
                a++;
            }
            memset(cli_mes_final, 0, sizeof(cli_mes_final)); 
            memset(client_message, 0, sizeof(client_message));
        } 
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
    message=KGRN "Enter your password (password must be between 10-100 characters)\n" KWHT;
    write(fil_dsc , message , strlen(message));
    recv(fil_dsc , password_entry1 , 100 , 0);

   // memset(message, 0, sizeof(message));
    message=KGRN "Re-enter the password--\n" RESET KWHT;
    write(fil_dsc , message , strlen(message));
    recv(fil_dsc , password_entry2 , 100 , 0);
    
    if (strcmp(password_entry1,password_entry2))
    {   //printf("%s\n","saa" );
memset(password_entry2, 0, sizeof(password_entry2));
memset(password_entry1, 0, sizeof(password_entry1));
return password_entry(fil_dsc);

}
else {
    return password_entry1;
    }
}

char * substring_bool(char *big_str,char*lit_str){
   int i,j;
   int temp;
   char *temp_ptr=NULL;

   for(i=0;big_str[i]!='\0';i++)
   {
    j=0;
    if(big_str[i]==lit_str[j])
    {
        temp=i;
        while(big_str[i]==lit_str[j])
        {
            i++;
            j++;
        }
        
        if(lit_str[j]=='\0')
        {
            temp_ptr=big_str+i;
              //  //printf("The substring is present in given string at position %s\n",temp_ptr);
            return temp_ptr;
        }
        else
        {
            i=temp;
            temp=0;
            temp_ptr=NULL;
        }
    }
}

if(temp==0){
       // //printf("The substring is not present in given string\n");
    return temp_ptr;
}
}
void remove_newline(char *array)
{
    if (array[strlen(array)-2] == 0x0d) 
    {
        array[strlen(array)-2] = 0;
    } 
    else if (array[strlen(array)-1] == '\n') 
    {
        array[strlen(array)-1] = 0;
    }       
}