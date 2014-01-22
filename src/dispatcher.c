#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TAMFILA		5
#define MAXNOMEHOST	30


/* Busca portas vazias e instancia os servidores. */
char **InstanciaServidores(int nServidores, char **listaServidores){
	int p, i, tam, aux;
	char lsof[50] = "lsof -i :\0";
	char serv[50] = "./servidor \0";
	char porta[10];
	FILE *fp;
	srand(time(NULL));

	listaServidores = malloc((1 + nServidores) * sizeof(char*));
	for(i = 1; i <= nServidores; i++) {
		listaServidores[i] = malloc(10);
		for(aux = 0; aux < 10; aux++)
			listaServidores[i][aux] = "\0";
	}

	for(i=1; i <= nServidores; i++){
		for(aux = 0; aux < 25; aux++){
			lsof[aux] = '\0';
			serv[aux] = '\0';
		}
		strcpy(lsof,"lsof -i :\0");
		strcpy(serv,"./servidor \0");

		printf("Instanciando servidor: %d\n",i);
		p = 2001 + rand() % 4001;		// Pega uma porta aleatoria entre 2001 e 6001.
		snprintf(porta, 10,"%d",p);

		strcat(lsof,porta);  			// Concatena o comando com a porta escolhida.
		strcat(lsof," > tmp/dispatcher.tmp");	// Redireciona a saida do comando lsof para um arquivo temporario.
		printf("%s\n",lsof);
		system(lsof);                  		// Executa o comando pra checar se a porta esta' sendo usada.


		fp = fopen("tmp/dispatcher.tmp","rb");	// Abre o arquivo temporario.
		fseek(fp, 0L, SEEK_END);		// Checa o tamanho do arquivo.
		tam = ftell(fp);
		if (tam == 0) {				// Se o arquivo estiver vazio, a porta nao esta' ocupada.
			strcat(serv,porta);
			strcat(serv," &");
			printf("%s\n",serv);
			system(serv);           	  // Instancia um servidor.
			strcpy(listaServidores[i],porta); // Relaciona a porta ao respectivo servidor .
		} else
			 i--;				// Caso contrario, tenta achar uma porta vazia novamente.
		fclose(fp);
		system("rm tmp/dispatcher.tmp");
	}
	printf("Fim de instanciacao de servidores.\n");
 	for(aux=1; aux <= nServidores; aux++)
		printf("Servidor:   %d :%s\n",aux,listaServidores[aux]);

	printf("------------\n");
	return listaServidores;
}

/* Retorna a porta de um servidor. */
char *selecionaServidor (char *tipo, int *ultimoServidor, char **listaServidores, int nServidores){
	int i, aux;

	if (!strcmp(tipo,"roundrobin")){
		i = 1 + (*ultimoServidor) % nServidores;
		(*ultimoServidor)++;
		printf("Servidor alocado ao cliente: %d\n\n",i);
		return listaServidores[i];
	} else 
		if (!strcmp(tipo,"random")){
			i = 1 + rand() % nServidores;
			printf("Servidor alocado ao cliente: %d\n\n",i);
			return listaServidores[i];
	} else
		printf ("Modo invalido.\n");

}


main (int argc, char *argv[]) {
	 int sock_escuta, sock_atende;
	 int nServidores,  ultimoServidor = 0;
	 unsigned int aux;
	 char buffer[BUFSIZ + 1], *listaServidores, *porta;
	 struct sockaddr_in EnderecLocal, EnderecClient;
	 struct hostent *RegistroDNS;
	 char NomeHost[MAXNOMEHOST];

	 if (argc != 4) {
	  puts("Uso correto: ./dispatcher <Porta> <Numero de Servidores> <Modo>\n");
	  exit(1);
	 }

	gethostname (NomeHost, MAXNOMEHOST);

	if ((RegistroDNS = gethostbyname(NomeHost)) == NULL){
		puts ("(Dispatcher) Nao consegui meu proprio IP.\n");
		exit (1);
	}	
	
	EnderecLocal.sin_port = htons(atoi(argv[1]));
	EnderecLocal.sin_family = AF_INET;		
	bcopy ((char *) RegistroDNS->h_addr, (char *) &EnderecLocal.sin_addr, RegistroDNS->h_length);

	if ((sock_escuta = socket(AF_INET,SOCK_STREAM,0)) < 0){
		puts ("(Dispatcher) Nao consegui abrir o socket.\n");
		exit (1);
	}	
	
	if (bind(sock_escuta, (struct sockaddr *) &EnderecLocal, sizeof(EnderecLocal)) < 0){
		puts ("(Dispatcher) Nao consegui fazer o bind.\n");
		exit (1);
	}		
 
	nServidores = atoi(argv[2]);
	listaServidores = InstanciaServidores(nServidores,listaServidores);
	porta = malloc(5);

	listen (sock_escuta, TAMFILA);

	while (1){
		aux = sizeof(EnderecLocal);

		if ((sock_atende=accept(sock_escuta, (struct sockaddr *) &EnderecClient, &aux))<0){
			puts ("(Dispatcher) Nao consegui fazer conexao com cliente.\n");
			exit (1);
		}
	
		read(sock_atende, buffer, BUFSIZ);
		printf("(Dispatcher) requisicao recebida do cliente %s\n",buffer);
		porta = selecionaServidor(argv[3], &ultimoServidor, listaServidores, nServidores);

		write(sock_atende, porta, BUFSIZ);
		for(aux = 0; aux <= BUFSIZ; aux++)
			buffer[aux] = '\0';
		close(sock_atende);
	}
}

