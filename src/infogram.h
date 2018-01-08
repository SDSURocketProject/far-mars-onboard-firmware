/*
 * infogram.h
 *
 * Created: 1/7/2018 6:57:25 PM
 *  Author: Jake Carver
 */ 


#ifndef INFOGRAM_H_
#define INFOGRAM_H_


struct infogram
{
	//System time data from rtc
	int32_t time_hrs,
	int32_t time_mins,
	int32_t time_secs,
	int32_t time_mils,
	//System State
	bool power_good,
	bool radio_on,
	bool wired_on,
	bool charging,
	bool battery_low,
	bool log_media_present,
	int32_t charge,
	//Valve States - NEED TO NAME
	bool valve0_open,
	bool valve1_open,
	bool valve2_open,
	bool valve3_open,
	bool valve4_open,	 
	//Pressure data in PSI
	int32_t ch4_pres,	//Methane
	int32_t lo2_pres,	//Liquid Oxygen
	int32_t he_pres,	//Helium
	int32_t cham_pres,	//Chamber
	//Temperature from various sources, degrees Celsius
	int32_t temp_cpu,
	int32_t temp_imu,
	//Four point quaternion output, Quanterniun units
	int32_t quan_a,
	int32_t quan_b,
	int32_t quan_c,
	int32_t quan_d,
	//Three axis of gravitational acceleration,  m/s^2
	int32_t grav_x,
	int32_t grav_y,
	int32_t grav_z,
	//Three axis of magnetic field, micro Tesla
	int32_t mag_x,
	int32_t mag_y,
	int32_t mag_z,
	//Warnings - NEED TO NAME
	
}infogram;

char * infogram_to_json(infogram * );

#endif /* INFOGRAM_H_ */