// daemon.c - gathering info daemon
// 
// Copyright 2012 Edward V. Emelianoff <eddy@sao.ru>
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA 02110-1301, USA.

#include "defines.h"
#include "quick_median.c"

uint Nth = 1 << CACHE_EVERY_PWR; // шаг по кэшированию
int MAX_DATA_LEN = 1;	// длина массивов данных
int shmid;
int cache_fd, out_fd;	// дескрипторы файлов кэша и выходного файла
FILE *F_log;
char TERMINATED[]	= "Caught SIGTERM. Quit.";
char HUPPED[]		= "Caught SIGHUP. Quit.";
char QUITTED[]		= "Caught SIGQUIT. Quit.";
char INTTED[]		= "Caught SIGINT. Quit.";
char STARTED[]		= "Started.";
char *pidfilename	= PID_FILE;

data_type *t_out=NULL, *t_in=NULL, *t_mir=NULL,
	*pres=NULL, *hum=NULL; // будущие массивы для данных

int readname(char *name, pid_t pid){ // считать имя процесса из /proc/...
	char *pp = name, byte, path[256];
	int cntr = 0, file;
	snprintf (path, 255, PROC_BASE "/%d/cmdline", pid);
	file = open(path, O_RDONLY);
	if(file == -1) return 0; // нет файла или чужой процесс
	do{	// считываем имя без слешей
		read(file, &byte, 1);
		if (byte != '/') *pp++ = byte;
		else pp = name;
	}
	while(byte != EOF && byte != 0 && cntr++ < 255);
	name[255] = 0;
	close(file);
	return 1;
}

void check4running(){
	DIR *dir;
	FILE* pidfile;
	struct dirent *de;
	struct stat s_buf;
	pid_t pid, self, run = 0;
	char name[256], myname[256];
	if (!(dir = opendir(PROC_BASE))){ // открываем директорию /proc
		perror(PROC_BASE);
		exit(1);
	}
	self = getpid(); // свой идентификатор
	if(stat(pidfilename, &s_buf) == 0){ // есть файл с pid'ом
		pidfile = fopen(pidfilename, "r");
		fscanf(pidfile, "%d", &run); // получаем pid (возможно) запущенного процесса
		fclose(pidfile);
		if(readname(name, run) && strncmp(name, myname, 255) == 0){
			WARN("\nFound running process (pid=%d), exit.\n", run);
			exit(0); 
		}
	}
	// файла с pid'ом нет, или там неправильная запись
	readname(myname, self); // свое имя процесса
	while ((de = readdir (dir)) != NULL){ // пока не дойдем до конца директории
	// пропустим, если директория не указывает на процесс, или указывает на self
		if (!(pid = (pid_t) atoi (de->d_name)) || pid == self)
			continue;
		readname(name, pid); // считываем имя процесса
		if(strncmp(name, myname, 255) == 0){ // если оно совпадает с myname
			WARN("\nFound running process (pid=%d), exit.\n", pid);
			exit(0);
		}
	}
	closedir(dir);
	unlink(pidfilename); // пробуем удалить pidfilename
	WARN("my PID: %d\n", self);
	pidfile = fopen(pidfilename, "w");
	fprintf(pidfile, "%d\n", self); // записываем в pidfilename свой pid
	fclose(pidfile);
}

inline void LOG(char* thetime, char* thetext){
	fprintf(stderr, "%s\t%s\n", thetime, thetext);
	fprintf(F_log, "%s\t%s\n",  thetime, thetext);
}

double dtime(){
	double ret;
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	ret = tv.tv_sec + tv.tv_usec / 1000000.;
	return ret;
}

void printdate(char *s_time){
	time_t now = time(NULL);
	struct tm ltime = *localtime(&now);
	strftime(s_time, 32, "%d/%m/%Y, %H:%M:%S", &ltime);
}

