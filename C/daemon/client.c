// client.c
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


#define __CLIENT_C__
#include <string.h>
#include "defines.h"
#include "lang.h"
#include "quick_median.c"

#define MAX_QUERY_SIZE		1024 // максимальный объем данных, передаваемых в запросе
#define SVG			0	// изображение - svg (этот параметр должен быть самым первым)
#define JPG			1
//#define JPG			1 -> GIF = 2
#define GIF			2	// изображение - gif (должен быть самым последним)

static char* Content_type = "Content-type: multipart/form-data; charset=koi8-r\n\n";
static char* SCRIPT_PATH = "/cgi-bin/eddy/tempmon"; // полный путь к скрипту
char *qs = NULL, *buf = NULL;
unsigned char Lang = 1; // английский
long long starting_pos = 0LL;
typedef struct shm_data shm_d;
int Graph = 0;
time_t t_start, t_end;
time_t aver_interval = 1;
int out_fd;
void sendSVG(const int image_type, unsigned char stat_mask, int im_height);

inline void minmax(float *min, float *max, float param){
	if(param > *max) *max = param;
	else if(param < *min) *min = param;
}

void print_curvals(unsigned char stat_mask){ // stat_mask -  битовая маска для Status
	int i=1, f, d_len = 0;
	monit_d data;
	float	max_otemp, min_otemp, avr_otemp,	// внешняя температура
			max_itemp, min_itemp, avr_itemp,	// внутренняя температура
			max_mtemp, min_mtemp, avr_mtemp,	// температура зеркала
			max_wind,  min_wind,  avr_wind,		// скорость ветра
			max_pres,  min_pres,  avr_pres,		// давление
			max_hmd,   min_hmd,   avr_hmd;		// влажность
	printf(Content_type);
	while(read(out_fd, &data, sizeof(data)) == sizeof(data)){
		if(data.seconds < t_start) continue;
		if(data.seconds > t_end) break;
		if(!(stat_mask & (1 << data.status))) continue;
		d_len++;
		if(i){
			max_otemp = min_otemp = avr_otemp = data.outdoor_temp;
			max_itemp = min_itemp = avr_itemp = data.indoor_temp;
			max_mtemp = min_mtemp = avr_mtemp = data.mirror_temp;
			max_wind = min_wind = avr_wind = data.wind_speed;
			max_pres = min_pres = avr_pres = data.pressure;
			max_hmd = min_hmd = avr_hmd = data.humidity;
			i = 0;
		}else{
			minmax(&min_otemp, &max_otemp, data.outdoor_temp);
			minmax(&min_itemp, &max_itemp, data.indoor_temp);
			minmax(&min_mtemp, &max_mtemp, data.mirror_temp);
			minmax(&min_wind, &max_wind, data.wind_speed);
			minmax(&min_pres, &max_pres, data.pressure);
			minmax(&min_hmd, &max_hmd, data.humidity);
			avr_otemp += data.outdoor_temp;
			avr_itemp += data.indoor_temp;
			avr_mtemp += data.mirror_temp;
			avr_wind += data.wind_speed;
			avr_pres += data.pressure;
			avr_hmd += data.humidity;
		}
	}
	if(d_len < 1){
		printf("<ha align=center>%s</h1>", _L(_s_noData_));
		return;
	}
	printf("<table>\n"
"<tr><th rowspan=\"2\"></th><th colspan=\"3\">%s</th><th rowspan=\"2\">%s</th>"
"<th rowspan=\"2\">%s</th><th rowspan=\"2\">%s</th></tr>\n"
"<tr><th>%s</th><th>%s</th><th>%s</th></tr>\n"
"<tr><td>min</td><td>%.2f</td><td>%.2f</td><td>%.2f</td><td>%.2f</td><td>%.2f</td><td>%.2f</td></tr>\n"
"<tr><td>max</td><td>%.2f</td><td>%.2f</td><td>%.2f</td><td>%.2f</td><td>%.2f</td><td>%.2f</td></tr>\n"
"<tr><td>avr</td><td>%.2f</td><td>%.2f</td><td>%.2f</td><td>%.2f</td><td>%.2f</td><td>%.2f</td></tr></table>\n"
		"%s: %d<p>\n",
		_L(_s_Temp_), _L(_s_WSpeed_), _L(_s_Pressure_), _L(_s_Humidity_),
		_L(_s_Ot_), _L(_s_It_), _L(_s_Mt_),
		min_otemp, min_itemp, min_mtemp, min_wind, min_pres, min_hmd,
		max_otemp, max_itemp, max_mtemp, max_wind, max_pres, max_hmd,
		avr_otemp/d_len, avr_itemp/d_len, avr_mtemp/d_len,
			avr_wind/d_len, avr_pres/d_len, avr_hmd/d_len,
		_L(_s_Monlen_), d_len);
		printf("<a href=\"%s?Save=1&Tstart=%d&Tend=%d&Stat=%d&Aver=%d\"'>%s</a>\n",
			SCRIPT_PATH, t_start, t_end, stat_mask, aver_interval, _L(_s_Save_file_));
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
	char *tok, *val, *par, *str;
	int stat = 0;
	str = calloc(MAX_QUERY_SIZE, 1);
	strncpy(str, qs, MAX_QUERY_SIZE);
	tok = strtok(str, "& \n");
	do{
		if((val = strchr(tok, '=')) == NULL) continue;
		*val++ = '\0';
		par = tok;
		if(strcasecmp(par, param)==0){
			if(strlen(val) > 0){
				stat = 1;
				strncpy(meaning, val, l);
				meaning[l-1] = 0;
			}
			break;
		}
	}while((tok = strtok(NULL, "& \n"))!=NULL);
	free(str);
	return stat;
}

