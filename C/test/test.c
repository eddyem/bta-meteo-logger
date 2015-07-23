#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include "lang.h"
#include "bta_shdata.h"

#define MAX_QUERY_SIZE		1024 // максимальный объем данных, передаваемых в запросе 
#define MAX_IPs			20
#define SHMSZ			((MAX_IPs + 1) * sizeof(struct shm_data)) // резервируем на каждый key память для MAX_IPs IP
#define TIMEINTERVAL		60 // интервал в секундах между запросами данных
#define	MAX_MONITORING_TIME	86400 // максимальное время, в течение которого будет выполняться мониторинг
static char* SCRIPT_PATH = "/cgi-bin/eddy/test"; // полный путь к скрипту

int MAXLEN = MAX_MONITORING_TIME/TIMEINTERVAL;

unsigned char Lang = 1; // английский
key_t key;
char  IP[16];
struct shm_data{ // данные, размещаемые для каждого key в разделяемой памяти
	char IP[16]; // IP-адрес
	pid_t pid;   // pid, соответствующий процессу для этого IP
};
typedef struct shm_data shm_d;
struct monitoring_data{
	time_t seconds;		// время в секундах
	float outdoor_temp;	// температура снаружи
	float indoor_temp;	// температура внутри
	float mirror_temp;	// температура зеркала
	float wind_speed;	// скорость ветра
};
typedef struct monitoring_data monit_d;
monit_d *Monitoring;
int Monit_length = 0;
int Kill = 0;
unsigned char Graph = 0;

void detouch_shm();
void print_curvals();
void sendSVG(pid_t pid);
void blankSVG();

double dtime(){
	double ret;
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	ret = tv.tv_sec + tv.tv_usec / 1000000.;
	return ret;
}

static void signals(int sig){
	if(sig == SIGTERM || sig == SIGUSR1){
		char FIFO[32];
		int f, i;
		snprintf(FIFO, 32, "/tmp/%d-%d", getpid(), sig);
		mkfifo(FIFO, 0666);
		if((f = open(FIFO, O_WRONLY)) >0){
			unlink(FIFO);
			for(i = 0; i< Monit_length; i++)
				write(f, &Monitoring[i], sizeof(monit_d));
			close(f);
		}
	}
	else if(sig == SIGHUP) print_curvals();
	if(sig == SIGTERM){
		detouch_shm();
		exit(0);
	}
	signal(sig, signals);
}

void unhexdump(char *inp){
	char tmp[512], *o_ptr = inp, *tok;
	unsigned char ch;
	unsigned int a;
	strncpy(tmp, inp, 512);
	tok = strtok(tmp, "%");
	do{
		sscanf(tok, "%x", &a);
		ch = a;
		*o_ptr++ = ch;
	}while(tok = strtok(NULL, "%"));
	*o_ptr = 0;
}

void minmax(float *min, float *max, float param){
	if(param > *max) *max = param;
	else if(param < *min) *min = param;
}

void print_curvals(){
	int i, f;
	char FIFO[32], *cur_stat;
	monit_d *ptr = Monitoring;
	float	max_otemp, min_otemp,
		max_itemp, min_itemp,
		max_mtemp, min_mtemp,
		max_wind, min_wind;
	max_otemp = min_otemp = ptr->outdoor_temp;
	max_itemp = min_itemp = ptr->indoor_temp;
	max_mtemp = min_mtemp = ptr->mirror_temp;
	max_wind = min_wind = ptr->wind_speed;
	for(i = 1; i< Monit_length; i++){
		ptr++;
		minmax(&min_otemp, &max_otemp, ptr->outdoor_temp);
		minmax(&min_itemp, &max_itemp, ptr->indoor_temp);
		minmax(&min_mtemp, &max_mtemp, ptr->mirror_temp);
		minmax(&min_wind, &max_wind, ptr->wind_speed);
	}
	cur_stat = (char*)calloc(1024, 1);
/* Табличка:
	Наруж.т		Внут.т.		Т.зерк.		Ск.ветра
min
max
*/
	snprintf(cur_stat, 1024, "<div><table>\n"
		"<tr><th rowspan=\"2\"></th><th colspan=\"3\">%s</th><th rowspan=\"2\">%s</th></tr>\n"
		"<tr><th>%s</th><th>%s</th><th>%s</th></tr>\n"
		"<tr><td>min</td><td>%.1f</td><td>%.1f</td><td>%.1f</td><td>%.1f</td></tr>\n"
		"<tr><td>max</td><td>%.1f</td><td>%.1f</td><td>%.1f</td><td>%.1f</td></tr></table>\n"
		"%s: %d</div>\n",
		_L(_s_Temp_), _L(_s_WSpeed_),
		_L(_s_Otemp_), _L(_s_Itemp_), _L(_s_Mtemp_),
		min_otemp, min_itemp, min_mtemp, min_wind,
		max_otemp, max_itemp, max_mtemp, max_wind,
		_L(_s_Monlen_), Monit_length);
	snprintf(FIFO, 32, "/tmp/%d", getpid());
	mkfifo(FIFO, 0666);
	if((f = open(FIFO, O_WRONLY)) >0){
		unlink(FIFO);
		write(f, cur_stat, 1024);
		close(f);
	}
	free(cur_stat);
}