static void signals(int sig){
	char nowatime[32];
	char *str;
	int u;
	printdate(nowatime);
	if(sig == SIGTERM)
		str = TERMINATED;
	else if(sig == SIGHUP)
		str = HUPPED;
	else if(sig == SIGQUIT)
		str = QUITTED;
	else if(sig == SIGINT)
		str = INTTED;
	LOG(nowatime, str);
	shmctl(shmid, IPC_RMID, NULL); // удаляем сегмент *Visor
	close(cache_fd);
	close(out_fd);
	fclose(F_log);
	if(t_out) free(t_out); if(t_in) free(t_in);
	if(t_mir) free(t_mir); if(pres) free(pres);
	if(hum) free(hum);
	u = unlink(pidfilename);
	if(u == -1) perror("Can't delete PIDfile");
	exit(sig);
}

Status current_status(){// возвращаем текущее состояние телескопа
/*
 *	STOP	- телескоп выключен
 *	GUIDING	- ведение
 *	READY	- все остановлено, питание есть
 *	OTHER	- наведение или прочее перемещение телескопа/купола
 *  OPEN	- идут наблюдения
 */
 	time_t tt = time(NULL);
 	struct tm t_now = *localtime(&tt);
 	if(t_now.tm_hour > 7 && t_now.tm_hour < 18) *Visor = 0; // "закрываем" забрало днем
 	if(*Visor) return OPEN; // забрало открыто
	if(Tel_Hardware == Hard_Off){ // варианты STOP, OTHER
		if(Dome_State == D_Off) return STOP; // все выключено
		return OTHER; // купол движется
	}
	if(Tel_State == Stopping){ // варианты READY, OTHER
		if(Dome_State == D_Off) return READY; // питание есть, все остановлено
		return OTHER; // купол движется
	}
	// варианты GUIDING, OTHER
	if(Tel_Mode == Automatic) return GUIDING; // наблюдения
	return OTHER; // наведение, ручная коррекция
}

int get_data(monit_d *data){// получаем данные
/*
	превышение std 30% среднего значения - возвращаем 0
	по ошибке возвращаем -1
	все нормально - возвращаем 1
*/
	int nn = 0;
	double tt;
	data_type *to = t_out, *ti = t_in, *tm = t_mir, max_wnd = 0.,
		*p = pres, *h = hum;
// Состоянием телескопа считать последнее
	tt = dtime();
	while(check_shm_block(&sdat) && (dtime() - tt < TIMEINTERVAL)){
		*to++	= val_T1;
		*ti++	= val_T2;
		*tm++	= val_T3;
		if(val_Wnd > max_wnd) max_wnd = val_Wnd;
		*p++	= Pressure;
		*h++	= val_Hmd;
		if(++nn == MAX_DATA_LEN) break;
		usleep(100000); // опрашиваем 10 раз в секунду
// !!! на цифре 10 завязано вычисление объема памяти для calloc, если надо будет
// изменить величину паузы, необходимо и скорректировать MAX_DATA_LEN в main
	}
	if(!check_shm_block(&sdat)) return -1; // ошибка получения данных
	data->seconds		= time(NULL) - TIMEINTERVAL/2;
	// вычисляем медианные показания
	data->outdoor_temp	= quick_select(t_out, nn);
	data->indoor_temp	= quick_select(t_in, nn);
	data->mirror_temp	= quick_select(t_mir, nn);
	data->wind_speed	= max_wnd;
	data->pressure		= quick_select(pres, nn);
	data->humidity		= quick_select(hum, nn);
	data->status		= current_status();	
	return 1;
}