void send_data(unsigned char stat_mask){
	int ii=0, j;
	struct timeval tv;
	struct tm ltime;
	time_t t_first=0, t_last=0;
	double otemp=0., itemp=0., mtemp=0., wind=0., pres=0., hum=0.;
	int ctr, status, statuses[5] = {0,0,0,0,0};
	char s_time[32];
	monit_d data;
	printf("Content-type: text/plain; charset=koi8-r\n\n");
	printf("OT - %s\nIT - %s\nMT - %s\nWS - %s\nP - %s\nH - %s\nS - %s\n\n",
		_L(_s_Otemp_), _L(_s_Itemp_), _L(_s_Mtemp_),
		_L(_s_WSpeed_), _L(_s_Pressure_), _L(_s_Humidity_),
		_L(_s_State_));
	printf("%s\t\tOT\tIT\tMT\tWS\tP\tH\tS\n", _L(_s_Date_));
	while(read(out_fd, &data, sizeof(data))){
		if(data.seconds < t_start) continue;
		if(data.seconds > t_end) break;
		if(!(stat_mask & (1 << data.status))) continue;
		if(t_first == 0) t_first = t_last = data.seconds;
		else if((data.seconds - t_first) >= aver_interval){ // накопили достаточно данных
			t_first = t_first/2 + t_last/2; // середина временного интервала
			ltime = *localtime(&t_first);//*localtime(&t_first);
			strftime(s_time, 32, "%d/%m/%Y, %H:%M:%S", &ltime);
			if(ii == 0) ii = 1;
			ctr = 0;
			for(j = 0; j<5; j++){
				if(statuses[j] > ctr){
					ctr = statuses[j];
					status = j;
				}
				statuses[j] = 0;
			}
			printf("%s\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%d\n",
						s_time,
						otemp/ii,
						itemp/ii,
						mtemp/ii,
						wind/ii,
						pres/ii,
						hum/ii,
						status);
			t_first = data.seconds;
			otemp=itemp=mtemp=wind=pres=hum=0.;
			ii = 0;
		}
		t_last = data.seconds;
		otemp += data.outdoor_temp;
		itemp += data.indoor_temp;
		mtemp += data.mirror_temp;
		wind += data.wind_speed;
		pres += data.pressure;
		hum += data.humidity;
		statuses[data.status]++;
		ii++;
	}
}

