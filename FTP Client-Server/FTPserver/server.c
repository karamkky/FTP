#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/stat.h>
#include "sem.h"
#include <time.h>

#define PORT 8000
#define BUFSIZE 1024
#define TRUE 1
#define FALSE 0
#define MAXSZ 1024

void rvobj(int semid){

	if(semctl(semid,0,IPC_RMID,NULL) == -1)
		perror("\n semctl fails");
}

int initsem(key_t semkey){
int status=0, semid;
semun arg;

	if((semid = semget(semkey,1,SEMPERM|IPC_CREAT|IPC_EXCL)) == -1 ){ 
	if(errno == EEXIST)
		semid = semget (semkey, 1, 0);} 
	else{
 		arg.val =1; // initalize arg.val to 1
		status = semctl(semid, 0, SETVAL, arg);} 

if(semid== -1|| status == -1){
	perror("Initsem() fails");
	return (-1);}

return(semid);}

int p(int semid){

	struct sembuf p_buf;
	p_buf.sem_num = 0; 
	p_buf.sem_op = -1; 
	p_buf.sem_flg = SEM_UNDO; 
 
	if(semop(semid,&p_buf,1) == -1){
	perror("P() fails");
	exit(1);}
return(0);}

int v(int semid){

	struct sembuf v_buf;
	v_buf.sem_num = 0;
	v_buf.sem_op = 1; 
	v_buf.sem_flg = SEM_UNDO; 

if(semop(semid, &v_buf, 1)== -1){

perror("V (semid) fails");
exit(1); }
return(0);}

void handlesem(char cliaddr[], key_t skey, int fd[]){

	int semid;
	pid_t pid = getpid();

	if((semid = initsem(skey)) < 0)
		exit(1);

p(semid); 

//write to log (critical section)
 time_t current_time;
 current_time = time(NULL);
FILE *f = fopen("log.txt", "a"); 
	

 		  char counter[MAXSZ] ;
	  	  read(fd[0], counter, MAXSZ);
		  printf("\n%s\n",counter);
		

	fprintf(f,"(semid %d) %s has assessed system on %s", semid, cliaddr, ctime(&current_time));
	fclose(f);

//write to log

v(semid); 

}



char* validateLogin(char* info)
{
        strcat(info,"\n");
	int success = FALSE;
	FILE * fp;
       char * line = NULL;
       size_t len = 0;
       ssize_t read;
	
       fp = fopen("userdb.txt", "r");
       if (fp == NULL)
           exit(EXIT_FAILURE);

       while ((read = getline(&line, &len, fp)) != -1) {
          if(strcmp(line,info)== 0)
		  {success = TRUE;
		  break;}

       }

       fclose(fp);
       if (line)
           free(line);


	if(success)return "Login Successful.\n";
	else return "Invalid Credentials.Try Again.\n";

}

char* getUsername(char info[])
{
	
		 char *token;
   		token = strtok(info, "@");
		

return token;

}


void Register(char info[])
{
	FILE *f = fopen("userdb.txt", "a"); //a = append to eof
	if (f == NULL)
	{
	    printf("Error opening file!\n");
	    exit(1);
	}

	fprintf(f, "%s\n", info);

	fclose(f);

		char * usrname = getUsername(info);
		char newdir[] = "./";
		strcat(newdir,usrname);
		mode_t process_mask = umask(0);
		int result_code = mkdir(newdir, S_IRWXU | S_IRWXG | S_IRWXO);
		umask(process_mask);
}


