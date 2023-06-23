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
* @authors Paulo Diego S. Souza, Lucas R. Costa, Gustavo Borges, Celso Emiliano, Luis Felipe Silveira
* @date 16/10/21
* @bugs 
*      1- 
*
*      2- 
*         
*      3- 
*         
*/

#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>    /* POSIX Threads */
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/reboot.h>  //desligar
#include <linux/reboot.h> //desligar2
#include <glib/gprintf.h> //necessário para detectar tamanho da tela
#include <locale.h>


#define NPROCESS 8
#define MEM_SZ 4096
struct shared_area{	
  sem_t mutex;
  pid_t pids[NPROCESS]; //0 = botaoSudoku, 1 = botaoTexto, 2 = diretorios, 3 = terminal, 4 = navegador
  int pref; //preferencia do fundo
};
struct shared_area *shared_area_ptr;

//Globais!
GtkWidget	*window;
GtkWidget	*fixed1;
GtkWidget	*button1;
GtkWidget	*label1;
GtkWidget	*icone;
GtkBuilder	*builder;
GtkWidget	*barrainferior;
GtkWidget	*timer;
GtkWidget	*sobre;
GtkWidget	*sudokuicone;
GtkWidget	*sudokumenu;
//gedit
GtkWidget	*fechargedit;
GtkWidget	*gediticone;
GtkWidget	*geditmenu;
//pasta
GtkWidget	*pastaicone;
GtkWidget	*pastamenu;
//flybird
GtkWidget	*flybirdicone;
GtkWidget	*flybirdmenu;
//terminal
GtkWidget	*terminalicone;
GtkWidget	*terminalmenu;
//sobre
GtkWidget	*infosobre;
//logos
GtkWidget	*logo;
GtkWidget	*logo2;
GtkWidget	*logo3;
GtkWidget	*logo4;
//iniciar
GtkWidget	*iniciar;
//horario
gboolean timer_handler(GtkWidget *);



/** print_screen_resolution
* @brief Função responsável por verificar tamanho da tela
*        
*
* @param  int argc, char *argv
* @return 0 - outra, 1 - 1920x1080, 2 - 1366x768
*
* @bugs Nenhum conhecido
*/
int print_screen_resolution (int argc, char *argv[]) {
    GdkScreen *screen;
    gint width, height;
    gtk_init(&argc, &argv);

    if ((screen = gdk_screen_get_default()) != NULL) {
        width = gdk_screen_get_width(screen);
        height = gdk_screen_get_height(screen);
        g_printf("Resolução atual de tela: %dx%d\n", width, height);
		if(width == 1920 && height == 1080)
			return 1;
		if(width == 1366 && height == 768)
			return 2;
	}
	return 0;
}



