#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>

#define BUF_SIZ      1024    /*Tamaño del buffer de datos a enviar*/
#define MI_PAQUETE   513     /*Vamos a enviar paquetes de 513 bytes*/
#define true 1
#define false 0


 void ConvierteMAC (char *Mac, char *Org)
 {
   int i, j, Aux, Acu;
   for (i=0, j=0, Acu=0; i<12; i++)
   {
     if ((Org[i]>47)&&(Org[i]<58)) Aux = Org[i] - 48;
     if ((Org[i]>64)&&(Org[i]<97)) Aux = Org[i] - 55;
     if (Org[i]>96) Aux = Org[i] - 87;
     if ((i%2)==0) Acu = Aux * 16;
     else 
     {
       Mac[j] = Acu + Aux;  j++;
     }
   }
 }
