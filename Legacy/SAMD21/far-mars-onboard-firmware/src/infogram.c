/*
 * infogram.c
 *
 * Created: 1/8/2018 3:29:54 PM
 *  Author: Jake Carver
 */ 

/*
#include "infogram.h"

void infogram_to_json(infogram * state, info_header info_type, char * buffer)
{
	switch(info_type)
	{
		case info_header::info_time:
			sprintf(buffer, "NOT IMPLEMENTED");
			break;
		case info_header::info_system:
			sprintf(buffer, "{'sys':{'radio_on':%d,'wire_on':%d,'charging':%d,'battery_low':%d,'sd_card':%d}}"
			, state->radio_on, state->wired_on, state->charging, state->battery_low, state->log_media_present);
			break;
		case info_header::info_valves:
			sprintf(buffer, "NOT IMPLEMENTED");
			break;
		case info_header::info_pres:
			sprintf(buffer, "{'pres':{'ch4':%d,'lox':%d,'he':%d,'cham':%d}}"
			, state->ch4_pres, state->lo2_pres, state->he_pres, state->cham_pres);
			break;
		case info_header::info_temp:
			sprintf(buffer, "{'temp':{'cpu':%d,'imu':%d}}"
			, state->temp_cpu, state->temp_imu);
			break;
		case info_header::info_quan:
			sprintf(buffer, "{'quan':{'a':%f,'b':%f,'c':%f,'d':%f}}"
			, state->quan_a, state->quan_b, state->quan_c, state->quan_d);
			break;
		case info_header::info_acc:
			sprintf(buffer, "{'acc':{'x':%f,'y':%f,'z':%f}}"
			, state->acc_x, state->acc_y, state->acc_z);
			break;
		case info_header::info_angv:
			sprintf(buffer, "{'angv':{'i':%f,'j':%f,'k':%f}}"
			, state->ang_i, state->ang_j, state->ang_k);
			break;
		case info_header::info_mag:
			sprintf(buffer, "{'mag':{'x':%f,'y':%f,'z':%f}}"
			, state->mag_x, state->mag_y, state->mag_z);
			break;
		case info_header::info_warn:
			sprintf(buffer, "NOT IMPLEMENTED");
			break;
	}
}
*/