void fill_forms(const char* alert_message, const char S_flag){
	int shmid, i = 0;
	char tmp[16];
	unsigned char mask;
	int im_ht = 600;
	if(S_flag == 0){
		printf(Content_type);
		if(alert_message) printf("<div><h1 align=center>%s</h1></div>\n", alert_message);
		return;
	}
	if(get_qs_param(qs, "Stat", tmp, 16))
		mask = atoi(tmp);
	else mask = 0xff;
	if(get_qs_param(qs, "height", tmp, 16)){
		im_ht = atoi(tmp);
		if(im_ht < 100 || im_ht > 800) im_ht = 600;
	}
	if(Graph){
		if(get_qs_param(qs, "Gtype", tmp, 16)){
			i = atoi(tmp);
			if(i<SVG || i>GIF)
				i = GIF;
		}
		else i = GIF;
		sendSVG(i, mask, im_ht);
	}
	else{
		if(S_flag == 2) send_data(mask);
		else print_curvals(mask);
	}
}

void sendSVG(const int image_type, unsigned char stat_mask, int im_height){
	FILE *plot = NULL;
	char s_time[32];
	struct tm ltime;
	struct timeval tv;
	monit_d md, *data, *ptr;
	time_t time_interval;
	unsigned char Flag, Comma = 0;
	int twographs = 0;
	char *titles[] = {"T out", "T in", "T mir", "Wind", "Pres", "Hum"};
	char *command= "/Users/eddy/bin/gnuplot";
	char *types[] = {"svg", "jpeg", "gif"};
	char *contents[] = {"svg+xml", "jpeg", "gif"};
	int f, i, j, d_len, rb;
	data = calloc(1002, sizeof(monit_d)); // максимум тысяча точек
	ptr = data;
	printf("Content-type: image/%s\n\n", contents[image_type]);
	if(Graph < 1 || Graph > 63){
		fprintf(stderr, "Graph=%d, out of range\n", Graph);
		goto ret;
	}
	d_len = 0;
	//lseek(out_fd, 0, SEEK_SET);
	while(read(out_fd, &md, sizeof(md))){
//fprintf(stderr, "time=%d, t_start=%d, t_end=%d, d_len=%d, time_interval=%d\n", md.seconds, t_start, t_end, d_len, time_interval);
		if(md.seconds < t_start) continue;
		if(md.seconds > t_end) break;
		if(d_len == 0){
			t_start = md.seconds;
			time_interval = (t_end - t_start) / 300;
		}
		*ptr++ = md;
		if(++d_len > 300) break; // лишние значения
		t_start += time_interval;
//fprintf(stderr, ">>>time=%d, t_start=%d, t_end=%d, d_len=%d, time_interval=%d\n", md.seconds, t_start, t_end, d_len, time_interval);
	}
	close(f);
	if(d_len < 3){
		fprintf(stderr, "d_len=%d, error\n", d_len);
		goto ret;
	}
	plot = popen(command, "w");
	if(plot == NULL){
		perror("can't run gnuplot\n");
		goto ret;
	}
	//fprintf(plot, "set terminal %s size 800,%d font \"/usr/share/fonts/liberation/LiberationSans-Regular.ttf\"\nset xdata time\nset timefmt \"%%d/%%m-%%H:%%M\"\nset format x \"%%H:%%M\\n%%d/%%m\"\n",
	fprintf(plot, "set terminal %s size 800,%d\nset xdata time\nset timefmt \"%%d/%%m-%%H:%%M\"\nset format x \"%%H:%%M\\n%%d/%%m\"\n",
		types[image_type], im_height);
	if((Graph & 16) && (Graph & 32)){ // присутствуют и влажность, и давление
		twographs = 1;
		if(Graph & 15) // есть еще данные
			fprintf(plot, "set multiplot\nset origin 0,0\nset size 1,0.5\n");
	}
	else if(Graph & 48){ // присутствуют только влажность, либо только давление
		if(Graph & 15){ // выводим еще что-нибудь
				fprintf(plot, "set ytics nomirror\nset y2tics\nset ylabel \"degr C, m/s\"\n");
				if(Graph & 16)
					fprintf(plot, "set y2label \"mmHg\"\n"); // подпись для давления
				else
					fprintf(plot, "set y2label \"%%\"\n"); // подпись для влажности
		}
	}
	fprintf(plot, "plot ");
	for(j = 0; j < 4; j++){
		Flag = Graph & (1 << j);
		if(Flag){
			if(Comma) fprintf(plot, ",");
			Comma = 1;
			//w l smooth csplines
			fprintf(plot, " '-' using 1:2 w l lt %d title '%s'", j+1, titles[j]);
		}
	}
	if(!twographs){
		for(j = 4; j < 6; j++){
			if(Graph & (1 << j)){
				if(Comma) fprintf(plot, ",");
				Comma = 1;
				fprintf(plot, " '-' using 1:2 w l axes x1y2 lt %d title '%s'", j+1, titles[j]);
			}
		}
	}

	fprintf(plot, "\n");
	for(j = 0; j < 6; j++){
		Flag = Graph & (1 << j);
		if(twographs && j > 3) break;
		if(Flag){
			for(i = 0; i < d_len; i++){
				if(!(stat_mask & (1 << data[i].status))) continue;
				time_t X = (time_t)data[i].seconds;
				ltime = *localtime(&X);
				strftime(s_time, 32, "%d/%m-%H:%M", &ltime);
				fprintf(plot, "%s ", s_time);
				switch(Flag){
				case 1:  fprintf(plot, "%.2f\n", data[i].outdoor_temp);
					break;
				case 2:  fprintf(plot, "%.2f\n", data[i].indoor_temp);
					break;
				case 4:  fprintf(plot, "%.2f\n", data[i].mirror_temp);
					break;
				case 8:  fprintf(plot, "%.2f\n", data[i].wind_speed);
					break;
				case 16: fprintf(plot, "%.2f\n", data[i].pressure);
					break;
				case 32: fprintf(plot, "%.2f\n", data[i].humidity);
					break;
				}
			}
		fprintf(plot, "e\n");
		}
	}
	if(twographs){
		if(Graph & 15) // строим 2 графика
			fprintf(plot, "set origin 0,0.5\nset size 1,0.5\n");
		fprintf(plot, "set ylabel \"mmHg\"\nset y2label \"%%\"\n");
		fprintf(plot, "plot '-' using 1:2 w l lt 1 title 'Pres',");
		fprintf(plot, " '-' using 1:2 w l axes x1y2 lt 2 title 'Hum'\n");
		for(j = 0; j < 2; j++){
			for(i = 0; i < d_len; i++){
				if(!(stat_mask & (1 << data[i].status))) continue;
				time_t X = (time_t)data[i].seconds;
				ltime = *localtime(&X);
				strftime(s_time, 32, "%d/%m-%H:%M", &ltime);
				fprintf(plot, "%s ", s_time);
				switch(j){
					case 0: fprintf(plot, "%.2f\n", data[i].pressure); break;
					case 1: fprintf(plot, "%.2f\n", data[i].humidity); break;
				}
			}
			fprintf(plot, "e\n");
		}
	}
ret:
	if(plot){
		fflush(plot);
		pclose(plot);
	}
	free(data);
}

