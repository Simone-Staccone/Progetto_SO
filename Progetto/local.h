#define MAX_SIZE 1024
char *log_error = "Username not found";
char *log_error_2 = "Usr already exist";
char *log_error_3 = "User is not registered";
char *log_succ = "Username found";
char *log_succ_3 = "Username disponibile";
char *passwd_err = "Wrong password";
char *passwd_succ = "Password correct";

char *sys_log = "log ";
char *sys_reg = "log r=";
char *sys_psw = "psw ";
char *sys_pswr = "pswr ";
char *sys_mittente = "Mittente:";
char *sys_invio = "Invio:";
char *sys_oggetto = "Oggetto:";
char *sys_testo = "Testo:";
char *sys_show = "show ";
char *sys_tex = "tex ";
char *sys_logoff = "logoff ";
char *sys_del = "del ";
char *acc_file = "data/";
char *suffisso = ".txt";
char *bn = "\n";
char *del_succ = "Succesfully delated";

#define err {printf("Errno returned %d, %s\n",errno,strerror(errno)); exit(-1);}


struct ut{
	char name[MAX_SIZE];
	char passwd[MAX_SIZE];
	char *buff;
};

struct messaggio{
	char mittente[MAX_SIZE];
	char oggetto[MAX_SIZE];
	char *testo;
};