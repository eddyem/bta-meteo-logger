#include "bta_shdata.h"

inline double sec2deg(double angle){
	return angle / 3600;
}

int main(int argc, char** argv){
	get_shm_block( &sdat, ClientSide);
	if(!check_shm_block(&sdat)) return -1;
	printf("Access-Control-Allow-Origin: http://ishtar.sao.ru\n"
		"Acess-Control-Allow-Methods: POST\nContent-type:multipart/form-data\n\n");
	printf("telA=%.2f telZ=%.2f domeA=%.2f\n", sec2deg(val_A), sec2deg(val_Z), sec2deg(val_D));
}

