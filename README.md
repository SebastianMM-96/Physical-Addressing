# Direccionamiento físico

En la red de computadoras, cada interface o hardware de red, posee una dirección única, que permite
identificar a los equipos en la red. Cada dirección se compone de 6 bytes (48 bits en hexadecimal), se
recomienda investigar sobre las direcciones físicas o MAC.
La transmisión en este caso, se va a lograr empleado únicamente dos capas: enlace de datos y física. La capa
física proporciona el hardware necesario para comunicación, mientras que la capa de enlace de datos
proporciona, en este caso, de un protocolo de comunicación entre dos computadoras. No nos vamos a
preocupar por la capa física, pero si de la capa de enlace de datos, específicamente vamos a enviar un paquete
Ethernet.

Ethernet define un conjunto de estándares a nivel de capa física y de enlace de datos. A reserva de que realice
una mayor investigación (ver el estándar IEEE1 802.3), Ethernet resuelve el problema de identificación en la red
y de empaquetamiento. La información viaja en bloques de n bytes de tamaño determinado, en donde cada
bloque, posee un encabezado y una cola; después, este paquete se coloca en un “tren” de bytes, que en
conjunto se denomina trama.

***

Actividades a desarrollar:

1. Diseñe un programa que permita comunicar dos computadoras, mediante el envío de mensajes cortos. Los
usuarios en las máquinas, podrán capturar y enviar mensajes, que serán recibidos y mostrados en la pantalla la
computadora destino. Este programa es semejante a un chat entre dos usuarios.

2. Considere el programa anterior, pero ahora involucrando a 3 computadoras. Si un usuario captura y envía un
mensaje, este podrá ser recibido y mostrado por dos computadoras destino.

3. Considere un par de programas de n hilos, en donde cada hilo posee un identificador dado por un número
entero entre 1 y n. Cada programa, estará ejecutándose en maquinas diferentes y podrán comunicarse
mediante el envío de paquetes Ethernet. Cada hilo i, a través del proceso padre que los creo, se comunicará
con su hilo par i en otra máquina, de esta manera los hilos podrán enviarse información en ambos sentidos.
Usted decir qué tipo de información se debe enviar, pero deberá ser en ambos sentidos y está deberá pasar
por el padre que crea los hilos. Observe que el proceso padre origen, deberá saber la dirección destino; en
cambio, el proceso padre destino, debe además saber el hilo destino al cual entregarle la información.
