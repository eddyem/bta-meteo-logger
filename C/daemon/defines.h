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

#define TIMEINTERVAL	9.99 // �������� ���������� ������ (� ��������)
#define LOG_FILE		"/home/eddy/_data/.temp_daemon" // ���� � ������
#define	CACHE_FILE		"/home/eddy/_data/.cache" // ��������� ����
#define OUT_FILE		"/home/eddy/_data/.out" // ���� ����������
#define PID_FILE		"/home/eddy/_data/.pid" // PID
#define A_PATH			"/home/eddy/_data"
#define PROC_BASE		"/proc"
#define SHM_KEY			1234567890
#define CACHE_EVERY_PWR	10	// �������� ������ 2^CACHE_EVERY_PWR ������
#define tell(fd)		lseek(fd, 0, SEEK_CUR)

enum telescope_status{
	STOP,		// 0 - �������� ��������
	GUIDING,	// 1 - �������
	READY,		// 2 - ��� �����������, ������� ����
	OTHER,		// 3 - ��������� ��� ������ ����������� ���������/������
	OPEN		// 4 - ������� �������, ���� ����������
				// 5 - ��������������� ��� ���������� �������
};
typedef enum telescope_status Status;
typedef float data_type;
#pragma pack(4)
struct monitoring_data{
	uint32_t seconds;		// ����� � ��������
	data_type outdoor_temp;	// ������� ����������� �� TIMEINTERVAL �������
	data_type indoor_temp;	// -//- ����������� ������
	data_type mirror_temp;	// -//- ����������� �������
	data_type wind_speed;	// ������������ �� TIMEINTERVAL �������� �����
	data_type pressure;	// ��������� �������� �������� (��.��.��)
	data_type humidity;	// -//- ���������
/*	data_type sigma_ot;		// ����������� ���������� ������� �����������
	data_type sigma_it;		// -//- ���������� �����������
	data_type sigma_mt;		// -//- ����������� �������
	data_type sigma_w;		// -//- �������� �����
	data_type sigma_p;
	data_type sigma_h;
	data_type avr_ot;	// ������� �� TIMEINTERVAL �������� ������� �����������
	data_type avr_it;	// ...
	data_type avr_mt;
	data_type avr_w;
	data_type avr_p;
	data_type avr_h;*/
	Status status;	// ��������� ���������
};
typedef struct monitoring_data monit_d;

#pragma pack(4)
struct _cache{			// ��������� ��� �������� ������ � ����
	uint32_t time;		// ����� � �������� � 1.01.1970-0:0
	uint64_t offset;	// �������� � ���-�����
};
typedef struct _cache Cache;
int *Visor = NULL;			// ��������� �������
#endif