/** main
* @brief Função principal
*      
*       
* @param int argc, char *argv[]
* @return int sucesso.
*
* @bugs Nenhum conhecido
*/
int main(int argc, char *argv[]) {

	//detecta e seleciona o arquivo adequado para cada resolução
	int resolution = 0;
	resolution = print_screen_resolution(argc, argv);
	if(resolution == 1){
		builder = gtk_builder_new_from_file ("shell(res).glade");
	}else if(resolution == 2){
		builder = gtk_builder_new_from_file ("shell(res2).glade");
	}else{
		builder = gtk_builder_new_from_file ("shell(res).glade");
	}

	//prepado para shared_memory 
	int i;
	key_t key=5678;
	void *shared_memory = (void *)0;
	int shmid;

	shmid = shmget(key,MEM_SZ,0666|IPC_CREAT);
	if ( shmid == -1 ){
		printf("shmget falhou\n");
		exit(-1);
	}//fim_if

	shared_memory = shmat(shmid,(void*)0,0);
	if (shared_memory == (void *) -1 ){
		printf("shmat falhou\n");
		exit(-1);
	}//fim_if
	//printf("Memoria compartilhada no endereco=%p\n", shared_memory);

	shared_area_ptr = (struct shared_area *) shared_memory;
	if ( sem_init((sem_t *)&shared_area_ptr->mutex,1,1) != 0 ){ //inicializando o semáfaro
		printf("sem_init falhou\n");
		exit(-1);
	}//fim_if

	sem_wait((sem_t*)&shared_area_ptr->mutex);
	for(int i = 0; i < NPROCESS; i++) //zerando os pids na área compartilhada.
		shared_area_ptr->pids[i] = 0;

	shared_area_ptr->pref = 1; //preferência inicial do fundo
	sem_post((sem_t*)&shared_area_ptr->mutex);

	gtk_init(&argc, &argv); // inicia Gtk

	//-----------------------------------------------------------------------
	// Estabelece contato com o xml para ajustar as configurações dos widgets
	//-----------------------------------------------------------------------
 
	window = GTK_WIDGET(gtk_builder_get_object(builder, "janelaprincipal"));
	gtk_window_fullscreen(GTK_WINDOW(window)); //inicia maximizado

    gtk_builder_connect_signals(builder, NULL);

	//Widgets
	//área de trabalho
	fixed1 = GTK_WIDGET(gtk_builder_get_object(builder, "fixed1"));
	button1 = GTK_WIDGET(gtk_builder_get_object(builder, "botaopasta"));
	label1 = GTK_WIDGET(gtk_builder_get_object(builder, "labelon"));
	icone = GTK_WIDGET(gtk_builder_get_object(builder, "icone"));
	barrainferior = GTK_WIDGET(gtk_builder_get_object(builder, "barrainferior"));
	timer = GTK_WIDGET(gtk_builder_get_object(builder, "timer"));

	//sudoku
	sudokuicone = GTK_WIDGET(gtk_builder_get_object(builder, "sudokuicone"));
	sudokumenu = GTK_WIDGET(gtk_builder_get_object(builder, "sudokumenu"));

	//gedit
	fechargedit = GTK_WIDGET(gtk_builder_get_object(builder, "fechargedit"));
	gediticone = GTK_WIDGET(gtk_builder_get_object(builder, "gediticone"));
	geditmenu = GTK_WIDGET(gtk_builder_get_object(builder, "geditmenu"));

	//pasta
	pastaicone = GTK_WIDGET(gtk_builder_get_object(builder, "pastaicone"));
	pastamenu = GTK_WIDGET(gtk_builder_get_object(builder, "pastamenu"));

	//navegador
	flybirdicone = GTK_WIDGET(gtk_builder_get_object(builder, "flybirdicone"));
	flybirdmenu = GTK_WIDGET(gtk_builder_get_object(builder, "flybirdmenu"));

	//terminal
	terminalicone = GTK_WIDGET(gtk_builder_get_object(builder, "terminalicone"));
	terminalmenu = GTK_WIDGET(gtk_builder_get_object(builder, "terminalmenu"));

	//sobre
	infosobre = GTK_WIDGET(gtk_builder_get_object(builder, "infosobre"));

	//logos
	logo = GTK_WIDGET(gtk_builder_get_object(builder, "logo"));
	logo2 = GTK_WIDGET(gtk_builder_get_object(builder, "logo2"));
	logo3 = GTK_WIDGET(gtk_builder_get_object(builder, "logo3"));
	logo4 = GTK_WIDGET(gtk_builder_get_object(builder, "logo4"));

	//iniciar
	iniciar		= GTK_WIDGET(gtk_builder_get_object(builder, "iniciar"));
	
	
	//cores
	GdkColor color;
	color.red = 0x7744;
	color.green = 0x8844;
	color.blue = 0xFF44;
	gtk_widget_modify_bg(GTK_WIDGET(window), GTK_STATE_NORMAL, &color); //background

	color.red = 0x3344;
	color.green = 0x3344;
	color.blue = 0x4444;

	//altera a cor do relógio
	const static GdkRGBA rgba_color = {.red = 1.0, .green = 1.0, .blue = 1.0, .alpha = 1.0}; //padrão rgba
	const static GdkRGBA rgba_color2 = {.red = 0.0, .green = 1.0, .blue = 0.0, .alpha = 1.0};
	gtk_widget_override_color(timer, GTK_STATE_FLAG_NORMAL, &rgba_color);

	//altera cor da font e fundo da barra iniciar
	gtk_widget_override_color(iniciar, GTK_STATE_FLAG_NORMAL, &rgba_color);
	gtk_widget_modify_bg(GTK_WIDGET(iniciar), GTK_STATE_NORMAL, &color);
	
	//altera a cor da barra inferior
	color.red = 0x2244;
	color.green = 0x2244;
	color.blue = 0x2244;
	gtk_widget_modify_bg(GTK_WIDGET(barrainferior), GTK_STATE_NORMAL, &color);
	
	//inicia o contador do relógio
	guint ID = g_timeout_add_seconds(1, (GSourceFunc) timer_handler, timer);
	gtk_widget_show(window);

	gtk_main();

	return 0;
}



//-----------------------------------------------------------------------
// Botões principais, responsáveis por iniciar os eventos
//-----------------------------------------------------------------------

/** on_botaopasta_clicked 
* @brief botão da pasta
*		 pid[2] reservado para pasta
*
*/
void on_botaopasta_clicked (GtkButton *b) {
	gtk_widget_show(pastaicone);
	gtk_widget_show(pastamenu);	

	sem_wait((sem_t*)&shared_area_ptr->mutex);
	if(shared_area_ptr->pids[2] == 0){
		pid_t pidpasta = fork();
		if(pidpasta == 0){
			shared_area_ptr->pids[2] = getpid();
			char *args[3];
			args[0] = "/usr/bin/nautilus";
        	args[1] = "/home";
			args[2] = NULL;
        	execve(args[0], args, __environ);
		}else{
			
		}
	}
	sem_post((sem_t*)&shared_area_ptr->mutex);
	
}