char* get_qs(char* buf, int l){
	char *m, *qs = NULL;
	if((m = getenv("REQUEST_METHOD")) && strcasecmp(m, "POST") == 0)
		qs = fgets(buf, l, stdin);
	else if( (qs = getenv("QUERY_STRING")) )
		qs = strncpy(buf, qs, l);
	if(qs && strlen(qs) < 1) qs = NULL;
	return qs;
}

int get_qs_param(char *qs, char *param, char *meaning, int l){
	char *tok, *val, *par, str[l+1];
	int stat = 0;
	strncpy(str, qs, l);
	tok = strtok(str, "& \n");
	do{
		if((val = strchr(tok, '=')) == NULL) continue;
		*val++ = '\0';
		par = tok;
		if(strcasecmp(par, param)==0){
			if(strlen(val) > 0){
				stat = 1;
				strncpy(meaning, val, l);
			}
			break;
		}
	}while((tok = strtok(NULL, "& \n"))!=NULL);
	return stat;
}

void getkey(){ // key вычисляется как последние три байта IP-адреса
	int cntr=0;
	char IPnum[13], tmp[4], ipaddr[16];
	char *tok = strtok(IP, ".");
	strncpy(ipaddr, IP, 16);
	IPnum[0] = 0;
	do{
		strncpy(tmp, tok, 4);
		if(cntr > 0)
			sprintf(IPnum, "%s%s", IPnum, tmp);
		cntr++;
		if(cntr > 4) break;
	}while((tok = strtok(NULL, "."))!=NULL);
	key = atoi(IPnum);
}

void Refresh(){
	printf("<div><button type='submit' OnClick='refresh_all();'>%s</button></div>\n", _L(_s_Refresh_));
	printf("<script language=\"JavaScript\">$('widecen').style.display='block';</script>\n");
}

void mk_starting_form(){
	printf("%s:<input type='text' name='Name' OnChange='submit_form();' size=50>\n", _L(_s_Name_));
	printf("<button type='submit' OnClick='submit_form();'>OK</button>\n");
	printf("<script language=\"JavaScript\">$('widecen').style.display='none';</script>\n");
}

void mk_stopping_form(char *timeout){
	printf("<a href=\"%s?Kill=1\" OnClick='sleep(5);window.location.reload();'>%s</a>\n", SCRIPT_PATH, _L(_s_Stop_n_write_));
	printf("<script language=\"JavaScript\">start_monitoring(%s);</script>\n", timeout);
	Refresh();
}

void detouch_shm(){
	shm_d *Data, *ptr;
	int i = 0, shmid;
	if ((shmid = shmget(key, SHMSZ, 0666)) < 0)
		return;
	if((Data = (shm_d*)shmat(shmid, NULL, 0)) == (shm_d *) -1)
		return;
	ptr = Data;
	do{
		if(!*(ptr->IP)){
			i = MAX_IPs;
			break;
		}
		if(strcmp(ptr->IP, IP) == 0) break;
		ptr++;
	}while(i < MAX_IPs);
	if(i != MAX_IPs){
		i++;
		if(i == MAX_IPs) *(ptr->IP) = 0; // наша запись - последняя
		else
		while(i < MAX_IPs){ // передвигаем следующие записи
			if(!*((ptr+1)->IP)){
				*(ptr->IP) = 0;
				break;
			}
			strcpy(ptr->IP, (ptr+1)->IP);
			ptr->pid = (ptr+1)->pid;
			ptr++; i++;
		}
	}
}

void printdata(monit_d data){
	struct tm ltime = *localtime(&data.seconds);
	char s_time[32];
	strftime(s_time, 32, "%d/%m/%Y, %H:%M:%S", &ltime);
	printf("%s\t%.1f\t%.1f\t%.1f\t%.1f\n",
		s_time,
		data.outdoor_temp,
		data.indoor_temp,
		data.mirror_temp,
		data.wind_speed);
}

