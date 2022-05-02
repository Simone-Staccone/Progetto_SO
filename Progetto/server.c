
//*** SERVER ***//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>

#include "local.h"
#define LISTENQ 1024





int verifica_username(char *usr_name);
int verifica_password(char *usr_name,char *passwd);
void addUtente(char *passwd);
void addPasswd(char *name,char *passwd);
void printUtenti();
void parseCmd(char **argv,char **szPrt);


char *path = "log/log.txt";
char *mittente;
char *dest;
char *oggetto;
char *testo;
char *tempo;
char *messaggio;




int main(int argc,char **argv)
{
	int port;
	int sck,ret,lst;
	struct sockaddr_in servAddr,clAddr;
	char msg[MAX_SIZE],last_msg[MAX_SIZE],last_file[MAX_SIZE];
	int conn,sin_size;
	char buff[MAX_SIZE];
	char log[MAX_SIZE];
	char passwd[MAX_SIZE];
	int fd;
	int i;
	char *l;
	time_t rawtime;
	struct tm* leggibile;
	char *endptr;
	char *szPort;
	char reading_buff[MAX_SIZE];

	//Controllo per vedere se il programma è stato lanciato con un numero corretto di argomenti
	if(argc < 2 || argc > 3)
	{
		puts("Error: ./server -p <PortNum>");
		exit(EXIT_FAILURE);
	}

	parseCmd(argv,&szPort);


	port = strtol(szPort,&endptr,10);
	
	if (*endptr != '\0')
	{ 
		puts("Strtol error");
		exit(-1);
	} 	
	
	printf("Server listening on port %d\n",port);


    struct timeval timeout;      
    timeout.tv_sec = 0;
    timeout.tv_usec = 500;
	
	
	if( signal(SIGPIPE, SIG_IGN) == SIG_ERR )
	{
		perror("signal SIGPIPE failed: ");
		exit(EXIT_FAILURE);
	}

	
	
reop:
	//Apro o creo il file che dovrà gestire il login, in particolare se il file è appena stato creato aggiungo subito un utente root di passsword 0 
	fd = open(path,O_RDWR|O_CREAT,0666);
	
	if(fd == -1)
	{
		if(errno == 2)
		{
			l = "mkdir data";
			system(l);
			l = "mkdir log";
			system(l);	
			goto reop;			
		}
		else
		{
			printf("Error opening file,errno returned %d,%s\n",errno,strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	l = "root:0 ";
	char rea[1];
	
	if(read(fd,rea,1) == -1)
	{
		puts("Error reading");
		exit(EXIT_FAILURE);
	}	
	
	if(rea[0] == 0)
	{
		i=0;

		while(i<strlen(l))
		{
			if((ret = write(fd,&l[i],1)) == -1)
			{
				if(errno == EPIPE)
				{
					puts("Broken pipe detected");
					goto start;
				}
				else
				{
					printf("Error writing, errno returned %d,%s\n",errno,strerror(errno));
					exit(EXIT_FAILURE);
				}
			}
			i++;
		}	
	}
	
	lseek(fd,0,SEEK_SET);

	close(fd);
	
	
	//Initializing last msg to empty
	memset(last_msg,0,MAX_SIZE);


	/* Creo un nuovo socket */
	sck = socket(AF_INET, SOCK_STREAM, 0);
	if(sck < 0)
	{
		printf("Error opening socket, errno returned %d,%s\n",errno,strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* bind */
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(port);
	
	ret = bind(sck,(const struct sockaddr *) &servAddr,sizeof(servAddr));
	if(ret < 0)
	{
		printf("Error binding,errno returned %d,%s\n",errno,strerror(errno));
		exit(1);
	}


	/* listen */
	lst = listen(sck,LISTENQ);
	
	if(lst < 0)
	{
		printf("Error listening in socket, errno returned %d,%s\n",errno,strerror(errno));
		exit(EXIT_FAILURE);	
	}



	while(1)
	{
		
		sin_size = sizeof(struct sockaddr_in);
		 		
		

start:
		/*Connect */
		conn = accept(sck,(struct sockaddr *)&clAddr, (socklen_t *)&sin_size);
		
		if(conn < 0)
		{
			printf("%s: Error accepting message,errno returned %d,%s\n",argv[0],errno,strerror(errno));
			exit(EXIT_FAILURE);
		}
		
		
		if (setsockopt (conn, SOL_SOCKET, SO_RCVTIMEO, &timeout,sizeof timeout) < 0)
        {
			puts("Error in socketoption 1");
			err;
		}

		if (setsockopt (conn, SOL_SOCKET, SO_SNDTIMEO, &timeout,sizeof timeout) < 0)
        {
			puts("Error in socketoption 2");
			err;
		}




		//Pulisco le aree di memoria che utilizzo come stringhe per leggere il messaggio dal client
		memset(last_msg,0,MAX_SIZE);
		
		memcpy(last_msg,msg,strlen(msg));
		
		memset(msg,0,MAX_SIZE);

		memset(reading_buff,0,MAX_SIZE);

	
		i = 0;
		
		
		do{
			if((ret = read(conn,&msg[i],1)) == -1)
			{
				if(errno == 11)
				{
					break;
				}
				else
				{
					printf("Error reading 5, errno returned %d,%s\n",errno,strerror(errno));
					exit(EXIT_FAILURE);
				}
			}
			i++;
		}while(ret > 0);	

		



		/* Controllo su last_msg per vedere se entrare in registrazione di un nuovo utente */ 
		if(strncmp(last_msg,sys_reg,strlen(sys_reg)) == 0)
		{
			char trylog[strlen(msg)-strlen(sys_reg)];
			int res;
			char temp[MAX_SIZE];


			
			memset(last_msg,0,MAX_SIZE);

			//Copio il nome utente arrivato da Client
			for(int i = 0;i<=strlen(msg)-strlen(sys_log);i++)
			{
				printf("%c ",msg[i+strlen(sys_log)]);
				trylog[i] = msg[i+strlen(sys_log)];	
			}
			
			
			
			//Verifico se l'utente è già registrato
			if(verifica_username(trylog) == 1)
			{
				puts("User already exist");
				memset(last_msg,0,MAX_SIZE);
				memset(msg,0,MAX_SIZE);
				

				i=0;

				while(i<strlen(log_error_2))
				{
					if((ret = write(fd,&log_error_2[i],1)) == -1)
					{
						if(errno == EPIPE)
						{
							puts("Broken pipe detected");
							goto start;
						}
						else
						{
							printf("Error writing, errno returned %d,%s\n",errno,strerror(errno));
							exit(EXIT_FAILURE);
						}
					}
					i++;
				}
			}
			else
			{
				i=0;

				while(i<strlen(log_succ))
				{
					if((ret = write(fd,&log_succ[i],1)) == -1)
					{
						if(errno == EPIPE)
						{
							puts("Broken pipe detected");
							goto start;
						}
						else
						{
							printf("Error writing, errno returned %d,%s\n",errno,strerror(errno));
							exit(EXIT_FAILURE);
						}
					}
					i++;
				}
				printf("New User!!!,%s\n",trylog);
				fflush(stdout);
				
				
				//L'utente non è registrato,allora lo registro aggiungendolo a log/log.txt
				addUtente(trylog);
				
				
				
				memset(temp,0,MAX_SIZE);
			
				sprintf(temp,"data/%s.txt",trylog);
				printf("%s\n",temp);
				fflush(stdout);
			
			
				//Creo il file nella directory data/<NOME_UTENTE>.txt
				res = open(temp,O_CREAT|O_TRUNC|O_RDWR,0666);
			
				if(res == -1)
				{
					puts("Error opening/creating file");
					exit(EXIT_FAILURE);
				}


				close(res);

				}

		}
		/* Controllo su msg per fare il login di un utente */ 
		//else if(msg[0] == 'l' && msg[1] == 'o' && msg[2] == 'g' && msg[3] == ' ')
		else if(strncmp(msg,sys_log,strlen(sys_log)) == 0)
		{
			char trylog[strlen(msg)-strlen(sys_log)];		
			char risposta[MAX_SIZE];
			int res;	

			//In trylog metto il nome utente in arrivo da Client
			memset(trylog,0,strlen(msg)-strlen(sys_log));
			for(int i = 0;i<strlen(msg)-strlen(sys_log)+1;i++)
			{
				trylog[i] = msg[i+strlen(sys_log)];
			}
	
			//Verifico se il nome utente è corretto,ovvero se esiste un profilo associato al nome arrivato da Client
			if((res = verifica_username(trylog)) == 0)
			{
				i=0;

				while(i<strlen(log_error))
				{
					if((ret = write(conn,&log_error[i],1)) == -1)
					{
						if(errno == EPIPE)
						{
							puts("Broken pipe detected");
							goto start;
						}
						else
						{
							printf("Error writing, errno returned %d,%s\n",errno,strerror(errno));
							exit(EXIT_FAILURE);
						}
					}
					i++;
				}
			}
			else if(res == 1)
			{
				i=0;

				while(i<strlen(log_succ))
				{
					if((ret = write(conn,&log_succ[i],1)) == -1)
					{
						if(errno == EPIPE)
						{
							puts("Broken pipe detected");
							goto start;
						}
						else
						{
							printf("Error writing, errno returned %d,%s\n",errno,strerror(errno));
							exit(EXIT_FAILURE);
						}
					}
					i++;
				}
				printf("Try logging %s\n",trylog); //Aspetto la verifica della password per confermare il log in avvenuto con successo
				fflush(stdout);
				
				
				//Svuoto log e gli inserisco il nuovo nome utente da ricordare
				memset(log,0,MAX_SIZE);
				
				memcpy(log,trylog,strlen(trylog));
			}	

		}
		/* Controllo su msg per verificare la password inserita di un utente */ 
		//else if(msg[0] == 'p' && msg[1] == 's' && msg[2] == 'w' && msg[3] == ' ')
		else if(strncmp(msg,sys_psw,strlen(sys_psw)) == 0)
		{
			char trypsswd[strlen(msg)-strlen(sys_psw)];		
			int res;	
			char temp_name[strlen(msg)-strlen(sys_psw)];
			char temp_psswd[strlen(msg)-strlen(sys_psw)];
			int flg = 0,j = 0;
		
			//tryppswd contiene la password inserita da Client
			memset(trypsswd,0,strlen(msg)-strlen(sys_psw));

			for(int i = 0;i<strlen(msg)-strlen(sys_psw)-1;i++)
			{
				trypsswd[i] = msg[i+strlen(sys_psw)];
			}
	
			for(int i = 0;i<strlen(trypsswd);i++)
			{
				if(flg == 1)
				{
					temp_name[j] = trypsswd[i];
					j++;
				}
				else if(trypsswd[i] == ' ')
				{
					temp_psswd[i] = 0;
					flg = 1;
				}
				else
				{
					temp_psswd[i] = trypsswd[i];
				}
			}
			temp_name[j] = 0;	
	
			//Verifico se la password associata all'utente, che è salvato in log, è corretta
			if((res = verifica_password(temp_name,temp_psswd)) == 0)
			{
				i=0;

				while(i<strlen(passwd_err))
				{
					if((ret = write(conn,&passwd_err[i],1)) == -1)
					{
						if(errno == EPIPE)
						{
							puts("Broken pipe detected");
							goto start;
						}
						else
						{
							printf("Error writing, errno returned %d,%s\n",errno,strerror(errno));
							exit(EXIT_FAILURE);
						}
					}
					i++;
				}
			}
			else
			{
				i=0;

				while(i<strlen(passwd_succ))
				{
					if((ret = write(conn,&passwd_succ[i],1)) == -1)
					{
						if(errno == EPIPE)
						{
							puts("Broken pipe detected");
							goto start;
						}
						else
						{
							printf("Error writing, errno returned %d,%s\n",errno,strerror(errno));
							exit(EXIT_FAILURE);
						}
					}
					i++;
				}
				printf("Logged %s\n",log); //Confermo il login avvenuto
				fflush(stdout);
			}	

		}
		/* Controllo su msg per registrare la password di un nuovo utente */ 
		//else if(msg[0] == 'p' && msg[1] == 's' && msg[2] == 'w' && msg[3] == 'r' && msg[4] == ' ')
		else if(strncmp(msg,sys_pswr,strlen(sys_pswr)) == 0)
		{
			char trypsswd[strlen(msg)-strlen(sys_pswr)];		
			int res;	
			char temp[MAX_SIZE];

			puts("Entrato");


			//In trypsswd c'è la password inserita da Client da registrare su log.txt relativamente all'utente salvato nella variabile log
			memset(trypsswd,0,strlen(msg)-strlen(sys_pswr));

			for(int i = 0;i<strlen(msg)-strlen(sys_pswr)+1;i++)
			{
				trypsswd[i] = msg[i+strlen(sys_pswr)];
			}
	
	
			//Aggiungo tryppswd come password di log
			addPasswd(log,trypsswd);


			puts("Succesfully registered"); //Se arrivo a questo punto la registrazione,>NOME_UTENTE> + <passwd>, è stata completata con successo
			fflush(stdout);

		}
		/* Inizio a ricevere il messaggio stesso, sono finite le operazioni di login e registrazione */
		/* Controllo su msg per ricevere il nome di chi ha inviato il messaggio */ 		
		//else if(msg[0] == 'M' && msg[1] == 'i' && msg[2] == 't' && msg[3] == 't' && msg[4] == 'e' && msg[5] == 'n' && msg[6] == 't' && msg[7] == 'e' && msg[8] == ':')
		else if(strncmp(msg,sys_mittente,strlen(sys_mittente)) == 0)
		{
			int h = 0,k = 0;
			int verify = 0;
			
			messaggio = (char *)malloc(sizeof(char)*strlen(msg));
			
			if(messaggio == NULL)
			{
				puts("Error in malloc 2");
				err;
			}
			
			dest = (char *)malloc(sizeof(char)*strlen(msg));
			
			if(dest == NULL)
			{
				puts("Error in malloc 1");
				err;
			}
						
			sprintf(messaggio,"%s$",msg);
		
		
			verify = 0;
			
			while(k<strlen(messaggio))
			{
				if(verify == 1)
				{
					if(messaggio[k] == ':')
					{
						k++;
						while(messaggio[k] != '$')
						{;
							dest[h] = messaggio[k];
							h++;
							k++;
						}
						break;
					}
				}
				
				if(messaggio[k] == '$')
				{
					verify++;
				}
				k++;
			}
			
			if(verifica_username(dest) == 0)
			{
				i=0;

				while(i<strlen(log_error_3))
				{
					if((ret = write(conn,&log_error_3[i],1)) == -1)
					{
						if(errno == EPIPE)
						{
							puts("Broken pipe detected");
							goto start;
						}
						else
						{
							printf("Error writing, errno returned %d,%s\n",errno,strerror(errno));
							exit(EXIT_FAILURE);
						}
					}
					i++;
				}	
			}
			else
			{
				char prot[strlen(dest)+strlen(suffisso)+strlen(acc_file)];
				int fd;
				
				sprintf(prot,"%s%s%s",acc_file,dest,suffisso);
				
				
				fd = open(prot,O_RDWR);
				
				if(fd == -1)
				{
					puts("Error opening");
					err;
				}

				lseek(fd,0,SEEK_END);

				i=0;

				while(i<strlen(messaggio))
				{
					if((ret = write(fd,&messaggio[i],1)) == -1)
					{
						if(errno == EPIPE)
						{
							puts("Broken pipe detected");
							goto start;	
						}
						else
						{
							printf("Error writing, errno returned %d,%s\n",errno,strerror(errno));
							exit(EXIT_FAILURE);
						}
					}
					i++;
				}						

				memset(buff,0,MAX_SIZE);
	
				time (&rawtime);
				leggibile = localtime(&rawtime);
	
				sprintf(buff,"%d:%d:%d %d/%d/%d",leggibile->tm_hour,leggibile->tm_min,leggibile->tm_sec,leggibile->tm_mday,leggibile->tm_mon+1,leggibile->tm_year+1900);


				i=0;

				while(i<strlen(buff))
				{
					if((ret = write(fd,&buff[i],1)) == -1)
					{
						if(errno == EPIPE)
						{
							puts("Broken pipe detected");
							goto start;	
						}
						else
						{
							printf("Error writing, errno returned %d,%s\n",errno,strerror(errno));
							exit(EXIT_FAILURE);
						}
					}
					i++;
				}	

				if((ret = write(fd,&bn[0],1)) == -1)
				{
					if(errno == EPIPE)
					{
						puts("Broken pipe detected");
						goto start;			
					}
					else
					{
						printf("Error writing, errno returned %d,%s\n",errno,strerror(errno));
						exit(EXIT_FAILURE);
					}
				}
							

				close(fd);
							
				i=0;

				while(i<strlen(log_succ))
				{
					if((ret = write(conn,&log_succ[i],1)) == -1)
					{
						if(errno == EPIPE)
						{
							puts("Broken pipe detected");
							goto start;		
						}
						else
						{
							printf("Error writing, errno returned %d,%s\n",errno,strerror(errno));
							exit(EXIT_FAILURE);
						}
					}
					i++;
				}				
			}

		}
		
		
		/* Controllo su msg per leggere i messaggi ricevuti da un utente */ 	
		//else if(msg[0] == 's' && msg[1] == 'h' && msg[2] == 'o' && msg[3] == 'w' && msg[4] == ' ')
		else if(strncmp(msg,sys_show,strlen(sys_show)) == 0)
		{
			char name[strlen(msg)+strlen(sys_show)];
			int fd;
			int flag = 0;
			int i;
			char c[1];
			char kj[MAX_SIZE];
			memset(name,0,strlen(msg)+strlen(sys_show));
			
			strcat(name,"data/");
			
			for(i = strlen(name);i<strlen(msg);i++)
			{
				name[i] = msg[i];
			}
			strcat(name,".txt");
			
			memset(last_file,0,MAX_SIZE);
			
			memcpy(last_file,name,strlen(name));
			
			printf("Trying to access to the messages of %s\n",name);
			fflush(stdout);
			
			fd = open(name,O_RDONLY,O_CREAT,0666);
			
			if(fd == -1)
			{
				puts("Error opening");
				exit(EXIT_FAILURE);
			}
			

			i = 0;
			memset(kj,0,MAX_SIZE);
fil:
			ret = read(fd,c,1);
			
			if(ret == -1)
			{
				puts("Error reading");
				exit(EXIT_FAILURE);
			}
			
			//Leggo finché non incotro per la terza volta il simbolo $, infatti devo mostrare al client tutto meno 
			//che il testo del messaggio, allora invio al client tutto quello che sta prima del secondo $ e ricomincio 
			//a contare i $ quando incontro il terminatore di linea
			if(ret > 0)
			{
				if(flag == 0)
				{
					if(c[0] == '$')
					{
						kj[i] = ' ';
						i++;
						flag = 1;
					}
					else
					{
						kj[i] = c[0];
						i++;
					}
				}
				else if(flag == 1)
				{
					if(c[0] == '$')
					{
						kj[i] = ' ';
						i++;
						flag = 2;
					}
					else
					{
						kj[i] = c[0];
						i++;
					}
				}
				else if(flag == 2)
				{
					if(c[0] == '$')
					{
						flag = 3;
					}
					else
					{
						kj[i] = c[0];
						i++;
					}						
				}				
				else if(flag == 3)
				{
					if(c[0] == '\n')
					{
						kj[i] = c[0];
						
						i=0;

						while(i<strlen(kj))
						{
							if((ret = write(conn,&kj[i],1)) == -1)
							{
								if(errno == EPIPE)
								{
									puts("Broken pipe detected");
									goto start;
								}
								else
								{
									printf("Error writing, errno returned %d,%s\n",errno,strerror(errno));
									exit(EXIT_FAILURE);
								}
							}
							i++;
						}
						
						
						flag = 0;
						memset(kj,0,MAX_SIZE);
						i = 0;
					}
				}
				goto fil;			
			}
		
	
			close(fd);

			
		}
		
		
		
		/* Controllo su msg per ottenere il testo del messaggio che il client vuole leggere */ 	
		//else if(msg[0] == 't' && msg[1] == 'e' && msg[2] == 'x' && msg[3] == ' ')
		else if(strncmp(msg,sys_tex,strlen(sys_tex)) == 0)
		{
			char nm[strlen(msg)-strlen(sys_tex)];
			int i = 0;
			int fd;
			char ris[MAX_SIZE];
			char c[1];
			int flag = 0;
			int end = 0;
			int n;
			
			
			for(i=0;i<=strlen(msg)-strlen(sys_tex);i++)
			{
				nm[i] = msg[i+strlen(sys_tex)];
			}
			n = strtol(nm,0,10);
			printf("Trying to access to the text of %s,%s\n",last_file,nm);
			fflush(stdout);
			
			
			fd = open(last_file,O_RDONLY,0666);
			
			if(fd == -1)
			{
				puts("Error opening");
				exit(EXIT_FAILURE);
			}
			end = 0;
			n--;
			
fil2:
			ret = read(fd,c,1);
			
			if(ret == -1)
			{
				puts("Error reading");
				exit(EXIT_FAILURE);
			}
			
			//Dal client leggo il numero di messaggi che devo saltare, quindi leggo n volte il terminatore di stringhe
			//per poi usare il solito parsing, ovvero invio al client solo dopo aver letto 2 $ fino al prossimo terminatore di stringa
			
			
			if(ret > 0 && end == 0)
			{
				if(n > 0)
				{
					if(c[0] == '\n')
					{
						n--;
					}
				}
				else if(flag == 0)
				{
					if(c[0] == '$')
					{
						flag = 1;
					}
				}
				else if(flag == 1)
				{
					if(c[0] == '$')
					{
						flag = 2;
					}			
				}
				else if(flag == 2)
				{
					if(c[0] == '$')
					{
						flag = 3;
					}			
				}
				else if(flag == 3)
				{
					if(c[0] != '\n')
					{
						if(write(conn,c,1) == -1)
						{
							puts("Error writing");
							exit(EXIT_FAILURE);				
						}
					}
					else
					{
						end = 1;
					}
				}
				goto fil2;			
			}
			
		
			close (fd);

		}
		/* Controllo su msg per effettuare il log off dell'utente */ 	
		//else if(msg[0] == 'l' && msg[1] == 'o' && msg[2] == 'g' && msg[3] == 'o' && msg[4] == 'f' && msg[5] == 'f' && msg[6] == ' ')
		else if(strncmp(msg,sys_logoff,strlen(sys_logoff)) == 0)
		{
			char m[strlen(msg)-strlen(sys_logoff)];
			
			for(int i = 0;i<=strlen(msg)-strlen(sys_logoff);i++)
			{
				m[i] = msg[i+strlen(sys_logoff)];
			}
			
			printf("%s logged off\n",m);
			fflush(stdout);
			close(conn);
		}
		/* Controllo su msg per ricevere il numero del messaggio da cancellare */ 		
		//else if(msg[0] == 'd' && msg[1] == 'e' && msg[2] == 'l' && msg[3] == ' ')
		else if(strncmp(msg,sys_del,strlen(sys_del)) == 0)
		{
			char nm[strlen(msg)-strlen(sys_del)],c[1];
			int i;
			int n,fd,fd2,flg;
			char *temp;
			
			memset(nm,0,strlen(msg)-strlen(sys_del));
			
			for(i=0;i<strlen(msg)-strlen(sys_del)-1;i++)
			{
				nm[i] = msg[i+strlen(sys_del)];
			}
			n = strtol(nm,0,10);
			printf("Trying to delate message number %s of file %s\n",nm,last_file);
			fflush(stdout);
			
			
			
			fd = open(last_file,O_RDONLY,0666);
			
			if(fd == -1)
			{
				puts("Error opening");
				exit(EXIT_FAILURE);
			}
			flg = 0;
			n--;
			
			
			fd2 = open("yg.txt",O_RDWR|O_CREAT|O_TRUNC,0666);
			
			if(fd2 == -1)
			{
				puts("Error opening");
				exit(EXIT_FAILURE);
			}	
	
fil3:
			ret = read(fd,c,1);
			
			if(ret == -1)
			{
				puts("Error reading");
				exit(EXIT_FAILURE);
			}
			
			//Dal client leggo il numero di messaggi che devo saltare, quindi leggo n volte il terminatore di stringhe 
			//e copio tutti i caretteri che incontro nel file temporaneo tranne quelli della riga in questione
			
			
			if(ret > 0)
			{
				if(n > 0)
				{
					if(write(fd2,c,1) == -1)
					{
						if(errno==EPIPE)
						{
							puts("Broken pipe detected");
							goto start;
						}
						else
						{
							puts("Error writing");
							exit(EXIT_FAILURE);
						}
					}
					if(c[0] == '\n')
					{
						n--;
					}
				}
				else
				{
					if(flg == 1)
					{
						if(write(fd2,c,1) == -1)
						{
							puts("Error writing");
							exit(EXIT_FAILURE);
						}	
					}
					else if(c[0] == '\n')
					{
						flg = 1;;
					}			
				}
				goto fil3;			
			}
			i=0;

			while(i<strlen(del_succ))
			{
				if((ret = write(conn,&del_succ[i],1)) == -1)
				{
					if(errno == EPIPE)
					{
						puts("Broken pipe detected");
						goto start;
					}
					else
					{
						printf("Error writing, errno returned %d,%s\n",errno,strerror(errno));
						exit(EXIT_FAILURE);
					}
				}
				i++;
			}
		
			close (fd);
			close(fd2);
			
			temp = (char *)malloc((sizeof(char)*strlen(last_file))+13);
			sprintf(temp,"rm %s",last_file);
			system(temp);
			
			memset(temp,0,strlen(last_file)+13);

			sprintf(temp,"mv yg.txt %s",last_file);
			
			system(temp);

		}
		else //Else utilizzato per verificare nel server se avvengono scambi di messaggi che non rientrano nei casi precedenti e quindi per capire da dove proviene il bug
		{
			printf("%s:TCP:%u:%s\n",inet_ntoa(clAddr.sin_addr),ntohs(clAddr.sin_port),msg);
		}
		close(conn);
	}

	close(sck);
	return 0;
}









void printUtenti()
{
	int flg = 0;
	int i = 0;
	int ret;
	char c[1];
	char temp[MAX_SIZE];
	int fd = open(path,O_RDONLY,0666);
	
	if(fd == -1)
	{
		puts("Error opening");
		exit(EXIT_FAILURE);
	}

	memset(temp,0,MAX_SIZE);


	//Leggo dal file e stampo su standar output
	while(1){
		ret = read(fd,c,1);
		if(ret == -1)
		{
			puts("Error reading");
			exit(EXIT_FAILURE);
		}
		if(ret == 0) break;
		
		if(flg == 0)
		{
			if(c[0] == ':')
			{
				temp[i+1] = 0;
				printf("%s\n",temp);
				fflush(stdout);
				flg = 1;
				i = 0;
			}
			else
			{
				temp[i] = c[0];
				i++;
			}
		}
		else
		{
			if(c[0] == '\n')
			{
				flg = 0;
			}
			memset(temp,0,MAX_SIZE);
		}

	}

	close(fd);
	
}








int verifica_username(char *name)
{
	int flag = 0,flg = 0;;
	int i = 0;
	int ret;
	char c[1];
	char temp[MAX_SIZE];
	int fd = open(path,O_RDONLY,0666);
	
	if(fd == -1)
	{
		puts("Error opening");
		exit(EXIT_FAILURE);
	}

	//Controllo sul file per verificare se un utente esiste già,controllo prima di ogni carattere : e salvo sulla variabile temp
	//Faccio un confronto tra temp e il nome dell'utente arrivato dal client 
	
	memset(temp,0,MAX_SIZE);
	while(1){
		ret = read(fd,c,1);
		if(ret == -1)
		{
			puts("Error reading");
			exit(EXIT_FAILURE);
		}
		if(ret == 0) break;
	
		if(flag == 0)
		{
			if(flg == 0)
			{
				if(c[0] == ':')
				{			
					if(strncmp(temp,name,strlen(name)) == 0)
					{
						flag = 1;
						temp[i] = ' ';
						i++;
					}
					flg = 1;
					i = 0;
				}
				else
				{
					temp[i] = c[0];
					i++;
				}
			}
			else
			{
				if(c[0] == '\n')
				{
					flg = 0;
				}
			}
		}
		else
		{
			break;
		}

	}

	
	close(fd);
	return flag;
}







int verifica_password(char *name,char *passwd)
{
	int flag = 0,flg = 0;;
	int i = 0;
	int ret;
	char c[1];
	char temp[MAX_SIZE];
	int fd = open(path,O_RDONLY,0666);
	
	if(fd == -1)
	{
		puts("Error opening");
		exit(EXIT_FAILURE);
	}

	//Per verificare se la password inserita è corretta scorro fino a trovare il nome utente inserito dal client in precedenza, per poi controllare quello che c'è scritto dopo i :
	//se c'è un match tra le due password allora la funzione ritorna 1, altrimenti ritorna 0
	
	memset(temp,0,MAX_SIZE);
	while(1){
		ret = read(fd,c,1);
		if(ret == -1)
		{
			puts("Error reading");
			exit(EXIT_FAILURE);
		}
		if(ret == 0) break;
	
		if(flag == 0)
		{
			if(flg == 0)
			{
				if(c[0] == ':')
				{			
					if(strncmp(temp,name,strlen(name)) == 0)
					{
						flag = 1;
						memset(temp,0,MAX_SIZE);
						i=0;
					}
					flg = 1;
					i = 0;
				}
				else
				{
					temp[i] = c[0];
					i++;
				}
			}
			else
			{
				if(c[0] == '\n')
				{
					flg = 0;
				}
			}
		}
		else
		{
			if(c[0] != '\n')
			{
				temp[i] = c[0];
				i++;
			}
			else if(strcmp(temp,passwd) == 0)
			{
				if(strlen(passwd) != strlen(temp))
				{
					flag = 0;
				}
				else
				{
					flag = 1;
				}
				break;
			}
			else
			{
				flag = 0;
				break;
			}
		}

	}

	
	close(fd);
	return flag;	
}








//Inserisco una passwor din fondo al file
void addPasswd(char *name,char *passwd)
{
	int i;
	int fd = open(path,O_RDWR,0666);
	int ret;

	if(fd == -1)
	{
		puts("Error opening");
		exit(EXIT_FAILURE);
	}

	puts("Adding passwd...");

	lseek(fd,-1,SEEK_END);
	

	i=0;

	while(i<strlen(passwd))
	{
		if((ret = write(fd,&passwd[i],1)) == -1)
		{
			if(errno == EPIPE)
			{
				puts("Broken pipe detected");
			}
			else
			{
				printf("Error writing, errno returned %d,%s\n",errno,strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		i++;
	}
	if(write(fd,&bn[0],1) == -1)
	{
		puts("Error writing");
		exit(EXIT_FAILURE);
	}


	printf("[+]Server database updated\n");
	fflush(stdout);



	close(fd);
}








//Inserisco un nome utente seguit da : in fondo al file 
void addUtente(char *name)
{
	int i;
	int ret;
	int fd = open(path,O_RDWR,0666);
	char temp[strlen(name)+2];	

	memset(temp,0,strlen(name));	
	sprintf(temp,"\n%s: ",name);


	if(fd == -1)
	{
		puts("Error opening");
		exit(EXIT_FAILURE);
	}

	puts("Adding new usr...");

	lseek(fd,-1,SEEK_END);
	

	i=0;

	while(i<strlen(temp))
	{
		if((ret = write(fd,&temp[i],1)) == -1)
		{
			if(errno == EPIPE)
			{
				puts("Broken pipe detected");
			}
			else
			{
				printf("Error writing, errno returned %d,%s\n",errno,strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		i++;
	}


	printf("[+]Server database updated\n");
	fflush(stdout);



	close(fd);

	printUtenti();
	
}




void parseCmd(char **argv,char **szPrt)
{
	int i = 1;
	int flag = 0;
	
	while(argv[i] != NULL)
	{
		if(strcmp(argv[i],"-p") == 0)
		{
			*szPrt = argv[i+1];
			i = i+2;
			flag = 1;
		}
		else
		{
			puts("Error, program should be launched as ./server -p <PortNumber>");
			fflush(stdout);
			exit(-1);
		}
	}
	if(flag == 0)
	{
		puts("Error,missing port number,program should be launched as ./server -p <PortNum>");
		fflush(stdout);
		exit(-1);
	}
}