void mkcache(uint *remain){
	char nowatime[32];
	off_t offset = 0; // в данном случае - off64_t, т.е. long long
	Cache s_cache;
	struct stat filestat;
	unsigned long long cntr = 1ULL;
	uint cc = 1;
	monit_d data;
	printdate(nowatime);
	LOG(nowatime, "Making cache");
	if(read(out_fd, &data, sizeof(data)) < sizeof(data)){
		LOG(nowatime, "Corrupted out file");
		exit(1);
	}
	s_cache.offset = 0;	
	s_cache.time = data.seconds;
	write(cache_fd, &s_cache, sizeof(s_cache));
	do{
		if(cc >= Nth){
			s_cache.time = data.seconds;
			write(cache_fd, &s_cache, sizeof(s_cache));
			cc = 0;
		}
		cc++; cntr++;
		s_cache.offset = tell(out_fd);
	}while(read(out_fd, &data, sizeof(data)) == sizeof(data));
	*remain = cntr && (Nth - 1);
	printdate(nowatime);
	LOG(nowatime, "Cache is ready");
}

int main(int argc, char** argv){
	char nowatime[32];
	struct stat s_stat;
	uint cc = Nth + 1;
	int tmp;
	FILE *pidfile;
	monit_d data;
	Cache s_cache;
	check4running();
	if(!(F_log = fopen(LOG_FILE, "a"))){
		fprintf(stderr, "Can't open log file, quit\n");
		exit(1);
	}
	setbuf(F_log, NULL);
	printdate(nowatime);
	LOG(nowatime, STARTED);
	if((cache_fd = open(CACHE_FILE, O_RDWR|O_CREAT, 00644)) < 0){
		LOG(nowatime, "Can't open cache file, quit");
		exit(1);
	}
	if((out_fd = open(OUT_FILE, O_WRONLY|O_CREAT, 00644)) < 0){
		LOG(nowatime, "Can't open out file, quit");
		exit(1);
	}
	if(stat(OUT_FILE, &s_stat) < 0){
		LOG(nowatime, "Can't stat out file, quit");
		exit(1);		
	}
	if(s_stat.st_size > 0){	// если файл с данными есть, проверяем наличие записей в кэше
		if(stat(CACHE_FILE, &s_stat) < 0){
			LOG(nowatime, "Can't stat cache file, quit");
			exit(1);		
		}
		if(s_stat.st_size == 0)
			mkcache(&cc);
		lseek(cache_fd, 0, SEEK_END);
		lseek(out_fd, 0, SEEK_END);
	}
	close(0); close(1); close(2);
	if(fork() != 0) exit(0);
	signal(SIGTERM, signals); // kill (-15)
	signal(SIGHUP, SIG_IGN);  // на него можно что-нибудь повесить
	signal(SIGINT, signals);  // ctrl+C
	signal(SIGQUIT, signals); // ctrl+\  
	signal(SIGTSTP, SIG_IGN); // игнорируем ctrl+Z
	get_shm_block( &sdat, ClientSide);
	shmid = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0666);
	Visor = (int*) shmat(shmid, NULL, 0);
	MAX_DATA_LEN = ((int)(TIMEINTERVAL)) * 10 + 11; // 11 = погрешность округления + 1 про запас
	t_out	= (data_type*)calloc(MAX_DATA_LEN, sizeof(data_type));
	t_in	= (data_type*)calloc(MAX_DATA_LEN, sizeof(data_type));
	t_mir	= (data_type*)calloc(MAX_DATA_LEN, sizeof(data_type));
	pres	= (data_type*)calloc(MAX_DATA_LEN, sizeof(data_type));
	hum	= (data_type*)calloc(MAX_DATA_LEN, sizeof(data_type));
	while(1){
		tmp = get_data(&data);
		if(tmp == 0){
			sleep(10);
			continue;
		}
		if(tmp == -1){
			printdate(nowatime);
			LOG(nowatime, "Error getting data");
			sleep(300);
			continue;
		}
		s_cache.offset = tell(out_fd);
		write(out_fd, &data, sizeof(data));
		if(++cc >= Nth){
			s_cache.time = data.seconds;
			write(cache_fd, &s_cache, sizeof(s_cache));
			cc = 0;	
			printdate(nowatime);
			LOG(nowatime, "Add cache data");
		}
	}
}

