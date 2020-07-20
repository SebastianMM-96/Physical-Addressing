# Parte 3

Considere un par de programas de n hilos, en donde cada hilo posee un identificador dado por un número
entero entre 1 y n. Cada programa, estará ejecutándose en maquinas diferentes y podrán comunicarse
mediante el envío de paquetes Ethernet. Cada hilo i, a través del proceso padre que los creo, se comunicará
con su hilo par i en otra máquina, de esta manera los hilos podrán enviarse información en ambos sentidos.
Usted decir qué tipo de información se debe enviar, pero deberá ser en ambos sentidos y está deberá pasar
por el padre que crea los hilos. Observe que el proceso padre origen, deberá saber la dirección destino; en
cambio, el proceso padre destino, debe además saber el hilo destino al cual entregarle la información.
