#include "eth.h"
#include "pthread.h"

char *datE[3];

void seth(void *arg);
void reth(void *arg);

int main(int argc, char *argv[])
{

	if (argc!=4)
	{
	printf ("Error en argumentos.\n\n");
	printf ("seth INTERFACE-ETRADA/SALIDA MAC-DESTINO1 MAC-DESTINO2\n\n");
	exit (1);
	}

	pthread_t enviar;
	pthread_t recibir;
	datE[0] = argv[1];
	datE[1] = argv[2];
	datE[2] = argv[3];

	if ((pthread_create(&enviar,NULL,(void*)&seth,NULL))!=0){
		printf("Error en la creación de hilo enviar\n");
		exit(0);
	}

	if ((pthread_create(&recibir,NULL,(void*)&reth,NULL))!=0){
		printf("Error en la creación de hilo recibir\n");
		exit(0);
	}

	pthread_join (enviar, NULL);
	pthread_join (recibir, NULL);

}

void seth (void *arg){

	int sockfd;
	struct ifreq if_idx;
	struct ifreq if_mac;
	int tx_len = 0, i, j, iLen;
	char sendbuf[BUF_SIZ], Mac[2][6];
	char mensaje[100];
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

	ConvierteMAC (Mac[0], datE[1]);
	ConvierteMAC (Mac[1], datE[2]);

	socket_address.sll_ifindex = if_idx.ifr_ifindex;
			socket_address.sll_halen = ETH_ALEN;

	/*Ahora construimos los paquetes, vamos a enviar mensajes cortos*/
	eh->ether_type = MI_PAQUETE;
	int salida = true; //Usamos una bandera para cuando salgamos del chat.
	mensaje[0] = ' '; //Util para corregir el error del scanf siguiente.
	while (salida){
		gets(mensaje); //Sin error aunque peligrosa si se ingresa una cadena más grande del tamaño de la cadena.
		printf ("Enviando...\n");
		iLen = strlen (mensaje);
		tx_len = sizeof(struct ether_header);
		for (i=0; i<iLen; i++) sendbuf[tx_len++] = mensaje[i];
		/*Rellenamos por ceros nuestro paquete, asociado a su tamaño*/
		for (i=iLen; i<(MI_PAQUETE-iLen); i++) sendbuf[tx_len++] = 0;
		/*Se manda el paquete*/
		for(i=0;i<2;i++){ //Para enviar a las dos MAC destino el mensaje.
			/*Dirección destino*/
			for(j=0;j<6;j++)
				eh->ether_dhost[j] = Mac[i][j];

			for(j=0;j<6;j++)
				socket_address.sll_addr[j] = Mac[i][j];

			if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
				printf("Send failed\n");
		}
		if(strcmp(mensaje,"exit")==0) salida = false; //Para salir del chat.
		usleep (1000);
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
	printf("Conectado en el chat...\n");

	//Obtenemos nuestra dirección MAC
	struct ifreq if_mac;

	memset (&if_mac, 0, sizeof(struct ifreq));
	strncpy (if_mac.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0) perror("SIOCGIFHWADDR");

	uint8_t myHost[6];
	for(i=0;i<6;i++)
		myHost[i] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[i];

	//Se utiliza una bandera para saber si nuestra dirección MAC corresponde a la MAC destino.
	int flag = 0; //Por lo mientras la inicializamos en true.
	char mensaje [100]; //Para recibir el mensaje.
	int salida = true; //Para salir del ciclo.

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
			for (i=0; i<6; i++){
				if(i==5){ //Se anota la dirección MAC de donde se recibió.
					printf ("%02X", eh->ether_shost[i]);
				}else{
					printf ("%02X:", eh->ether_shost[i]);
				}
			}
			tx_len = sizeof(struct ether_header);
			for (i=0; buf[tx_len]; i++) 
				mensaje[i] = buf[tx_len++];
			printf(" dice: %s\n",mensaje);
			if(strcmp(mensaje,"exit")==0) salida = false; //Para salir del ciclo.
			memset(mensaje,'\0',100); //Vaciamos la cadena para proximos mensajes.
		}
		if (!flag) flag = true; //Si la bandera esta en false la cambiamos a true, para la proxima comparación.
	} 
	while (salida);

	close (sockfd);

	pthread_exit (NULL); //Termina el hilo.
}