void find_starting_pos(){ // ищем в кэше смещение для начала поиска
	int cache_fd;
	Cache cache;
	if((cache_fd = open(CACHE_FILE, O_RDONLY)) < 0){
		printf(Content_type);
		printf("<h1>%s</h1>", _L(_s_Cant_open_cache_));
		exit(1);
	}
	while(read(cache_fd, &cache, sizeof(cache))){
		if(cache.time < t_start)
			starting_pos = cache.offset;
		else break;
	}
	close(cache_fd);
}

void quit(int status){
	if(buf) free(buf);
	if(out_fd) close(out_fd);
	exit(status);
}

char *switch_names(int i, char* ss){
	switch(i){
		case 0: ss = _L(_s_Otemp_); break;
		case 1: ss = _L(_s_Itemp_); break;
		case 2: ss = _L(_s_Mtemp_); break;
		case 3: ss = _L(_s_WSpeed_); break;
		case 4: ss = _L(_s_Pressure_); break;
		case 5:
		default: ss = _L(_s_Humidity_);
	}
	return ss;
}

char *format_time(time_t t, char *ss, int i){
	struct timeval tv;
	struct tm ltime = *localtime(&t);
	strftime(ss, i, "%d/%m/%Y, %H:%M", &ltime);
	return ss;
}

