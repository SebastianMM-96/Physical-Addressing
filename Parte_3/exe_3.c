#include "eth.h"
#include "pthread.h"

char *datE[2], **msjsS, **msjsR;
int *fR, *fS, N; //N es el número de hilos

void hilo(void *arg);
void seth(void *arg);
void reth(void *arg);

int main(int argc, char *argv[]){

	int i;

	if (argc!=3){
		printf ("Error en argumentos.\n\n");
		printf ("seth INTERFACE-ETRADA/SALIDA MAC-DESTINO\n\n");
		exit (1);
	}

	datE[0] = argv[1];
	datE[1] = argv[2];

	//Recibimos el número de hilos que se crearan
	do{
		printf("\nIngresa el número de hilos: ");
		scanf("%d",&N);
		if(N<1) printf("El número deber ser mayor a 0.\n");
	}while(N<1);
	printf("\n");

	sleep(5);

	//Aquí se guardaran los mensajes que reciban los hilos.
	msjsR = (char **)malloc(50*sizeof(char*)); 
	for (i=0;i<50;i++) 
		msjsR[i] = (char*)malloc(N*sizeof(char));

	//Aquí se guardaran los mensajes que envien los hilos.
	msjsS = (char **)malloc(50*sizeof(char*)); 
	for (i=0;i<50;i++) 
		msjsS[i] = (char*)malloc(N*sizeof(char));

	//Banderas de recibido y enviado, comienzan en FALSE todas.
	fR = (int*)malloc(N*sizeof(int));
	fS = (int*)malloc(N*sizeof(int));

	for (i=0;i<N;i++){
		fR[i] = false;
		fS[i] = false;
	} 

	pthread_t *hilos = (pthread_t*)malloc(N*sizeof(pthread_t));
	pthread_t enviar;
	pthread_t recibir;

	int *iId = (int*)malloc(N*sizeof(int)); //Un identificador para cada hilo.
  
	for (i=0; i<N; i++){
		iId[i] = i+1; //Van del 1 a N.
		if ((pthread_create(&hilos[i],NULL,(void*)&hilo,(void*)&iId[i]))!=0){
			printf("Error en la creación de hilos\n");
			exit(0);
		}
	}

	if ((pthread_create(&enviar,NULL,(void*)&seth,NULL))!=0){
		printf("Error en la creación de hilo enviar\n");
		exit(0);
	}

	if ((pthread_create(&recibir,NULL,(void*)&reth,NULL))!=0){
		printf("Error en la creación de hilo recibir\n");
		exit(0);
	}

	for (i=0; i<N; i++)
    	pthread_join (hilos[i], NULL);

    pthread_join (enviar, NULL);
	pthread_join (recibir, NULL);

	free(hilos);
	free(iId);
	return 0;

}

void hilo(void *arg){

	int iId;
	iId = *(int *)arg;

	//Este es el mensaje que se enviará.
	char msj[40]; 
	sprintf(msj,"%s %d","Hola hilo",iId);

	do{
	
		//Si no se ah enviado un mensaje lo enviamos.
		if(!fS[iId-1]){
			sprintf(msjsS[iId-1],"%d %s",iId-1,msj); //Ponemos msj en el bufer.
			fS[iId-1] = true; //Confirmamos que ya enviamos mensaje.
		}

		//Si llegó un mensaje lo leemos.
		if(fR[iId-1]){
			printf("Soy el hilo %d me llegó el mensaje: %s\n",iId,msjsR[iId-1]);
			memset(msjsR[iId-1],'\0',50);//Limpiamos el bufer.
			fR[iId-1] = false; //Confirmamos que ya leimos el mensaje.
		}

	}while(true);

	pthread_exit (NULL); //Termina el hilo.

}

void seth (void *arg){

	int sockfd;
	struct ifreq if_idx;
	struct ifreq if_mac;
	int tx_len = 0, i, j, iLen;
	char sendbuf[BUF_SIZ], Mac[6];
	struct ether_header *eh = (struct ether_header *) sendbuf;
	struct iphdr *iph = (struct iphdr *) (sendbuf + sizeof(struct ether_header));
	struct sockaddr_ll socket_address;
	char ifName[IFNAMSIZ];

	/*Coloca la interface*/
	strcpy(ifName, datE[0]);

	/*Abre el socket*/
	if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
		perror("socket");
	}

	/* Mediante el nombre de la interface, se obtiene su indice */
	memset (&if_idx, 0, sizeof(struct ifreq));  /*Llena de 0*/
	strncpy (if_idx.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0) perror("SIOCGIFINDEX");
	/*Ahora obtenemos la MAC de la interface por donde saldran los datos */
	memset (&if_mac, 0, sizeof(struct ifreq));
	strncpy (if_mac.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0) perror("SIOCGIFHWADDR");

	/* Ahora se construye el encabezado Ethernet */
	memset(sendbuf, 0, BUF_SIZ);
	/*Dirección Origen*/
	for(i=0;i<6;i++)
		eh->ether_shost[i] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[i];
	/*Dirección destino*/
	ConvierteMAC (Mac, datE[1]);
	for(i=0;i<6;i++)
		eh->ether_dhost[i] = Mac[i];

	socket_address.sll_ifindex = if_idx.ifr_ifindex;
	socket_address.sll_halen = ETH_ALEN;
	for(i=0;i<6;i++)
		socket_address.sll_addr[i] = Mac[i];

	/*Ahora construimos los paquetes, vamos a enviar mensajes cortos*/
	eh->ether_type = MI_PAQUETE;
	while (true){
		
		for(j=0;j<N;j++){
			if(fS[j]){ //Si hay algún mensaje.
				iLen = strlen (msjsS[j]);
				tx_len = sizeof(struct ether_header);
				for (i=0; i<iLen; i++) sendbuf[tx_len++] = msjsS[j][i];
				/*Rellenamos por ceros nuestro paquete, asociado a su tamaño*/
				for (i=iLen; i<(MI_PAQUETE-iLen); i++) sendbuf[tx_len++] = 0;
				/*Se manda el paquete*/
				if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
					printf("Send failed\n");
				memset(msjsS[j],'\0',50); //Limpiamos el bufer.
				fS[j] = false; //Confirmamos que ya enviamos.
				usleep(1000);
			}
		}
	}
	close (sockfd);	

	pthread_exit (NULL); //Termina el hilo.
}

