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
#include <getopt.h>
#include <stdarg.h>
#include <stdlib.h>
#include "defines.h"
#include "lang.h"

#define Julian(x) ((double)x/86400. + 2440587.5)

extern const char *__progname;

char *db_path = A_PATH;
off_t starting_pos = 0LL;
typedef struct shm_data shm_d;
unsigned char Lang = 1; // английский
time_t t_start = 0, t_end = 0;
time_t aver_interval = 1;
int force_dbpath = 0;
int out_fd;
int only_extr = 0;

inline void minmax(float *min, float *max, float param){
	if(param > *max) *max = param;
	else if(param < *min) *min = param;
}

void send_data(unsigned char stat_mask){
	int ii=0, j;
	uint64_t data_amount = 0;
	struct tm ltime;
	time_t t_first=0, t_last=0;
	double otemp=0., itemp=0., mtemp=0., wind=0., pres=0., hum=0.;
	double otmax=-1e6,otmin=1e6,itmax=-1e6,itmin=1e6,mtmax=-1e6,mtmin=1e6,windmax=-1e6,windmin=1e6;
	int ctr, status, statuses[5] = {0,0,0,0,0};
	char s_time[32];
	monit_d data;
	if(!only_extr){
		printf("OT - %s\nIT - %s\nMT - %s\nWS - %s\nP - %s\nH - %s\nS - %s\n\n",
			_L(_s_Otemp_), _L(_s_Itemp_), _L(_s_Mtemp_),
			_L(_s_WSpeed_), _L(_s_Pressure_), _L(_s_Humidity_),
			_L(_s_State_));
		printf("%s\t\tJulian date\tOT\tIT\tMT\tWS\tP\tH\tS\n", _L(_s_Date_));
	}
	while(read(out_fd, &data, sizeof(data)) > 0){
		time_t X = (time_t) data.seconds;
		//printf("time: %zd (tstart: %zd, tend: %zd) -- %s", data.seconds, t_start, t_end, ctime(&X));
		if(X < t_start) continue;
		if(X > t_end) break;
		if(!(stat_mask & (1 << data.status))) continue;
		data_amount++;
		if(!only_extr){
		if(t_first == 0) t_first = t_last = X;
		else if((X - t_first) >= aver_interval){ // накопили достаточно данных
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
			printf("%s\t%.6f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%d\n",
						s_time,
						Julian(t_first),
						otemp/ii,
						itemp/ii,
						mtemp/ii,
						wind/ii,
						pres/ii,
						hum/ii,
						status);
			t_first = X;
			otemp=itemp=mtemp=wind=pres=hum=0.;
			ii = 0;
		}
		t_last = X;
		otemp += data.outdoor_temp;
		itemp += data.indoor_temp;
		mtemp += data.mirror_temp;
		wind += data.wind_speed;
		pres += data.pressure;
		hum += data.humidity;
		statuses[data.status]++;
		ii++;
		}
		if(otmax < data.outdoor_temp) otmax = data.outdoor_temp;
		if(otmin > data.outdoor_temp) otmin = data.outdoor_temp;
		if(itmax < data.indoor_temp) itmax = data.indoor_temp;
		if(itmin > data.indoor_temp) itmin = data.indoor_temp;
		if(mtmax < data.mirror_temp) mtmax = data.mirror_temp;
		if(mtmin > data.mirror_temp) mtmin = data.mirror_temp;
		if(windmax < data.wind_speed) windmax = data.wind_speed;
		if(windmin > data.wind_speed) windmin = data.wind_speed;
	}
	if(!data_amount){
		printf("No data found!\n");
		return;
	}
	printf("\n\nExtremal values (min / max):\n");
	printf("%s: %g / %g\n", _L(_s_Otemp_), otmin, otmax);
	printf("%s: %g / %g\n", _L(_s_Itemp_), itmin, itmax);
	printf("%s: %g / %g\n", _L(_s_Mtemp_), mtmin, mtmax);
	printf("%s: %g / %g\n", _L(_s_WSpeed_),windmin, windmax);
}