void print_string(time_t t1, time_t t2, int i){
	char s_t1[32], s_t2[32], *ss;
	printf("%s - %s (%s)<br>\n", format_time(t1, s_t1, 32),
		format_time(t2, s_t2, 32), switch_names(i, ss));
}

void show_extremums(int ch){
	int stat_mask = 0xFF, i, AndOr = 0, memsize, counter, first;
	char tmp[32], g = 0, l = 0, *ss;
	monit_d data;
	float Greater, Less;
	float extr[6], *meds[6]={NULL, NULL, NULL, NULL, NULL, NULL}; // экстремумы, медианы
	struct timeval tv;
	struct tm ltime;
	time_t t1[6], t2[6], t_first=0, t_last=0;

	printf(Content_type);
	if(get_qs_param(qs, "Stat", tmp, 16))
		stat_mask = atoi(tmp);
	if(stat_mask == 0) stat_mask = 0xFF;
	if(get_qs_param(qs, "Graph", tmp, 16))
		Graph = atoi(tmp);
	if(Graph == 0) Graph = 63;
	if(ch == 3){ // получение данных и проверка их корректности
		if(get_qs_param(qs, "AndOr", tmp, 16))
			AndOr = atoi(tmp);
		if((AndOr & 1) != AndOr) AndOr = 0;
		if(get_qs_param(qs, "Greater", tmp, 32)){
			Greater = atof(tmp);
			g = 1;
		}
		if(get_qs_param(qs, "Less", tmp, 32)){
			Less = atof(tmp);
			l = 1;
		}
		if(!(g | l) ||	// нет ни Greater, ни Less
			(AndOr && !(g & l))	// указано "И", но не хватает данных
			){
			printf("<h1>%s</h1>\n", _L(_s_No_Data_));
			return;
		}
		if(AndOr){
			if(Greater >= Less){
				printf("<h1>%s</h1>\n", _L(_s_G_mustbe_less_L_));
				return;
			}
		}
		else{
			if((g & l) && (Greater <= Less)){
				printf("<h1>%s</h1>\n", _L(_s_L_mustbe_less_G_));
				return;
			}
		}
	}
	if(aver_interval < 60) aver_interval = 60; // медианное усреднение по крайней мере за минуту
	switch(ch){
		case 0:  ss = _L(_s_Modes_); break;
		case 1:  ss = _L(_s_Max_data_); break;
		case 2:  ss = _L(_s_Min_data_); break;
		case 3:
		default: ss = _L(_s_Diapazon_);
	}
	printf("<h2>%s ", ss);
	if(ch == 3){
		if(g){
			printf("%s %.1f ", _L(_s_Gtr_), Greater);
			if(AndOr) printf("%s ", _L(_s_And_));
			else if(l) printf("%s ", _L(_s_Or_));
		}
		if(l) printf("%s %.1f", _L(_s_Less_), Less);
	}
	printf("</h2>\n");
	memsize = aver_interval / TIMEINTERVAL + 3;
	if(ch) for(i = 0; i < 6; i++)
		if(Graph & (1<<i)) meds[i] = (float*)calloc(memsize, sizeof(float));
	counter = 0;
	if(ch == 1 || ch == 2) first = 1;
	else for(i=0; i<6; i++) t1[i] = 0;
	while(read(out_fd, &data, sizeof(data))){
		if(data.seconds < t_start) continue;
		if(ch == 0){
			i = (int)(TIMEINTERVAL + 0.5);
			t1[5] += i;
			t1[data.status] += i;
		}
		if(!(stat_mask & (1 << data.status))){
			if(ch == 0 && t_first){
				t_last = data.seconds;
				printf("%s - ",  format_time(t_first, tmp, 32));
				printf("%s<br>\n", format_time(t_last, tmp, 32));
				t_first = 0;
			}
			continue;
		}
		if(ch == 0){ // оценка временнЫх интервалов
			if(t_first == 0) t_first = t_last = data.seconds;
			continue;
		}
		if(t_first == 0) t_first = t_last = data.seconds;
		else if((data.seconds - t_first) >= aver_interval || counter > memsize || data.seconds > t_end){
			t_first = t_first/2 + t_last/2; // середина временного интервала
			for(i=0; i<6; i++)
				if(Graph & (1<<i)) meds[i][0] = quick_select(meds[i], counter);
			if(first){
				first = 0;
				for(i=0; i<6; i++){
					if(Graph & (1<<i)){
						extr[i] = meds[i][0];
						t1[i] = t_first;
					}
				}
			}
			else if(ch != 3){
				for(i=0; i<6; i++)
					if(Graph & (1<<i)){
						if(ch == 1){ // ищем максимумы
							if(meds[i][0] > extr[i]){
								extr[i] = meds[i][0];
								t1[i] = t_first;
							}
						}
						else{ // ch = 2 - ищем минимумы
							if(meds[i][0] < extr[i]){
								extr[i] = meds[i][0];
								t1[i] = t_first;
							}
						}
					}
			}
			else{ // ищем периоды (ch == 3)
				for(i=0; i<6; i++)
					if(Graph & (1<<i)){
						if(AndOr){ // ищем больше Greater и меньше Less
							if(meds[i][0] > Greater && meds[i][0] < Less)
								if(t1[i] == 0) t1[i] = t2[i] = t_first;
								else t2[i] = t_first;
							else if(t1[i] != 0){
								print_string(t1[i], t2[i], i);
								t1[i] = 0;
							}
						}
						else{ // вариант с "или"
							if( (g && meds[i][0] > Greater) || (l && meds[i][0] < Less) )
								if(t1[i] == 0) t1[i] = t2[i] = t_first;
								else t2[i] = t_first;
							else if(t1[i] != 0){ // выводим данные
								print_string(t1[i], t2[i], i);
								t1[i] = 0;
							}
						}
					}
			}
			t_first = data.seconds;
			counter = 0;
		}
		if(data.seconds > t_end) break;
		t_last = data.seconds;
		if(Graph & 1)  meds[0][counter] = data.outdoor_temp;
		if(Graph & 2)  meds[1][counter] = data.indoor_temp;
		if(Graph & 4)  meds[2][counter] = data.mirror_temp;
		if(Graph & 8)  meds[3][counter] = data.wind_speed;
		if(Graph & 16) meds[4][counter] = data.pressure;
		if(Graph & 32) meds[5][counter] = data.humidity;
		counter++;
	}
	if(ch == 1 || ch == 2)
		for(i=0; i<6; i++){
			if(Graph & (1<<i))
				printf("%s %s: %.1f<br>\n", format_time(t1[i], tmp, 32),
					switch_names(i, ss), extr[i]);
		}
	if(ch == 0){
		printf("<h2>%s</h2>\n", _L(_s_Mode_Times_));
		inline void fmtprnt(char **str, time_t tm){
			printf("%s: %d %s (%.2f %s)<br>\n", _L(str), tm, _L(_s_seconds_),
				((float)tm)/86400., _L(_s_days_));
		}
		fmtprnt(_s_Stopped_, t1[0]);
		fmtprnt(_s_Guiding_, t1[1]);
		fmtprnt(_s_Ready_, t1[2]);
		fmtprnt(_s_Other_, t1[3]);
		fmtprnt(_s_Vopen_, t1[4]);
		fmtprnt(_s_Total_, t1[5]);
	}
	for(i=0; i<6; i++) if(meds[i]) free(meds[i]);
}

