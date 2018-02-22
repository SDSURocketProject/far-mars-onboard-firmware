/*
 * infogram.h
 *
 * Created: 1/7/2018 6:57:25 PM
 *  Author: Jake Carver
 */ 


#ifndef INFOGRAM_H_
#define INFOGRAM_H_


enum info_header
{
	info_time,
	info_system,
	info_valves,
	info_pres,
	info_temp,
	info_imu,
	info_quan,
	info_acc,
	info_angv,
	info_mag,
	info_warn,

}info_header;

//Stores the current system state
struct infogram
{
	//System State
	bool power_good,
	bool radio_on,
	bool wired_on,
	bool charging,
	bool battery_low,
	bool log_media_present,
	uint8_t charge,
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
	//Four point quaternion output, quaternion units
	float quan_a,
	float quan_b,
	float quan_c,
	float quan_d,
	//Three axis acceleration vector,  m/s^2
	float acc_x,
	float acc_y,
	float acc_z,
	//Three axis angular velocity vector,  rad/s
	float ang_i,
	float ang_j,
	float ang_k,
	//Three axis of magnetic field, micro Tesla
	float mag_x,
	float mag_y,
	float mag_z,
	//Warnings - NEED TO NAME
	
}infogram;

void infogram_to_json(infogram * , info_header);

#endif /* INFOGRAM_H_ */