void kill_and_save(pid_t pid){
/*
Передаем процессу pid имя файла для сохранения и посылаем сигнал завершения;
либо узнаем у pid, куда пишет, прихлопываем его и сохраняем файл;
*/
	char FIFO[32], *cur_stat;
	int f, i, rb, sig_;
	struct timeval tv;
	fd_set rfds;
	monit_d data;
	if(pid == 0) exit(0);
	sig_ = (Kill) ? SIGTERM : SIGHUP;
	if(Kill) snprintf(FIFO, 32, "/tmp/%d-%d", pid, SIGTERM);
	else snprintf(FIFO, 32, "/tmp/%d", pid);
	mkfifo(FIFO, 0666);
	if((f = open(FIFO, O_RDONLY|O_NONBLOCK)) < 0) return;
	if(kill(pid, sig_) != 0){ // посылаем сигнал о готовности к чтению
		unlink(FIFO);
		detouch_shm();
		close(f);
		exit(0);
	}
	FD_ZERO(&rfds);
	FD_SET(f, &rfds);
	tv.tv_sec = 5; tv.tv_usec = 0; // ждем готовности не более 5 секунд
	select(f+1, &rfds, NULL, NULL, &tv);
	i = 0;
	if(Kill) do{
		FD_ZERO(&rfds);
		FD_SET(f, &rfds);
		tv.tv_sec = 1; tv.tv_usec = 0; // 1 секунда на ожидание
		if(select(f+1, &rfds, NULL, NULL, &tv) > 0)
			if(FD_ISSET(f, &rfds)){
				rb = read(f, &data, sizeof(data));
				if(rb > 0){
					printdata(data);
					i++;
				}
				else break;
			}
		else {printf("no data...\n"); break;}
	}while(i < MAXLEN);
	else {
		cur_stat = (char*)calloc(1024, 1);
		FD_ZERO(&rfds);
		FD_SET(f, &rfds);
		tv.tv_sec = 1; tv.tv_usec = 0; // 1 секунда на ожидание
		if(select(f+1, &rfds, NULL, NULL, &tv) > 0)
			if(FD_ISSET(f, &rfds)){
				rb = read(f, cur_stat, 1024);
				if(rb > 0){
					printf("%s\n", cur_stat);
				}
			}
		free(cur_stat);
		mk_stopping_form("");
	}
	close(f);
	exit(0);
}

void fill_forms(const char* alert_message, const char S_flag){
/*
В заголовок страницы выводим alert_message;
проверяем, не запущен ли уже процесс с IP
если запущен - генерируем форму остановки мониторинга и сохранения результатов в файл
иначе - генерируем начальную форму
*/
	int shmid, i = 0;
	shm_d *Data, *ptr;
	if(alert_message) printf("<div><h1 align=center>%s</h1></div>\n", alert_message);
	if ((shmid = shmget(key, SHMSZ, 0666)) < 0){ // для этого ключа еще не выделялась память
		if(Graph){
//			blankSVG();
			exit(0);
		}
		if(S_flag) // хотим удалить процесс, для которого нет записей
			printf("<div><h1 align=center>%s</h1></div>\n", _L(_s_Mon_didnt_start_));
		mk_starting_form(); // генерируем начальную форму
		return;
	}
	if((Data = (shm_d*)shmat(shmid, NULL, 0)) == (shm_d *) -1){
		if(Graph){
//			blankSVG();
			exit(0);
		}
		printf("<div><h1 align=center>%s:<p>%s</h1></div>\n", _L(_s_Err_), _L(_s_Cant_shmat_));
		return;
	}
	// теперь проверяем, запущен ли процесс для данного IP
	ptr = Data; i = 0;
	do{
		if(!*(ptr->IP)){ // список закончился
			i = MAX_IPs;
			break;
		}
		if(strcmp(ptr->IP, IP) == 0) break;
		ptr++;
	}while(i < MAX_IPs);
	if(i == MAX_IPs){ // такой ключ уже есть, но для данного IP процесса не существует
		if(Graph){
//			blankSVG();
			exit(0);
		}
		if(S_flag) printf("<div><h1 align=center>%s</h1></div>\n", _L(_s_Cant_find_IP_));
		mk_starting_form();
		return;
	}
	// Для данного IP уже запущен процесс
	if(Graph) sendSVG(ptr->pid);
	else kill_and_save(ptr->pid);
}

