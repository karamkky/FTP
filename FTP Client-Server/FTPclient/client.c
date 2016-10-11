#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <ifaddrs.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/stat.h>
#define PORT 8000
#define BUFSIZE 1024
#define TRUE 1
#define FALSE 0
#define SERVER_IP "10.0.90.59" //change it to server
#define MAXSZ 1024




void printWelcome() {
  printf ("+=================================================+\n");
  printf ( "| W    E    L   C   O   M   E                     |\n");
  printf ( "+=================================================+\n");
  printf ( "| Already Have An Account?                        |\n");
  printf ( "| ***  Enter Y to log in  ***                     |\n");
  printf ( "| ***  Enter N to register for an account  ***    |\n");
  printf ( "| ***  Enter X to exit the program  ***           |\n");
  printf ( "+=================================================+\n");
}

void printMenu() {
   printf ("+=================================================+\n");
   printf ("| 1 - Upload File                                 |\n");
   printf ("| 2 - Download File                               |\n");
   printf ("| 3 - Change File Permission                      |\n");
   printf ("| 4 - Delete File                                 |\n");
   printf ("+=================================================+\n");
   printf ("| X => Log out from this account                  |\n");
   printf ("| R => View your files & Exit Program             |\n");
   printf ("+=================================================+\n");
}



