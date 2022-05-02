
//*** SERVER ***//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "local.h"
#define LISTENQ 1024

char *path = "log/log.txt";


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

//Inserisco un nome utente seguit da : in fondo al file 
void addUtente(char *name)
{
	int i;
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

	lseek(fd,0,SEEK_END);
	

	if(write(fd,temp,strlen(temp)) == -1)
	{
		puts("Error writing");
		exit(EXIT_FAILURE);
	}


	printf("[+]Server database updated\n");
	fflush(stdout);



	close(fd);

	printUtenti();
	
}

//Inserisco una passwor din fondo al file
void addPasswd(char *name,char *passwd)
{
	int i;
	int fd = open(path,O_RDWR,0666);


	if(fd == -1)
	{
		puts("Error opening");
		exit(EXIT_FAILURE);
	}

	puts("Adding passwd...");

	lseek(fd,-1,SEEK_END);
	

	if(write(fd,passwd,strlen(passwd)) == -1)
	{
		puts("Error writing");
		exit(EXIT_FAILURE);
	}


	printf("[+]Server database updated\n");
	fflush(stdout);



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
			else if(strncmp(temp,passwd,strlen(passwd)) == 0)
			{
				flag = 1;
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


void thread_fun(void *l)
{
	long int conn = (long int)l;
	char msg[MAX_SIZE],last_msg[MAX_SIZE],last_file[MAX_SIZE];
	char buff[MAX_SIZE];
	char log[MAX_SIZE];
	char passwd[MAX_SIZE];
	char *mittente;
	char *dest;
	char *oggetto;
	char *testo;	
	int ret;
	struct sockaddr_in clAddr;
	

	//Pulisco le aree di memoria che utilizzo come stringhe per leggere il messaggio dal client
	memset(last_msg,0,MAX_SIZE);
		
	memcpy(last_msg,msg,strlen(msg));
		
	memset(msg,0,MAX_SIZE);	
	
	
	/* Read dal client */
	ret = read(conn,msg,MAX_SIZE);		
	
	if(ret == -1)
	{
		printf("Error ret returned :%d %d %s\n",ret,errno,strerror(errno));
		exit(EXIT_FAILURE);
	}


	//printf("il messaggio è:%s\n",msg);
	fflush(stdout);	



	/* Controllo su last_msg per vedere se entrare in registrazione di un nuovo utente */ 
	if(last_msg[0] == 'l' && last_msg[1] == 'o' && last_msg[2] == 'g' && last_msg[3] == ' ' && last_msg[4] == 'r' && last_msg[5] == '=')
	{
		char trylog[strlen(msg)-4];
		int res;
		char temp[MAX_SIZE];


		//Copio il nome utente arrivato da Client
		for(int i = 0;i<strlen(msg)-3;i++)
		{
			trylog[i] = msg[i+4];
		}
			
		//Verifico se l'utente è già registrato
		if(verifica_username(trylog) == 1)
		{
			puts("User already exist");
			memset(last_msg,0,MAX_SIZE);
			memset(msg,0,MAX_SIZE);
				

			if(write(conn,log_error_2,strlen(log_error_2)) == -1)
			{
				puts("Error writing");
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			if(write(conn,log_succ,strlen(log_succ)) == -1)
			{
				puts("Error writing");
				exit(EXIT_FAILURE);
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
	else if(msg[0] == 'l' && msg[1] == 'o' && msg[2] == 'g' && msg[3] == ' ')
	{
		char trylog[strlen(msg)-4];		
		char risposta[MAX_SIZE];
		int res;	

		//In trylog metto il nome utente in arrivo da Client
		memset(trylog,0,strlen(msg)-4);
		for(int i = 0;i<strlen(msg)-4;i++)
		{
			trylog[i] = msg[i+4];
		}
	
		//Verifico se il nome utente è corretto,ovvero se esiste un profilo associato al nome arrivato da Client
		if((res = verifica_username(trylog)) == 0)
		{
			if(write(conn,log_error,strlen(log_error)) == -1)
			{
				puts("Error writing");
				exit(EXIT_FAILURE);
			}
		}
		else if(res == 1)
		{
			if(write(conn,log_succ,strlen(log_succ)) == -1)
			{
				puts("Error writing");
				exit(EXIT_FAILURE);
			}
			printf("Try logging %s\n",trylog); //Aspetto la verifica della password per confermare il log in avvenuto con successo
			fflush(stdout);
				
				
			//Svuoto log e gli inserisco il nuovo nome utente da ricordare
			memset(log,0,MAX_SIZE);
				
			memcpy(log,trylog,strlen(trylog));
		}	
		
	}
	/* Controllo su msg per verificare la password inserita di un utente */ 
	else if(msg[0] == 'p' && msg[1] == 's' && msg[2] == 'w' && msg[3] == ' ')
	{
		char trypsswd[strlen(msg)-4];		
		int res;	
	
		//tryppswd contiene la password inserita da Client
		memset(trypsswd,0,strlen(msg)-4);
		
		for(int i = 0;i<strlen(msg)-3;i++)
		{
			trypsswd[i] = msg[i+4];
		}
	
		//Verifico se la password associata all'utente, che è salvato in log, è corretta
		if((res = verifica_password(log,trypsswd)) == 0)
		{
			if(write(conn,passwd_err,strlen(passwd_err)) == -1)
			{
				puts("Error writing");
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			if(write(conn,passwd_succ,strlen(passwd_succ)) == -1)
			{
				puts("Error writing");
				exit(EXIT_FAILURE);
			}
			printf("Logged %s\n",log); //Confermo il login avvenuto
			fflush(stdout);
		}	
		
	}
	/* Controllo su msg per registrare la password di un nuovo utente */ 
	else if(msg[0] == 'p' && msg[1] == 's' && msg[2] == 'w' && msg[3] == 'r' && msg[4] == ' ')
	{
		char trypsswd[strlen(msg)-5];		
		int res;	
		char temp[MAX_SIZE];

		//In trypsswd c'è la password inserita da Client da registrare su log.txt relativamente all'utente salvato nella variabile log
		memset(trypsswd,0,strlen(msg)-5);

		for(int i = 0;i<strlen(msg)-4;i++)
		{
			trypsswd[i] = msg[i+5];
		}
	
	
		//Aggiungo tryppswd come password di log
		addPasswd(log,trypsswd);


		puts("Succesfully registered"); //Se arrivo a questo punto la registrazione,>NOME_UTENTE> + <passwd>, è stata completata con successo
		fflush(stdout);
	}
	/* Inizio a ricevere il messaggio stesso, sono finite le operazioni di login e registrazione */
	/* Controllo su msg per ricevere il nome di chi ha inviato il messaggio */ 		
	else if(msg[0] == 'M' && msg[1] == 'i' && msg[2] == 't' && msg[3] == 't' && msg[4] == 'e' && msg[5] == 'n' && msg[6] == 't' && msg[7] == 'e' && msg[8] == ':')
	{
		mittente = (char *)malloc(sizeof(char)*strlen(msg)+1);
		sprintf(mittente,"%s$",msg);
		printf("%s\n",mittente);
	}
	/* Controllo su msg per ricevere il nome di chi è il destinatario del messaggio*/
	else if(msg[0] == 'I' && msg[1] == 'n' && msg[2] == 'v' && msg[3] == 'i' && msg[4] == 'o' && msg[5] == ':')
	{
		char newtry[strlen(msg)-5];;
		
			
		//In newtry ci sarà il nome del destinat<rio
		for(int i = 0;i<(strlen(msg)-5);i++)
		{
			newtry[i] = msg[i+6];
		}
			
		//Verifico se il destinatario è un utente esistente
		if(verifica_username(newtry) == 0)
		{
			if(write(conn,log_error_3,strlen(log_error_2)) == -1)
			{
				puts("Error writing");
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			if(write(conn,log_succ,strlen(log_succ)) == -1)
			{
				puts("Error writing");
				exit(EXIT_FAILURE);
			}			
			dest = (char *)malloc(sizeof(char)*strlen(newtry));
			sprintf(dest,"%s",newtry);
			printf("%s\n",dest);
		}
	}
	/* Controllo su msg per ricevere l'oggetto messaggio */ 	
	else if(msg[0] == 'O' && msg[1] == 'g' && msg[2] == 'g' && msg[3] == 'e' && msg[4] == 't' && msg[5] == 't' && msg[6] == 'o' && msg[7] == ':')
	{
		oggetto = (char *)malloc(sizeof(char)*strlen(msg)+1);
		sprintf(oggetto,"%s$",msg);
		printf("%s\n",oggetto);
	}
	/* Controllo su msg per ricevere il testo del messaggio */ 	
	else if(msg[0] == 'T' && msg[1] == 'e' && msg[2] == 's' && msg[3] == 't' && msg[4] == 'o' && msg[5] == ':')
	{
		char han[strlen(dest)+9];
		memset(han,0,strlen(dest)+9);
		
		//Nel file data/>USER_NAME>.txt ci sarà una stringa con il simbolo $ alla fine di mittente e oggetto, utile per il parsing nella lettura di un messaggio
		sprintf(han,"data/%s.txt",dest);
		int fd = open(han,O_RDWR|O_CREAT,0666);
			
		if(fd == -1)
		{
			puts("Error opening file");
			exit(EXIT_FAILURE);
		}
			
		testo = (char *)malloc(sizeof(char)*strlen(msg)+1);
		sprintf(testo,"%s\n",msg);
			
			
		lseek(fd,0,SEEK_END);
			
		if(write(fd,mittente,strlen(mittente)) == -1)
		{
				puts("Error writing");
				exit(EXIT_FAILURE);				
		}
		if(write(fd,oggetto,strlen(oggetto)) == -1)
		{
				puts("Error writing");
				exit(EXIT_FAILURE);				
		}
		if(write(fd,testo,strlen(testo)) == -1)
		{
				puts("Error writing");
				exit(EXIT_FAILURE);				
		}			
	
		close(fd);
	}	
	/* Controllo su msg per leggere i messaggi ricevuti da un utente */ 	
	else if(msg[0] == 's' && msg[1] == 'h' && msg[2] == 'o' && msg[3] == 'w' && msg[4] == ' ')
	{
		char name[strlen(msg)+4];
		int fd;
		int flag = 0;
		int i;
		char c[1];
		char kj[MAX_SIZE];
		memset(name,0,strlen(msg)+4);
		
		strcat(name,"data/");
		
		for(i = 5;i<strlen(msg);i++)
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
			
		//Leggo finché non incotro per la seconda volta il simbolo $, infatti devo mostrare al client tutto meno 
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
				if(c[0] == '\n')
				{
					printf("%s\n",kj);
					fflush(stdout);
					
					if(write(conn,kj,strlen(kj)) == -1)
					{
						puts("Error writing");
						exit(EXIT_FAILURE);				
					}	
					
						
					flag = 0;
					memset(kj,0,MAX_SIZE);
					i = 0;
				}
			}
			goto fil;			
		}
		
						
				
				
				
				
				/* //Use this if you want only the object of a message
				if(flag == 0)
				{
					if(c[0] == '$')
					{
						flag = 1;
					}
				}
				else if(flag == 1)
				{
					if(c[0] == ':')
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
						printf("%s\n",kj);
						fflush(stdout);
						
						if(write(conn,kj,strlen(kj)) == -1)
						{
							puts("Error writing");
							exit(EXIT_FAILURE);				
						}	
						
						
						flag = 0;
						memset(kj,0,MAX_SIZE);
						i = 0;
					}
				}
				goto fil;*/
	
			close(fd);
			
		}	
	/* Controllo su msg per ottenere il testo del messaggio che il client vuole leggere */ 	
	else if(msg[0] == 't' && msg[1] == 'e' && msg[2] == 'x' && msg[3] == ' ')
	{
		char nm[strlen(msg)-4];
		int i = 0;
		int fd;
		char ris[MAX_SIZE];
		char c[1];
		int flag = 0;
		int end = 0;
		int n;
		
		
		for(i=0;i<strlen(msg)-3;i++)
		{
			nm[i] = msg[i+4];
		}
		n = strtol(nm,0,10);
		printf("Trying to access to of %s,%s\n",last_file,nm);
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
	else if(msg[0] == 'l' && msg[1] == 'o' && msg[2] == 'g' && msg[3] == 'o' && msg[4] == 'f' && msg[5] == 'f' && msg[6] == ' ')
	{
		char m[strlen(msg)-7];
		
		for(int i = 0;i<strlen(msg)-6;i++)
		{
			m[i] = msg[i+7];
		}
		
		printf("%s logged off\n",m);
		fflush(stdout);
	}
	/* Controllo su msg per ricevere il numero del messaggio da cancellare */ 		
	else if(msg[0] == 'd' && msg[1] == 'e' && msg[2] == 'l' && msg[3] == ' ')
	{
		char nm[strlen(msg)-4],c[1];
		int i;
		int n,fd,fd2,flg;
		char *temp;
		
		memset(nm,0,strlen(msg)-4);
		
		for(i=0;i<strlen(msg)-3;i++)
		{
			nm[i] = msg[i+4];
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
					puts("Error writing");
					exit(EXIT_FAILURE);
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
		if(write(conn,"Succesfully Delated",18) == -1)
		{
			puts("Error writing");
			exit(EXIT_FAILURE);				
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


int main(int argc,char **argv)
{
	int port;
	int sck,lst;
	struct sockaddr_in servAddr,clAddr;
	int sin_size;
	int fd;
	long int conn;
	pthread_t thr;
	int ret;

	//Controllo per vedere se il programma è stato lanciato con un numero corretto di argomenti
	if(argc != 2)
	{
		puts("Error: <Program Port_Num>");
		exit(EXIT_FAILURE);
	}

	port = strtol(argv[1],0,10);
	printf("Server listening on port %d\n",port);
	
	
	
	//Apro o creo il file che dovrà gestire il login, in particolare se il file è appena stato creato aggiungo subito un utente root di passsword 0 
	fd = open(path,O_RDWR|O_CREAT,0666);
	
	if(fd == -1)
	{
		printf("Error opening file,errno returned %d,%s\n",errno,strerror(errno));
		exit(EXIT_FAILURE);
	}

	char *l = "root:0";
	char rea[1];
	
	if(read(fd,rea,1) == -1)
	{
		puts("Error reading");
		exit(EXIT_FAILURE);
	}	
	
	if(rea[0] == 0)
	{
		if(write(fd,l,strlen(l)) == -1)
		{
			puts("Error writing");
			exit(EXIT_FAILURE);
		}
	}
	
	lseek(fd,0,SEEK_SET);

	close(fd);
	

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
		 		

		/*Connect */
		conn = accept(sck,(struct sockaddr *)&clAddr, (socklen_t *)&sin_size);
		
		if(conn < 0)
		{
			printf("%s: Erroraccepting message,errno returned %d,%s\n",argv[0],errno,strerror(errno));
			exit(EXIT_FAILURE);
		}



		if(pthread_create(&thr,NULL,(void *)thread_fun,(void *)conn) == -1)
		{
			puts("Error creating thread");
			exit(EXIT_FAILURE);
		}

		puts("Thread creato");

	
	}
	close(sck);
	return 0;
}