/** on_botaotexto_clicked 
* @brief botão do aplicativo de texto
*		 pid[1] reservado para app de texto
*
*/
void on_botaotexto_clicked (GtkButton *b) {
	gtk_widget_show(gediticone);
	gtk_widget_show(geditmenu);	

	sem_wait((sem_t*)&shared_area_ptr->mutex);
	if(shared_area_ptr->pids[1] == 0){
		pid_t pidexe = fork();
		if(pidexe == 0){
			shared_area_ptr->pids[1] = getpid();
			char *args[3];
			args[0] = "/usr/bin/gedit";
        	args[1] = "gedit";
			args[2] = NULL;
        	execve(args[0], args, __environ);
		}else{
			
		}
	}
	sem_post((sem_t*)&shared_area_ptr->mutex);
}
	

/** on_botaoterminal_clicked 
* @brief botão do shell linha de comando
*		 pid[3] reservado para o shell
*
*/
void on_botaoterminal_clicked (GtkButton *b) {
	gtk_widget_show(terminalicone);
	gtk_widget_show(terminalmenu);

	sem_wait((sem_t*)&shared_area_ptr->mutex);
	if(shared_area_ptr->pids[3] == 0){
		pid_t pidterm = fork();
		if(pidterm == 0){
			shared_area_ptr->pids[3] = getpid();
			char *args[3];
			args[0] = "/usr/bin/gnome-terminal";
        	args[1] = "";
			args[2] = NULL;
        	execve(args[0], args, __environ);
		}else{
			
		}
	}
	sem_post((sem_t*)&shared_area_ptr->mutex);
}
	

/** on_botaoexecutavel_clicked 
* @brief botão do app executável (sudoku)
*		 pid[0] reservado para o app executável
*
*/	
void	on_botaoexecutavel_clicked (GtkButton *b) {
	gtk_widget_show(sudokuicone);
	gtk_widget_show(sudokumenu);

	sem_wait((sem_t*)&shared_area_ptr->mutex);
	if(shared_area_ptr->pids[0] == 0){
		pid_t pidexe = fork();
		if(pidexe == 0){
			shared_area_ptr->pids[0] = getpid();
			char *args[3];
			args[0] = "/usr/games/gnome-sudoku";
        	args[1] = NULL;
			args[2] = NULL;
        	execve(args[0], args, __environ);		
		}else{
			
		}
	}
	sem_post((sem_t*)&shared_area_ptr->mutex);
}//fim_on_botaoexecutavel_clicked 
	

/** on_botaoapp1_clicked
* @brief botão do app extra 1 (navegador)
*		 pid[4] reservado para o app extra 1
*
*/	
void on_botaoapp1_clicked (GtkButton *b) {
	gtk_widget_show(flybirdicone);
	gtk_widget_show(flybirdmenu);

	sem_wait((sem_t*)&shared_area_ptr->mutex);
	if(shared_area_ptr->pids[4] == 0){
		pid_t pidapp = fork();
		if(pidapp == 0){
			shared_area_ptr->pids[4] = getpid();
			char *args[3];
			args[0] = "/usr/bin/firefox";
        	args[1] = "http://gustavoboliveira.atwebpages.com/clinica/";
			args[2] = NULL;
        	execve(args[0], args, __environ);	
		}else{
			
		}
	}
	sem_post((sem_t*)&shared_area_ptr->mutex);

}


//mostra imagem do sobre
void	on_sobre_select (GtkButton *b) {
	gtk_widget_show(infosobre);
}

void	on_sobre_deselect (GtkButton *b) {
	gtk_widget_hide(infosobre);
}

//altera o fundo de acordo com a preferência
void	on_preferencias_activate (GtkButton *b) {
	sem_wait((sem_t*)&shared_area_ptr->mutex);
	if(shared_area_ptr->pref == 1){
		gtk_widget_hide(logo);
		gtk_widget_show(logo2);
		shared_area_ptr->pref = 2;
	}else if(shared_area_ptr->pref == 2){
		gtk_widget_hide(logo2);
		gtk_widget_show(logo3);
		shared_area_ptr->pref = 3;
	}else if(shared_area_ptr->pref == 3){
		gtk_widget_hide(logo3);
		gtk_widget_show(logo4);
		shared_area_ptr->pref = 4;
	}else if (shared_area_ptr->pref == 4){
		gtk_widget_hide(logo4);
		gtk_widget_show(logo);
		shared_area_ptr->pref = 5;
	}else if (shared_area_ptr->pref == 5){
		gtk_widget_hide(logo);
		shared_area_ptr->pref = 1;
	}
	sem_post((sem_t*)&shared_area_ptr->mutex);

}