void find_starting_pos(){ // ищем в кэше смещение для начала поиска
	int cache_fd;
	Cache cache;
	if(force_dbpath){
		char cname[256];
		snprintf(cname, 255, "%s/.cache", db_path);
		printf("using cache file %s\n", cname);
		cache_fd = open(cname, O_RDONLY);
	}else{
		if((cache_fd = open(CACHE_FILE, O_RDONLY)) < 0)
			cache_fd = open("./cache", O_RDONLY);
	}
	if(cache_fd < 0){
		WARN(_L(_s_Cant_open_cache_));
		exit(1);
	}
	while(read(cache_fd, &cache, sizeof(cache)) > 0){
		//time_t X = (time_t) cache.time;
		//printf("time: %zd -- %s", cache.time, ctime(&X));
		if(cache.time < t_start)
			starting_pos = cache.offset;
		else break;
	}
	//printf("pos: %zd \n", starting_pos);
	close(cache_fd);
}

time_t get_date(char *line){
	time_t date = time(NULL);
	struct tm time_, time_now;
	time_now = *localtime(&date);
	time_.tm_sec = 0;
	if(sscanf(line, "%d/%d/%d-%d:%d", &time_.tm_mday, &time_.tm_mon, &time_.tm_year,
		&time_.tm_hour, &time_.tm_min) == 5){time_.tm_mon -= 1;}
	else if(!strchr(line, ':') && sscanf(line, "%d/%d/%d", &time_.tm_mday, &time_.tm_mon, &time_.tm_year) == 3){
		date = -1; time_.tm_mon -= 1;}
	else if(!strchr(line, ':') && sscanf(line, "%d/%d", &time_.tm_mday, &time_.tm_mon) == 2){
		date = -1; time_.tm_mon -= 1; time_.tm_year = time_now.tm_year;}
	else if(sscanf(line, "%d:%d", &time_.tm_hour, &time_.tm_min) == 2){
		time_.tm_year = time_now.tm_year; time_.tm_mon = time_now.tm_mon;
		time_.tm_mday = time_now.tm_mday;}
	else if(!strchr(line, ':') && !strchr(line, '/') && !strchr(line, '.') && !strchr(line, '-')
			&& sscanf(line, "%d", &time_.tm_mon) == 1){
		date = -1; time_.tm_mon -= 1; time_.tm_year = time_now.tm_year;
		time_.tm_mday = 1;}
	else{
		printf("\nWrong datetime format!\n");
		printf("Formats: D/M/Y-hh:mm, D/M/Y, hh:mm, D/M, M\n");
		exit(1);
	}
	if(date == -1){
		time_.tm_hour = 0;
		time_.tm_min = 0;
	}
	if(time_.tm_mon > 11 || time_.tm_mon < 0){
		printf("\nMonth should be in 1..12\n");
		exit(2);
	}
	if(time_.tm_mday > 31 || time_.tm_mday < 1){
		printf("\nDate should be in 1..31, %d\n", time_.tm_mday);
		exit(3);
	}
	if(time_.tm_year > 1900) time_.tm_year -= 1900;
	else if(time_.tm_year > -1 && time_.tm_year < 100) time_.tm_year += 100;
	else if(time_.tm_year < 0 || time_.tm_year > 200){
		printf("\nBad year format %d\n", time_.tm_year);
		exit(4);
	}
	if(time_.tm_hour > 23 || time_.tm_hour < 0){
		printf("\nTime should be in 0..23\n");
		exit(5);
	}
	if(time_.tm_min > 59 || time_.tm_min < 0){
		printf("\nMinutes should be in 0..59\n");
		exit(6);
	}
	date = mktime(&time_);
	return date;
}

void print_dates(){
	char s_time[32];
	inline char *mktm(uint32_t t){
		time_t X = (time_t) t;
		struct tm ltime = *localtime(&X);
		strftime(s_time, 32, "%d/%m/%Y, %H:%M:%S", &ltime);
		return s_time;
	}
	monit_d data;
	if(read(out_fd, &data, sizeof(data)) != sizeof(data)){
		WARN("Error reading datafile!\n");
		exit(1);
	}
	printf("Date of first record in database: %s\n", mktm(data.seconds));
	lseek(out_fd, -sizeof(data), SEEK_END);
	if(read(out_fd, &data, sizeof(data)) != sizeof(data)){
		WARN("Error reading datafile!\n");
		exit(1);
	}
	printf("Date of last record in database: %s\n", mktm(data.seconds));
	exit(0);
}