void reth (void *arg){

	char sender[INET6_ADDRSTRLEN];
	int sockfd, ret, i, tx_len;
	int sockopt;
	ssize_t numbytes;
	struct ifreq ifopts; 
	struct ifreq if_ip; 
	struct sockaddr_storage their_addr;
	uint8_t buf[BUF_SIZ];
	char ifName[IFNAMSIZ];
	char Imprime;

	/*Coloca la interface*/
	strcpy(ifName, datE[0]);  

	/* Los encabezados */
	struct ether_header *eh = (struct ether_header *) buf;

	memset(&if_ip, 0, sizeof(struct ifreq));

	/* Se abre el socket para "escuchar" los ETHER_TYPE */
	/*PF_PACKET captura en principio cualquier paquete*/
	if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
		perror("listener: socket");
	}

	/* Modo Promiscuo*/
	strncpy(ifopts.ifr_name, ifName, IFNAMSIZ-1);
	ioctl(sockfd, SIOCGIFFLAGS, &ifopts);
	ifopts.ifr_flags |= IFF_PROMISC;
	ioctl(sockfd, SIOCSIFFLAGS, &ifopts);

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
		perror("setsockopt");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1)  {
		perror("SO_BINDTODEVICE");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	//Obtenemos nuestra dirección MAC
	struct ifreq if_mac;

	memset (&if_mac, 0, sizeof(struct ifreq));
	strncpy (if_mac.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0) perror("SIOCGIFHWADDR");

	uint8_t myHost[6];

	for (i=0; i<6; i++)
		myHost[i] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[i];

	printf("Escuchando de red...");
	//Se utiliza una bandera para saber si nuestra dirección MAC corresponde a la MAC destino.
	int flag = 0; //Por lo mientras la inicializamos en true.
	char mensaje[50], msj[40]; //Para recibir el mensaje.
	int id,lengM;

	do{  
		numbytes = recvfrom(sockfd, buf, BUF_SIZ, 0, NULL, NULL);
		/*Identificamos nuestro paquete por el numero de paquete
		  ojo: este programa captura cualquier paquete, por lo que deberan
		  asignar un numero a cada paquete de ustedes. Solo tengan cuidado
		  no se vale cualquier numero.*/

		if(eh->ether_type==MI_PAQUETE){ //Identificamos el paquete por el tamaño.
			for (i=0; i<6; i++){
				if(myHost[i] != eh->ether_dhost[i]){ //Si se encuentra que en alguna parte, la dirección no es la esta.
					flag = false; //Sabremos que este paquete no es para nosotros y por lo tanto la bandera sera false.
					i = 5; //Ya que se alló en alguna parte que la dirección MAC es diferente, finalizamos el ciclo.
				}
			}
		}

		//Identificamos el paquete por su tamaño y comprobamos que sea para esta dirección MAC.
		if (eh->ether_type==MI_PAQUETE&&flag){
			tx_len = sizeof(struct ether_header);
			for (i=0; buf[tx_len]; i++) 
				mensaje[i] = buf[tx_len++];
			sscanf(mensaje,"%d",&id);
			lengM =strlen(mensaje);
			if(id<10){
				for(i=2;i<lengM;i++) msj[i-2] = mensaje[i];
			}else{
				for(i=3;i<lengM;i++) msj[i-3] = mensaje[i];
			}
			if(id<N){ //Comprobamos que tengamos el hilo.
				if(!fR[id]){ //Si no ha leido el ultimo mensaje, ignoramos.
					strcpy(msjsR[id],msj);
					fR[id] = true; //Confirmamos que llego un mensaje.
				}
			}
			memset(mensaje,'\0',50); //Vaciamos la cadena para proximos mensajes.
			memset(msj,'\0',40);
		}
		if (!flag) flag = true; //Si la bandera esta en false la cambiamos a true, para la proxima comparación.
	} 
	while (true);

	close (sockfd);

	pthread_exit (NULL); //Termina el hilo.
}