//-----------------------------------------------------------------------
// Botões de fechar e ocultar os ícones da barra inferior
//-----------------------------------------------------------------------

void	on_fecharterminal_activate (GtkButton *b) {
	sem_wait((sem_t*)&shared_area_ptr->mutex);
	if(shared_area_ptr->pids[3] != 0){
		kill(shared_area_ptr->pids[3],SIGTERM);
		system("killall bash");
		shared_area_ptr->pids[3] = 0;
		gtk_widget_hide(terminalmenu);
		gtk_widget_hide(terminalicone);
	}
	sem_post((sem_t*)&shared_area_ptr->mutex);
}
	
	

void	on_fecharsudoku_activate (GtkButton *b) {
	sem_wait((sem_t*)&shared_area_ptr->mutex);
	kill(shared_area_ptr->pids[0],SIGTERM);
	shared_area_ptr->pids[0] = 0;
	sem_post((sem_t*)&shared_area_ptr->mutex);
	gtk_widget_hide(sudokumenu);
	gtk_widget_hide(sudokuicone);
}

void	on_fechargedit_activate (GtkButton *b) {
	sem_wait((sem_t*)&shared_area_ptr->mutex);
	kill(shared_area_ptr->pids[1],SIGTERM);
	shared_area_ptr->pids[1] = 0;
	sem_post((sem_t*)&shared_area_ptr->mutex);
	gtk_widget_hide(geditmenu);
	gtk_widget_hide(gediticone);

}

void	on_fecharpasta_activate (GtkButton *b) {
	kill(shared_area_ptr->pids[2],SIGKILL); 
	system("killall nautilus"); //verificar
	sem_wait((sem_t*)&shared_area_ptr->mutex);
	shared_area_ptr->pids[2] = 0;
	sem_post((sem_t*)&shared_area_ptr->mutex);
	gtk_widget_hide(pastamenu);
	gtk_widget_hide(pastaicone);

}

void	on_fecharflybird_activate (GtkButton *b) {
	sem_wait((sem_t*)&shared_area_ptr->mutex);
	kill(shared_area_ptr->pids[4],SIGKILL);
	shared_area_ptr->pids[4] = 0;
	sem_post((sem_t*)&shared_area_ptr->mutex);
	gtk_widget_hide(flybirdmenu);
	gtk_widget_hide(flybirdicone);

}

//aplica o contador do relógio ao label timer
gboolean timer_handler(GtkWidget *timer){
	setlocale( LC_TIME, "portuguese" );
	time_t rawtime;
  	struct tm * timeinfo;
  	time (&rawtime);
  	timeinfo = localtime (&rawtime);
  	char buffer[256];

	strftime(buffer, sizeof(buffer), "%a. %d de %b. %Y - %H:%M:%S", timeinfo);
	gtk_label_set_text(GTK_LABEL(timer), buffer);
	return TRUE;
}
	
/** on_event1_motion_notify_event
* @brief Função resonponsável por 
*      	 calcular a posição ao arrastar
*		 os ícones da área de trabalho
*       
* @param GtkEventBox *e, GdkEventMotion *event (id do evento)
* @return gboolean TRUE
*
* @bugs Ajustes superiores não funcionam tão bem quanto os laterais
*/
gboolean on_event1_motion_notify_event(GtkEventBox *e, GdkEventMotion *event) {

	int x_event = event -> x_root;	// relativa a janela principal
	int y_event = event -> y_root;	

	int x_main = event -> x;	// relativo à janela de evento
	int y_main = event -> y;	
	GdkWindow *Geventbox = gtk_widget_get_window(GTK_WIDGET(e)); // GDK event box window

	int x, y;

	gdk_window_get_position (Geventbox, &x, &y);

	int hor = gtk_widget_get_allocated_width (GTK_WIDGET(e));
	int ver = gtk_widget_get_allocated_height (GTK_WIDGET(e));

	GdkWindow *Gwindow = gtk_widget_get_window(window); // GDK desktop window
	GdkSeat *seat = gdk_display_get_default_seat(gdk_display_get_default ()); // dispositivos disponíveis
	GdkDevice *mouse = gdk_seat_get_pointer(seat); // mouse 
	gdk_window_get_device_position(Gwindow, mouse, &x, &y, NULL); // local do mouse cursor
	gtk_fixed_move (GTK_FIXED(fixed1), GTK_WIDGET(e), x-(hor/2), y-(ver/2));

	return TRUE; 

}

//funções da barra iniciar para desligar, reiniciar e suspender.
void on_quit(){
	exit(0);
	sync();
	reboot(RB_POWER_OFF);
}

void on_reboot(){
	exit(0);
	sync(); 
	reboot(RB_AUTOBOOT);
}