void get_data(){// получаем данные
	int nn = 0;
	double tt;
	double t1=0., t2=0., t3=0., wnd=0.;
	monit_d *ptr = &Monitoring[Monit_length];
	tt = dtime();
	while(check_shm_block(&sdat) && (dtime() - tt < 9.99)){
		t1 += val_T1;
		t2 += val_T2;
		t3 += val_T3;
		wnd += val_Wnd;
		nn++;
		usleep(100000);
	}
	ptr-> seconds = time(NULL);// - starting_time;
	ptr->outdoor_temp	= t1 / nn;
	ptr->indoor_temp	= t2 / nn;
	ptr->mirror_temp	= t3 / nn;
	ptr->wind_speed		= wnd / nn;
	Monit_length++;
}

void start_monitoring(char *Name){
	int shmid, i = 0;
	unsigned char not_first = 1;
	shm_d *Data, *ptr;
	pid_t pid;
	printf("<div>%s</div>\n", Name);
	if ((shmid = shmget(key, SHMSZ, 0666)) < 0){ // память еще не выделялась - выделяем
		not_first = 0;
		if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0){
			printf("<div><h1 align=center>%s<br>%s</h1></div>\n", _L(_s_Err_), _L(_s_Cant_shmget_));
			return;
		}
	}
	if((Data = (shm_d*)shmat(shmid, NULL, 0)) == (shm_d *) -1){
		printf("<div><h1 align=center>%s<p>%s</h1></div>\n", _L(_s_Err_), _L(_s_Cant_shmat_));
		return;
	}
	ptr = Data;
	if(not_first){
		i = 0;
		do{
			if(!*(ptr->IP)){ // список закончился
				i = MAX_IPs;
				break;
			}
			if(strcmp(ptr->IP, IP) == 0) break;
			ptr++; i++;
		}while(i < MAX_IPs);
		if(i != MAX_IPs){ 
			printf("<div><h1 align=center>%s (pid: %d)</h1></div>\n", _L(_s_Mon_running_), ptr->pid);
			kill_and_save(i);
			return;
		}
	}
	printf("<div><h1 align=center>%s<p>%s</h1>\n", _L(_s_Startmon_), _L(_s_Can_close_));
	printf("<h1>%s</h1></div>\n", _L(_s_Refreshing_));
	mk_stopping_form("5000");
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
	if((pid = fork()) != 0){exit(0);}
	strncpy(ptr->IP, IP, 16);
	ptr->pid = getpid();
	signal(SIGTERM, signals);
	signal(SIGHUP, signals);
	signal(SIGUSR1, signals);
	get_shm_block( &sdat, ClientSide);
	time_t t0 = time(NULL);
	time_t tt, t1 = t0;
	int size_ = MAX_MONITORING_TIME / TIMEINTERVAL + 1;
	Monitoring = calloc(size_, sizeof(monit_d));
	do{
		get_data();
		while( ((tt = time(NULL)) - t1) < TIMEINTERVAL )
			usleep(1000);
		t1 = tt;
	} while((tt - t0) < MAX_MONITORING_TIME);
	detouch_shm();
	exit(1);
}

void blankSVG(){printf("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
		"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\"\n"
		"\"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\n"
		"<?xmlstylesheet url=\"index.xsl\"?>\n"
		"<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n"
		"<g></g></svg>");
}