unsigned char get_modemask(const char *optarg){
	unsigned char mode = 0;
	char c;
	while((c = *optarg++)){
		switch (c){
			case 's': case 'S':
				mode |= 1;
			break;
			case 'g': case 'G':
				mode |= 2;
			break;
			case 'r': case 'R':
				mode |= 4;
			break;
			case 'u': case 'U':
				mode |= 8;
			break;
			case 'o': case 'O':
				mode |= 16;
			break;
			default:
				WARN("Bad mode: should be sgruo\n");
				exit(1);
		}
	}
	return mode;
}

void usage(char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	printf("\n");
	if (fmt != NULL){
		vprintf(fmt, ap);
		printf("\n\n");
	}
	va_end(ap);
	printf("Usage:\t%s [options]\n",
		__progname);
	printf("\tOptions:\n");
	printf("\t-d, --dbpath\t\tPath to database files\n");
	printf("\t-e, --tend\t\tEnding time (D/M/Y-hh:mm)\n");
	printf("\t-h, --help\t\tShow this help\n");
	printf("\t-i, --averint\t\tTime interval for data averaging (in seconds)\n");
	printf("\t-l, --lookout\t\t Show date interval of stored data\n");
	printf("\t-m, --mode\t\tTelescope mode (sgruo - Stopped, Guiding, Ready, Unknown, Opened)\n");
	printf("\t-s, --tstart\t\tStarting time (D/M/Y-hh:mm)\n");
	printf("\t-X, --extremal\t\tOnly show extremal values\n");
	exit(0);
}

int main(int argc, char **argv){
	unsigned char mask = 0;
	struct option long_options[] = {
		{"dbpath",		1,	0,	'd'},
		{"tend",		1,	0,	'e'},
		{"help",		0,	0,	'h'},
		{"averint",		1,	0,	'i'},
		{"lookout",		0,	0,	'l'},
		{"mode",		1,	0,	'm'},
		{"tstart",		1,	0,	's'},
		{"extremal",		0,	0,	'X'}
	};
	char short_options[] = "d:e:hi:lm:s:X";
	int lookout = 0;
	while (1){
		int opt;
		if((opt = getopt_long(argc, argv, short_options,
			long_options, NULL)) == -1) break;
		switch(opt){
			case 'd':
				db_path = strdup(optarg);
				force_dbpath = 1;
			break;
			case 'e':
				t_end = get_date((char*)optarg);
			break;
			case 'h':
				usage(NULL);
			break;
			case 'i':
				aver_interval = atoi((char*)optarg);
				if(aver_interval < 1) aver_interval = 1;
			break;
			case 'l':
				lookout = 1;
			break;
			case 'm':
				mask = get_modemask(optarg);
			break;
			case 's':
				t_start = get_date((char*)optarg);
			break;
			case 'X':
				only_extr = 1;
			break;
			default:
				usage("Unknown argument");
		}
	}
	if(argc != optind) usage("Too many arguments");
	if(t_end < 1) t_end = time(NULL);
	if(t_end <= t_start){
		WARN("%s\n", _L(_s_Bad_date_));
		return(1);
	}
	if(mask == 0) mask = 0xff;
	if(force_dbpath){
		char cname[256];
		snprintf(cname, 255, "%s/.out", db_path);
		printf("using DB file %s\n", cname);
		out_fd = open(cname, O_RDONLY);
	}else{
		if((out_fd = open(OUT_FILE, O_RDONLY)) < 0)
			out_fd = open("./out", O_RDONLY);
	}
	if(out_fd < 0){
		WARN(_L(_s_Cant_open_data_));
		return(1);
	}
	if(lookout) print_dates();
	find_starting_pos();
	lseek(out_fd, starting_pos, SEEK_SET);
	send_data(mask);
	return(0);
}