int main()
{
 int sockfd;

 struct sockaddr_in serverAddress;
 
 int WAITING = 0;
 int LOGIN = 1;
 int REGISTER = 2; 
 int MENU = 3;
 int UPLOAD= 4;
 int DOWNLOAD = 5;
 int CHMOD = 6;
 int DELETE = 7;
 int VIEW = 8;
 int STATE = WAITING; 
 int n;
 char msg1[MAXSZ];
 char msg2[MAXSZ];


 
 sockfd=socket(AF_INET,SOCK_STREAM,0);
 
 memset(&serverAddress,0,sizeof(serverAddress));
 serverAddress.sin_family=AF_INET;
 serverAddress.sin_addr.s_addr=inet_addr(SERVER_IP);
 serverAddress.sin_port=htons(PORT);


 connect(sockfd,(struct sockaddr *)&serverAddress,sizeof(serverAddress));



 //send to sever and receive from server
 while(1)
 {
	 switch(STATE){
	 case 0: {  
			   system ( "clear" );			    
			   printWelcome();
			    
			   
 			    scanf("%s", msg1);

			     if(strcmp(msg1,"X")!= 0)
			     {
				  send(sockfd,msg1,MAXSZ,0);  
				    
				     if(strcmp(msg1,"Y")==0)
				      		STATE = LOGIN;
				     else if(strcmp(msg1,"N")==0) 
						STATE = REGISTER;
				     else{ 
					printf("invalid choice");
					} 
				     bzero(msg1, sizeof(msg1));
				     break;
			     }
			     else
			     { printf("Bye.\n");exit(0);}
			}
	case 1: { //LOGIN
		char temp[MAXSZ]= "";
		
		system ( "clear" ); 
   		printf("LOGIN\nUsername:");
	        scanf("%s", msg1);
                strcat(temp,msg1);
		bzero(msg1, sizeof(msg1));
		strcat(temp,"@");
		printf("\nPassword:");
		scanf("%s", msg1);
	        strcat(temp,msg1);
		bzero(msg1, sizeof(msg1));
 		send(sockfd,temp,MAXSZ,0); 
		bzero(temp, sizeof(temp));
		recv(sockfd,msg1,MAXSZ,0); 
		printf("\n%s",msg1); 
		sleep( 1 );

		if(msg1[0]=='L') 
		 STATE = MENU;
		else
		 STATE = WAITING;

		bzero(msg1, sizeof(msg1));
		break;
		}
	case 2: { //REGISTER
		char temp[MAXSZ]= "";
		system ( "clear" );
		printf("REGISTER\nUsername:");
 		scanf("%s", msg1);
		strcat(temp,msg1);
		bzero(msg1, sizeof(msg1));
		strcat(temp,"@");
		printf("\nPassword:");
		scanf("%s", msg1);
	        strcat(temp,msg1);
		bzero(msg1, sizeof(msg1));
		send(sockfd,temp,MAXSZ,0); 
		bzero(temp, sizeof(temp));
		recv(sockfd,msg1,MAXSZ,0);
		printf("\n%s",msg1);
		sleep( 1 );
		bzero(msg1, sizeof(msg1));
		STATE = WAITING;
		break;
		}
	case 3 :{
			system ( "clear" );			    
			printMenu();
			scanf("%s", msg1);
				 if(strcmp(msg1,"1")== 0)     
			    	 STATE = UPLOAD;
				 else if(strcmp(msg1,"2")== 0)   
 				 STATE = DOWNLOAD;
				 else if(strcmp(msg1,"3")== 0)   
 				 STATE = CHMOD;
				 else if(strcmp(msg1,"4")== 0)   
 		 		 STATE = DELETE;
				 else if(strcmp(msg1,"X")== 0)  
				 STATE = WAITING;
				else if(strcmp(msg1,"R")== 0)   
				 STATE = VIEW;
				 else {
				  printf("Invalid choice. Try again...");
  				  sleep(3);
			          break;
				  }
			send(sockfd,msg1,MAXSZ,0);	

		break;
		}
 	case 4: { //UPLOAD
		system ( "clear" );
			printf("UPLOAD\nEnter the directory of your file: ");
			scanf("%s", msg1);
			
			send(sockfd, msg1, BUFSIZE,0);
			
			printf("Getting file size...\n");
			FILE *file;
			file = fopen(msg1, "r");
			int size;
			fseek(file, 0, SEEK_END);
			size = ftell(file);
			fseek(file, 0, SEEK_SET);
			printf("Size of file : %d \n",size);

			
			printf("Sending File Size...\n");
			write(sockfd, &size, sizeof(size));

			
			printf("Sending File as Byte Array...\n");
			char send_buffer[size];
			while(!feof(file)) {
			    fread(send_buffer, 1, sizeof(send_buffer), file);
			    write(sockfd, send_buffer, sizeof(send_buffer));
			    bzero(send_buffer, sizeof(send_buffer));}
			
			printf("Upload successful. Redirecting in 3s...\n");		
			sleep(3);


			STATE =MENU;
	 		 break;
			}
	case 5: { //DOWNLOAD
		system ( "clear" );
			printf("DOWNLOAD\n");	
			bzero(msg1, sizeof(msg1));
			recv(sockfd,msg1,MAXSZ,0);
			printf("Download List:\n%s\n",msg1);
			bzero(msg1, sizeof(msg1));
			printf("Enter file name to download:\n");
			scanf("%s", msg1);
			send(sockfd,msg1,MAXSZ,0);

			
				char buffer[BUFSIZE+1];
				//Read File Size
				recv(sockfd,buffer,BUFSIZE,0);
				printf("\nFile name: [%s]\n", buffer);
				printf("Reading File Size...\n");
				int size;
				read(sockfd, &size, sizeof(int));
				printf("File Size : %d \n", size);

		
				printf("Reading File Byte Array...\n");
				char p_array[size];
				read(sockfd, p_array, size);
	
				
				printf("Converting Byte Array to File...\n");
				printf("Saving file as '%s'\n",buffer); 
				FILE *file;
				file = fopen(buffer, "w");
				fwrite(p_array, 1, sizeof(p_array), file);
				fclose(file);

				

			
			printf("Download successful. Redirecting in 3s...\n");		
			sleep(3);

			STATE = MENU;
	 		 break;
			}
       case 6 : { //CHMOD
			printf("CHANGE FILE PERMISSION\n");	
			recv(sockfd,msg1,MAXSZ,0);
			printf("%s\n",msg1);
			bzero(msg1, sizeof(msg1));
			printf("\nPlease enter the name of the file to change permission.\n");
			scanf("%s", msg1);
			send(sockfd,msg1,MAXSZ,0);  
			bzero(msg1, sizeof(msg1));
			printf("\nPlease enter the permission (eg: '0777' / '0753')\n");
			scanf("%s", msg1);
   			send(sockfd,msg1,MAXSZ,0);
			bzero(msg1, sizeof(msg1));
			printf("File has been successfully updated. Redirecting in 3s...\n");
			sleep(3);

			STATE = MENU;
			break;
			}
	case 7 : { //	DELETE
			printf("DELETE FILE\n");
			recv(sockfd,msg1,MAXSZ,0);
			printf("%s\n",msg1);
			bzero(msg1, sizeof(msg1));
			printf("\nPlease enter the name of the file to delete.\n");
			scanf("%s", msg1);
			send(sockfd,msg1,MAXSZ,0);  
			bzero(msg1, sizeof(msg1));
			recv(sockfd,msg1,MAXSZ,0);
			printf("\n%s",msg1);
			printf(" Redirecting in 3s...\n");
			sleep(3);
			STATE = MENU;
			break;
			}
	case 8 : { //VIEW
		execl("/bin/ls","ls","-l",(char *)0);	
		break;
		}
	}

 
 }

 return 0;
}