void sendSVG(pid_t pid){
	FILE *plot;
	char FIFO[32], *cur_stat, s_time[32];
	struct tm ltime;
	struct timeval tv;
	fd_set rfds;
	monit_d *data, *ptr;
	unsigned char Flag, Comma = 0;
	char *titles[4] = {"T out", "T in", "T mir", "Wind"};
	//char *startplot = "plot '-' with lines title ";
	char *command= "/usr/bin/gnuplot";
//	char *set_term = "set terminal svg\nset xdata time\nset timefmt \"\%d/\%m-\%H:\%M\"\nset format x \"\%H:\%M\"\n";
	char *set_term = "set terminal gif size 800,600\nset xdata time\nset timefmt \"\%d/\%m-\%H:\%M\"\nset format x \"\%H:\%M\"\n";
	int f, i, j, d_len, rb;
	data = calloc(MAXLEN+1, sizeof(monit_d));
	ptr = data;
	if(pid == 0 || Graph == 0 || Graph > 15){
//		blankSVG();
		exit(0);
	}
	snprintf(FIFO, 32, "/tmp/%d-%d", pid, SIGUSR1);
	mkfifo(FIFO, 0666);
	if((f = open(FIFO, O_RDONLY|O_NONBLOCK)) < 0){
//		blankSVG();
		exit(0);		
	}
	if(kill(pid, SIGUSR1) != 0){ // посылаем сигнал о готовности к чтению
//		blankSVG();	
		close(f);
		exit(0);
	}
	FD_ZERO(&rfds);
	FD_SET(f, &rfds);
	tv.tv_sec = 5; tv.tv_usec = 0; // ждем готовности не более 5 секунд
	select(f+1, &rfds, NULL, NULL, &tv);
	d_len = 0;
	do{
		FD_ZERO(&rfds);
		FD_SET(f, &rfds);
		tv.tv_sec = 1; tv.tv_usec = 0; // 1 секунда на ожидание
		if(select(f+1, &rfds, NULL, NULL, &tv) > 0)
			if(FD_ISSET(f, &rfds)){
				rb = read(f, ptr, sizeof(monit_d));
				if(rb > 0){
					d_len++;
					ptr++;
				}
				else break;
			}
		else break;
	}while(d_len < MAXLEN);
	close(f);
	if(d_len < 2){
//		blankSVG();
		exit(0);
	}
	plot = popen(command, "w");
	if(plot == NULL){
//		blankSVG();
		exit(0);
	}
	fprintf(plot, "%s", set_term);
	fprintf(plot, "plot ");
	for(j = 0; j < 4; j++){
		Flag = Graph & (1 << j);
		if(Flag){
			if(Comma) fprintf(plot, ",");
			Comma = 1;
			fprintf(plot, " '-' using 1:2 with lines lt %d title '%s'", j+1, titles[j]);
		}
	}
	fprintf(plot, "\n");
	for(j = 0; j < 4; j++){
		Flag = Graph & (1 << j);
		if(Flag){
			for(i = 0; i < d_len; i++){
				ltime = *localtime(&data[i].seconds);
				strftime(s_time, 32, "%d/%m-%H:%M", &ltime);
				fprintf(plot, "%s ", s_time);
				switch(Flag){
					case 1:	fprintf(plot, "%.1f\n", data[i].outdoor_temp); break;
					case 2:	fprintf(plot, "%.1f\n", data[i].indoor_temp); break;
					case 4:	fprintf(plot, "%.1f\n", data[i].mirror_temp); break;
					case 8:	fprintf(plot, "%.1f\n", data[i].wind_speed); break;
				}
			}
		fprintf(plot, "e\n");
		}
	}
	fflush(plot);
	pclose(plot);
	fflush(stdout);
	printf("<?xmlstylesheet url=\"index.xsl\"?>\n");
	free(data);
}

int main(){
	char *qs, *buf, Name[512], *ptr, tmp[16];
	void quit(int status){
		free(buf);
		exit(status);
	}
	setbuf(stdout, NULL);
	buf = (char*)calloc(MAX_QUERY_SIZE, 1);
	qs = get_qs(buf, 1024);
	ptr = getenv("HTTP_ACCEPT_LANGUAGE");
	if(ptr) if(strncmp(ptr, "ru", 2) == 0) Lang = 0; // используем русский
	ptr = getenv("REMOTE_ADDR");
	if(!ptr){
	printf("Content-type: multipart/form-data; charset=koi8-r\n\n");	
		printf("<div><h1 align=center>%s</h1>\n", _L(_s_No_IP_));
		printf("%s</div>\n", _L(_s_Solvethis_));
		quit(0);
	}
	strncpy(IP, ptr, 16);
	getkey();
	if(!qs){
		printf("Content-type: multipart/form-data; charset=koi8-r\n\n");
		fill_forms(NULL, 0);
		quit(0);
	}
	if(get_qs_param(qs, "Graph", tmp, 16)){
		printf("Content-type: image/gif\n\n");
	//	printf("Content-type: image/svg+xml\n\n");
		Graph = atoi(tmp);
		if(Graph == 0) Graph = 16;
		fill_forms(NULL, 1);
		quit(0);
	}
	if(get_qs_param(qs, "Kill", tmp, 16)){
	printf("Content-type: text/plain; charset=koi8-r\n\n");
		Kill = 1;
		fill_forms(NULL, 1);
		quit(0);
	}
	printf("Content-type: multipart/form-data; charset=koi8-r\n\n");
	if(get_qs_param(qs, "Show", tmp, 16)){
		fill_forms(NULL, 1);
		quit(0);		
	}
	if(!get_qs_param(qs, "Name", Name, 512)){
		fill_forms(_L(_s_Fillname_), 0);
		quit(0);
	}
	unhexdump(Name);
	start_monitoring(Name);
	quit(0);
}

