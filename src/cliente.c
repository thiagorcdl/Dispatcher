#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//char buffer[10000024];

main(int argc, char *argv[]) {
	int sock_descr, sock_serv;
	int NumBytesRecebidos, lido=0;
	struct sockaddr_in EnderecRemoto;
	struct hostent *RegistroDNS;
	char portaServ[10], pid[20];
	char *NomeHost, touch[20] = "touch log/", rm[20] = "rm log/";
	char *dados, *buffer;

	if(argc != 5) {
		puts("Uso correto: ./cliente <HostServidor> <Porta> <Comando> <Palavra>");
		exit(1);
	}
	NomeHost = argv[1];

	buffer = malloc(1);
	dados = malloc(51);
	strcpy(dados+1,argv[4]);

	if(!strcmp(argv[3],"man"))
		dados[0]=1;
	else if(!strcmp(argv[3],"dict"))
		dados[0]=2;
	else
		puts("Comando invalido. Utilize \"man\" ou \"dict\"");

	if((RegistroDNS = gethostbyname(NomeHost)) == NULL){
		puts("(Cliente) Nao consegui obter endereco IP do servidor.");
		exit(1);
	}

	bcopy((char *)RegistroDNS->h_addr, (char *)&EnderecRemoto.sin_addr, 
		 RegistroDNS->h_length);
	EnderecRemoto.sin_family = AF_INET;
	EnderecRemoto.sin_port = htons(atoi(argv[2]));

	if((sock_descr=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		puts("(Cliente) Nao consegui abrir o socket.\n");
		exit(1);
	}

	if(connect(sock_descr, (struct sockaddr *) &EnderecRemoto, sizeof(EnderecRemoto)) < 0) {
		puts("(Cliente) Nao consegui conectar ao dispatcher.\n");
		exit(1);
	} 


	snprintf(pid, 10,"%d",getpid()); 

	if(write(sock_descr, pid, sizeof(pid)) !=  sizeof(pid)){
		puts("(Cliente) Nao foi possivel fazer requisicao ao dispatcher.\n"); 
		exit(1);
	}

	read(sock_descr, portaServ, BUFSIZ);			//Le a porta do servidor escolhido pelo dispatcher
	printf("Porta do Servidor: %s\n", portaServ);


	EnderecRemoto.sin_port = htons(atoi(portaServ));	//Troca a porta do dispatcher pela do servidor

	if((sock_serv=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		puts("(Cliente) Nao consegui abrir o socket.\n");
		exit(1);
	}

	if(connect(sock_serv, (struct sockaddr *) &EnderecRemoto, sizeof(EnderecRemoto)) < 0) {
		puts("(Cliente) Nao consegui conectar ao servidor.\n");
		exit(1);
	} 

	if(write(sock_serv, dados, strlen(dados)) != strlen(dados)){
		puts("(Cliente) Nao foi possivel enviar dados para o servidor.\n"); 
		exit(1);
	}

	do {
		buffer = realloc(buffer, 1 + lido);
		recv(sock_serv, buffer + lido, 1, 0);
	} while(buffer[lido++]);
	printf("%s\n",buffer);

	free(buffer);
	free(dados);
	close(sock_serv);
	close(sock_descr);

	exit(0);
}

