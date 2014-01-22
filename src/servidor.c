#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TAMFILA		6
#define MAXNOMEHOST	30


void ExecutaComando(char *dados, char *pid){
	char cmd[BUFSIZ];

	snprintf(pid, 10,"%d",getpid()); 	// Pega o Process ID do servidor e transforma em string.
	strcat(pid,".tmp");      		 // Guarda o conteudo em um arquivo temporario cujo nome
						 // e' o Process ID do servidor.

	if(!strcmp(dados,"rm tmp")){		// Remove o arquivo temporario.
		strcpy(cmd, "rm \0");	
		strcat(cmd,pid);		// Teremos algo no formato "rm PID.tmp".
	} else if(dados[0]==1){
		dados[0]=' ';
		strcpy(cmd, "man\0");	
		strcat(cmd,dados);
		strcat(cmd," > ");
		strcat(cmd,pid);   		// Teremos algo no formato "man COMANDO > PID.tmp".
	} else if(dados[0]==2){
		dados[0]=' ';
		strcpy(cmd, "dict\0");	
		strcat(cmd,dados);
		strcat(cmd," > ");
		strcat(cmd,pid);   		// Teremos algo no formato "dict PALAVRA > PID.tmp".
	} else exit(1);	 
	system(cmd);			 
}

main (int argc, char *argv[]) {
	int sock_escuta, sock_atende, lido=0;
	unsigned int aux;
	char dados[BUFSIZ+ 1], pid[20];
	struct sockaddr_in EnderecLocal, EnderecClient;
	struct hostent *RegistroDNS;
	char NomeHost[MAXNOMEHOST];
	FILE *fp;

	gethostname (NomeHost, MAXNOMEHOST);

	if ((RegistroDNS = gethostbyname(NomeHost)) == NULL){
		puts ("(Servidor) Nao consegui meu proprio IP");
		exit (1);
	}	
	
	EnderecLocal.sin_port = htons(atoi(argv[1]));
	EnderecLocal.sin_family = AF_INET;		
	bcopy ((char *) RegistroDNS->h_addr, (char *) &EnderecLocal.sin_addr, RegistroDNS->h_length);

	if ((sock_escuta = socket(AF_INET,SOCK_STREAM,0)) < 0){
		puts ("(Servidor) Nao consegui abrir o socket");
		exit ( 1 );
	}	
	
	if (bind(sock_escuta, (struct sockaddr *) &EnderecLocal, sizeof(EnderecLocal)) < 0){
		puts ("(Servidor) Nao consegui fazer o bind");
		exit ( 1 );
	}		
	 
	listen (sock_escuta, TAMFILA);
	
	while (1){
		aux = sizeof(EnderecLocal);
		if ((sock_atende=accept(sock_escuta, (struct sockaddr *) &EnderecClient, &aux))<0){
			puts ("(Servidor) Nao consegui fazer conexao com cliente");
			exit (1);
		}	
		read(sock_atende, dados, BUFSIZ);

		ExecutaComando(dados, pid);

		/* Le o conteudo do arquivo temporario e envia ao cliente. */
		fp = fopen(pid,"rb");

		if(fp==NULL) {
			puts("(Servidor) Impossivel abrir arquivo temporario");
		}

		while(!feof(fp)){
			for(lido=0; lido < BUFSIZ; lido++){

				fread(dados+lido,1,1,fp);
				if(feof(fp)) {
					dados[lido]=0;
					break;
				}
			}
			write(sock_atende, dados, BUFSIZ);
		}
		fclose(fp);

		ExecutaComando("rm tmp", pid);


		for(aux = 0; aux <= BUFSIZ; aux++) 
			dados[aux] = '\0';
		

		close(sock_atende);
	}
}

