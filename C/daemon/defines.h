#ifndef __DEFINES_H__
#define __DEFINES_H__
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <stdint.h>

#ifndef __CLIENT_C__
#include "bta_shdata.h"
#endif

#define WARN(...) do{fprintf(stderr, __VA_ARGS__);}while(0)

#define TIMEINTERVAL	599.999 // интервал усреднения данных (в секундах)
#define LOG_FILE		"/home/eddy/_data/.temp_daemon" // файл с логами
#define	CACHE_FILE		"/home/eddy/_data/.cache" // индексный файл
#define OUT_FILE		"/home/eddy/_data/.out" // сама информация
#define PID_FILE		"/home/eddy/_data/.pid" // PID
#define A_PATH			"/home/eddy/_data"
#define PROC_BASE		"/proc"
#define SHM_KEY			1234567890
#define CACHE_EVERY_PWR	10	// кэшируем каждую 2^CACHE_EVERY_PWR запись
#define tell(fd)		lseek(fd, 0, SEEK_CUR)

enum telescope_status{
	STOP,		// 0 - телескоп выключен
	GUIDING,	// 1 - ведение
	READY,		// 2 - все остановлено, питание есть
	OTHER,		// 3 - наведение или прочее перемещение телескопа/купола
	OPEN		// 4 - забрало открыто, идут наблюдения
				// 5 - ЗАРЕЗЕРВИРОВАНО для суммарного времени
};
typedef enum telescope_status Status;
typedef float data_type;
#pragma pack(4)
struct monitoring_data{
	uint32_t seconds;		// время в секундах
	data_type outdoor_temp;	// медиана температуры за TIMEINTERVAL снаружи
	data_type indoor_temp;	// -//- температура внутри
	data_type mirror_temp;	// -//- температура зеркала
	data_type wind_speed;	// максимальная за TIMEINTERVAL скорость ветра
	data_type pressure;	// медианное значение давления (мм.рт.ст)
	data_type humidity;	// -//- влажности
/*	data_type sigma_ot;		// стандартное отклонение внешней температуры
	data_type sigma_it;		// -//- внутренней температуры
	data_type sigma_mt;		// -//- температуры зеркала
	data_type sigma_w;		// -//- скорости ветра
	data_type sigma_p;
	data_type sigma_h;
	data_type avr_ot;	// среднее за TIMEINTERVAL значение внешней температуры
	data_type avr_it;	// ...
	data_type avr_mt;
	data_type avr_w;
	data_type avr_p;
	data_type avr_h;*/
	Status status;	// состояние телескопа
};
typedef struct monitoring_data monit_d;

#pragma pack(4)
struct _cache{			// структура для хранения данных в кэше
	uint32_t time;		// время в секундах с 1.01.1970-0:0
	uint64_t offset;	// смещение в лог-файле
};
typedef struct _cache Cache;
int *Visor = NULL;			// состояние забрала
#endif

