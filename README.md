# graphic_shell
C/GTK Graphic Shell
/*
* Intruções:
* Este aquivo pode ser compilado utilizando GNU Compiler Collection no sistema operacional Linux.
* 1- Inicial certifique-se de colocar o arquivo no diretório escolhido
* 2- Abra o terminal pelo comando Crtl + Alt T
* 3- Utilize o comando "cd Diretório" para navegar ao diretório especificado
* 4- Verifique o arquivo no diretório utilizando o comando "ls" (se necessário)
* 5- Digite o comando: gcc -Wno-format -o windoors main.c -pthread -Wno-deprecated-declarations
* 					-Wno-format-security -lm `pkg-config --cflags --libs gtk+-3.0` -export-dynamic
* 6- O arquivo executável foi gerado; Utilize o comando (./windoors) para executá-lo
* 7- Lembre-se de se certificar que os elementos da GUI estão desativados,
*	 caso necessário instale o gnome-tweaks e a extensão hide-top-bar para que o shell
*	 possa ocupar toda a tela.
* 8- Utilize o programa
*
* 
*
* @file main.c
* @brief Programa shell gráfico para linux, utilizando o programa Glade + Gtk e linguagem c.
* 
*
* @authors Paulo Diego S. Souza, Lucas R. Costa
* @date 16/10/21
*         
*/