int main(){
	char *ptr, tmp[128];
	setbuf(stdout, NULL);
	buf = (char*)calloc(MAX_QUERY_SIZE, 1);
	ptr = getenv("HTTP_ACCEPT_LANGUAGE");
	if(ptr) if(strncmp(ptr, "ru", 2) == 0) Lang = 0; // используем русский
	qs = get_qs(buf, MAX_QUERY_SIZE);
	if(!qs){
		fill_forms(_L(_s_no_QS_), 0);
		quit(0);
	}
	if(get_qs_param(qs, "Visor", tmp, 128)){	// открываем/закрываем забрало
		int shmid, param = atoi(tmp);
		shmid = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0666);
		printf(Content_type);
		if(shmid < 0) printf(_L(_s_noVisor_));
		else{
			Visor = (int*) shmat(shmid, NULL, 0);
			if(Visor){
				if(param > 0) *Visor = !(*Visor); // param >0 - смена состояния
			}
			else printf(_L(_s_noVisor_));
		}
		printf("%s:<br><font color='%s'>%s</font><p  style='margin-top: 25px;'>%s:<br>", _L(_s_CurVstat_),
			*Visor?"green":"red", *Visor?_L(_s_Vopen_):_L(_s_Vclose_), _L(_s_ChVstat_));
		printf("<input type='button' OnClick='sendrequest(\"Visor=1\", visor);' "
			"value=\"%s\">", *Visor ? _L(_s_Vclose_) : _L(_s_Vopen_) );
		quit(0);
	}
	if(!get_qs_param(qs, "Tstart", tmp, 128)){
		if(!get_qs_param(qs, "Graph", tmp, 16))
		fill_forms(_L(_s_no_tstart_), 0);
		quit(0);
	}
	t_start = atoi(tmp);
	if(get_qs_param(qs, "Tend", tmp, 128))
		t_end = atoi(tmp);
	else
		t_end = time(NULL);
	if(t_end <= t_start){
		if(!get_qs_param(qs, "Graph", tmp, 16))
		fill_forms(_L(_s_Bad_date_), 0);
		quit(1);
	}
	if(get_qs_param(qs, "Aver", tmp, 128)){
		aver_interval = atoi(tmp);
		if(aver_interval < 1) aver_interval = 1;
	}
	find_starting_pos();
	if((out_fd = open(OUT_FILE, O_RDONLY)) < 0){
		exit(1);
	}
	lseek(out_fd, starting_pos, SEEK_SET);
	if(get_qs_param(qs, "Select", tmp, 16)){
		int choice = atoi(tmp);
		if(choice < 0 || choice > 3){
			fill_forms(NULL, 1);
			quit(0);
		}
		show_extremums(choice);
		quit(0);
	}
	if(get_qs_param(qs, "Graph", tmp, 16)){
		Graph = atoi(tmp);
		if(Graph == 0) Graph = -1;
		fill_forms(NULL, 1);
		quit(0);
	}
	if(get_qs_param(qs, "Save", tmp, 16)){
		fill_forms(NULL, 2); // сохранить данные
		quit(0);
	}
	else{
		fill_forms(NULL, 1); // показать экстремальные значения
		quit(0);
	}
	quit(0);
}