int main()
{

 int sockfd;//create socket
 int newsockfd;//accept connection

 struct sockaddr_in serverAddress;//server receive on this address
 struct sockaddr_in clientAddress;//server sends to client on this address

 int WAITING = 0;
 int LOGIN = 1;
 int REGISTER = 2;
 int MENU = 3;
 int UPLOAD= 4;
 int DOWNLOAD = 5;
 int CHMOD = 6;
 int DELETE =7;
 int VIEW = 8;
 int STATE ; 

 int n;
 char msg[MAXSZ];
 char *token;
 int clientAddressLength;
 int pid;
 int usercount=0;
 int fd[2];


key_t semkey = 0x201;

//create socket
sockfd=socket(AF_INET,SOCK_STREAM,0);
//initialize the socket addresses
memset(&serverAddress,0,sizeof(serverAddress));
serverAddress.sin_family=AF_INET;
serverAddress.sin_addr.s_addr = inet_addr("10.0.90.59");
serverAddress.sin_port=htons(PORT);

 
 bind(sockfd,(struct sockaddr *)&serverAddress, sizeof(serverAddress)); //bind the socket with the server address and port

 listen(sockfd,5);

 while(1)
 {
 
  printf("\n*****server waiting for new client connection:*****\n");
  clientAddressLength=sizeof(clientAddress);
  newsockfd=accept(sockfd,(struct sockaddr*)&clientAddress,&clientAddressLength);
  usercount++;

  printf("connected to client: %s\n",inet_ntoa(clientAddress.sin_addr));

  if(pipe(fd)==-1)
	{perror("pipe error");
	exit(1);}
 

 char count[MAXSZ];
  snprintf(count, sizeof(count), "%d", usercount);
  char msg1[MAXSZ] = "user connected:";
  strcat(msg1,count);
  write(fd[1], msg1, MAXSZ);


  pid=fork();
  if(pid==0)
  { STATE = WAITING;
    char usr[MAXSZ];
    handlesem(inet_ntoa(clientAddress.sin_addr),semkey,fd);



   

   while(1)
   {
    
	 switch(STATE){
	 case 0: {
	         
		   recv(newsockfd,msg,MAXSZ,0);
		 	    
		    if(strcmp(msg,"Y")== 0)
			     STATE = LOGIN;
		    else if (strcmp(msg,"N")== 0)
			     STATE = REGISTER;

		break;
		}
	case 1: { 
		printf("[%s]Login: Waiting for credentials...\n",inet_ntoa(clientAddress.sin_addr));
		recv(newsockfd,msg,MAXSZ,0);
		printf("[%s]Login: Received username@password '%s'\n",inet_ntoa(clientAddress.sin_addr),msg);
		char* status = validateLogin(msg);
		
 	        send(newsockfd,status,MAXSZ,0); 
				if(status[0]=='L') 
				 {STATE = MENU; 
				
				memcpy(usr,getUsername(msg),MAXSZ);
				 printf("[%s]User'%s': %s ", inet_ntoa(clientAddress.sin_addr),usr,status);
				}
				else    	   
				 {STATE = WAITING;
				 printf("[%s]User: %s ", inet_ntoa(clientAddress.sin_addr),status);
				 }
			
		break;
		}
	case 2: { 
		
		printf("[%s]Register: Waiting for credentials...\n",inet_ntoa(clientAddress.sin_addr));
		recv(newsockfd,msg,MAXSZ,0);
		printf("[%s]Register: Received username@password '%s'\n",inet_ntoa(clientAddress.sin_addr),msg);	
		Register(msg);
		send(newsockfd,"New User Registered. Please login to continue\n",MAXSZ,0); 
		
		char * usrname = getUsername(msg);
		char temp[] = "./";
		strcat(temp,usrname);
		printf("[%s]Register: Client has succcesfully registered as user '%s'\n",inet_ntoa(clientAddress.sin_addr),usrname);
		printf("New user directory '%s' created.\n",temp);
	

		STATE = WAITING;
		 break;
		}
	case 3: { 
		
		recv(newsockfd,msg,MAXSZ,0);
		 if(strcmp(msg,"1")== 0)     
			STATE = UPLOAD;
		 else if(strcmp(msg,"2")== 0)   
 		 	STATE = DOWNLOAD;
		else if(strcmp(msg,"3")== 0)   
 		 	STATE = CHMOD;
		else if(strcmp(msg,"4")== 0)  
 		 	STATE = DELETE;
		else if(strcmp(msg,"R")== 0)   
				 STATE = VIEW;
		 else if(strcmp(msg,"X")== 0)  
			{STATE = WAITING;
			printf("[%s]Client logged out..\n",inet_ntoa(clientAddress.sin_addr));}

 		 break;
		}
	case 4: { 
				printf("[%s]Client: Request to upload..\n",inet_ntoa(clientAddress.sin_addr));
		
			        

				char newdir[] = "./";
				strcat(newdir,usr);
				strcat(newdir,"/");
				
				 

				char buffer[BUFSIZE+1];
				
				recv(newsockfd,buffer,BUFSIZE,0);
				printf("\nFile name: [%s]\n", buffer);
				strcat(newdir,buffer);
				printf("Reading File Size...\n");
				int size;
				read(newsockfd, &size, sizeof(int));
				printf("File Size : %d \n", size);

				
				printf("Reading File Byte Array...\n");
				char p_array[size];
				read(newsockfd, p_array, size);

				
				printf("Converting Byte Array to File...\n");
				printf("Save file to '%s'\n",newdir); 
				FILE *file;
				file = fopen(newdir, "w");
				fwrite(p_array, 1, sizeof(p_array), file);
				fclose(file);
			
			 printf("[%s]Client: Upload Complete..\n",inet_ntoa(clientAddress.sin_addr));
			STATE = MENU;
			break;
			}
	case 5: { 
	 			printf("[%s]Client: Request to download..\n",inet_ntoa(clientAddress.sin_addr));
				  
				char list[MAXSZ];
				bzero(list, sizeof(list));
				char newdir[MAXSZ] ;
				bzero(newdir, sizeof(newdir));
				strcat(newdir,"./");
				strcat(newdir,usr);
				
				 
				  DIR           *d;
				  struct dirent *dir;
				  d = opendir(newdir);
				  if (d)
				  {bzero(list, sizeof(list));
				   
				    while ((dir = readdir(d)) != NULL)
				    {
				      if (dir->d_type == DT_REG)
					  {   
					       strcat(list,dir->d_name);
					       strcat(list,"\n");
					  }
				    }

				    closedir(d);
				  }
				printf("%s",list);
				send(newsockfd,list,MAXSZ,0); 
				printf("[%s]Client: Waiting for user input..\n",inet_ntoa(clientAddress.sin_addr));
				bzero(msg, sizeof(msg));
				recv(newsockfd,msg,MAXSZ,0);
				printf("[%s]Client: Request to download '%s'..\n",inet_ntoa(clientAddress.sin_addr),msg);
				

				char udir[]= "./";
				strcat(udir,usr);
				strcat(udir,"/");
				strcat(udir,msg);

				
				
 
				send(newsockfd, msg, BUFSIZE,0);
				
				printf("Getting file size...\n");
				FILE *file;
				file = fopen(udir, "r");
				int size;
				fseek(file, 0, SEEK_END);
				size = ftell(file);
				fseek(file, 0, SEEK_SET);
				printf("Size of file : %d \n",size);

				
				printf("Sending File Size...\n");
				write(newsockfd, &size, sizeof(size));

				
				printf("Sending File as Byte Array...\n");
				char send_buffer[size];
				while(!feof(file)) {
			    	fread(send_buffer, 1, sizeof(send_buffer), file);
			    	write(newsockfd, send_buffer, sizeof(send_buffer));
			    	bzero(send_buffer, sizeof(send_buffer));}
			

				
				bzero(udir, sizeof(udir));
				 
				printf("Client download complete.\n");

			STATE = MENU;
 			break;
			}
      case 6 : { 
		printf("[%s]Client: Request to change permission..\n",inet_ntoa(clientAddress.sin_addr));
			  
			  char newdir[MAXSZ] ;
				bzero(newdir, sizeof(newdir));
				strcat(newdir,"./");
				strcat(newdir,usr);

			  DIR           *d;
			  struct dirent *dir;
			  d = opendir(newdir);
			  char name1[BUFSIZE+1];
			  char mode[4];
			  char b[BUFSIZE+1];
			  char list[MAXSZ];
			bzero(list, sizeof(list));

			  if (d)
			  {
			    while ((dir = readdir(d)) != NULL)
			    {
			      if (dir->d_type == DT_REG)
					{
						strcat(list,dir->d_name);
						strcat(list,"\n");
						printf("%s\n", dir->d_name);
						char a[]="";
					        strcpy(a,newdir);
						strcat(a,"/");
						memcpy(b,dir->d_name,BUFSIZE);
						strcat(a,b);
						strcat(list,"File path: (");
						strcat(list,a);
						strcat(list,")\n");
						printf("File path: (%s)\n",a);
			
						struct stat fileStat;
						stat(a,&fileStat);
			
					

						strcat(list,"File Permissions: \t");
						  (S_ISDIR(fileStat.st_mode)) ? strcat(list,"d") : strcat(list,"-");
						  (fileStat.st_mode & S_IRUSR) ? strcat(list,"r") : strcat(list,"-");
						  (fileStat.st_mode & S_IWUSR) ? strcat(list,"w") : strcat(list,"-");
						  (fileStat.st_mode & S_IXUSR) ? strcat(list,"x") : strcat(list,"-");
						  (fileStat.st_mode & S_IRGRP) ? strcat(list,"r") : strcat(list,"-");
						  (fileStat.st_mode & S_IWGRP) ? strcat(list,"w") : strcat(list,"-");
						  (fileStat.st_mode & S_IXGRP) ? strcat(list,"x") : strcat(list,"-");
						  (fileStat.st_mode & S_IROTH) ? strcat(list,"r") : strcat(list,"-");
						  (fileStat.st_mode & S_IWOTH) ? strcat(list,"w") : strcat(list,"-");
						  (fileStat.st_mode & S_IXOTH) ? strcat(list,"x") : strcat(list,"-");
						   strcat(list,"\n\n");



					}
			    }

			    closedir(d);
			  }
			printf("%s",list);
			send(newsockfd,list,MAXSZ,0); 
			recv(newsockfd,msg,MAXSZ,0);
			printf("%s\n",msg);
		               
				strcpy(name1,msg);
				char udir[]= "./";
				strcat(udir,usr);
				strcat(udir,"/");
				strcat(udir,msg);
				
			recv(newsockfd,msg,MAXSZ,0);
			printf("%s\n",msg);
 				
				strcpy(mode,msg);

			        int i;
			    i = strtol(mode, 0, 8);
			    if (chmod (udir,i) < 0)
			    {
				fprintf(stderr, "Error in chmod(%s, %s) - %d (%s)\n",
					name1, mode, errno, strerror(errno));
				exit(1);
			    }
				else
				{
					printf("\nFile path: (%s) has been successfully updated (Mode: %s)\n\n",udir,mode);
				}
			STATE = MENU;
			break;
			}
	case 7 : { 
			printf("[%s]Client: Request to delete file..\n",inet_ntoa(clientAddress.sin_addr));
			  
			  char newdir[MAXSZ] ;
				bzero(newdir, sizeof(newdir));
				strcat(newdir,"./");
				strcat(newdir,usr);

			  DIR           *d;
			  struct dirent *dir;
			  d = opendir(newdir);
			  char name1[BUFSIZE+1];
			  char mode[4];
			  char b[BUFSIZE+1];
			  char list[MAXSZ];
			bzero(list, sizeof(list));

			  if (d)
			  {
			    while ((dir = readdir(d)) != NULL)
			    {
			      if (dir->d_type == DT_REG)
					{
						strcat(list,dir->d_name);
						strcat(list,"\n");
						printf("%s\n", dir->d_name);
						char a[]="";
					        strcpy(a,newdir);
						strcat(a,"/");
						memcpy(b,dir->d_name,BUFSIZE);
						strcat(a,b);
						strcat(list,"File path: (");
						strcat(list,a);
						strcat(list,")\n");
						printf("File path: (%s)\n",a);
			
						struct stat fileStat;
						stat(a,&fileStat);
			
						

						strcat(list,"File Permissions: \t");
						  (S_ISDIR(fileStat.st_mode)) ? strcat(list,"d") : strcat(list,"-");
						  (fileStat.st_mode & S_IRUSR) ? strcat(list,"r") : strcat(list,"-");
						  (fileStat.st_mode & S_IWUSR) ? strcat(list,"w") : strcat(list,"-");
						  (fileStat.st_mode & S_IXUSR) ? strcat(list,"x") : strcat(list,"-");
						  (fileStat.st_mode & S_IRGRP) ? strcat(list,"r") : strcat(list,"-");
						  (fileStat.st_mode & S_IWGRP) ? strcat(list,"w") : strcat(list,"-");
						  (fileStat.st_mode & S_IXGRP) ? strcat(list,"x") : strcat(list,"-");
						  (fileStat.st_mode & S_IROTH) ? strcat(list,"r") : strcat(list,"-");
						  (fileStat.st_mode & S_IWOTH) ? strcat(list,"w") : strcat(list,"-");
						  (fileStat.st_mode & S_IXOTH) ? strcat(list,"x") : strcat(list,"-");
						   strcat(list,"\n\n");



					}
			    }

			    closedir(d);
			  }
			printf("%s",list);
			send(newsockfd,list,MAXSZ,0); 
			recv(newsockfd,msg,MAXSZ,0);
			printf("%s\n",msg);
		               
				strcpy(name1,msg);
				char udir[]= "./";
				strcat(udir,usr);
				strcat(udir,"/");
				strcat(udir,msg);
			
			  int ret = remove(udir);

			   if(ret == 0) 
			   {
			      strcpy(msg,"File deleted successfully"); 
			   }
			   else 
			   {
			     strcpy(msg,"Error: unable to delete the file");
			   }
			printf("%s\n",msg);
			send(newsockfd,msg,MAXSZ,0); 
			STATE = MENU;
			break;
			}
	case 8 : { //VIEW
	
		break;
		}

	}



   }
  exit(0);
  }
  else
  {
   close(newsockfd);
	 


  }
 }

 return 0;
}




