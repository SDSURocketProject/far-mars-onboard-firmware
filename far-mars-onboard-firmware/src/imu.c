/*
 * imu.c
 *
 * Created: 3/27/2018 7:21:42 PM
 *  Author: Jake Carver
 */ 

 #include "imu.h"



struct i2c_master_module i2c_master_instance;

struct i2c_master_packet packet = {
	.address     = SLAVE_ADDRESS,
	.data_length = 0,
	.data        = NULL,
	.ten_bit_address = false,
	.high_speed      = false,
	.hs_master_code  = 0x0,
};

struct i2c_master_packet reg_addr_packet = {
	.address     = SLAVE_ADDRESS,
	.data_length = 1,
	.data        = NULL,
	.ten_bit_address = false,
	.high_speed      = false,
	.hs_master_code  = 0x0,
};

struct bno055_t myBNO;


void configure_i2c_master(void)
{
	/* Initialize config structure and software module. */
	struct i2c_master_config config_i2c_master;
	i2c_master_get_config_defaults(&config_i2c_master);
	/* Change buffer timeout to something longer. */
	config_i2c_master.buffer_timeout = 10000;
	config_i2c_master.pinmux_pad0    = PINMUX_PA16C_SERCOM1_PAD0;
	config_i2c_master.pinmux_pad1    = PINMUX_PA17C_SERCOM1_PAD1;
	/* Initialize and enable device with config. */
	i2c_master_init(&i2c_master_instance, SERCOM1, &config_i2c_master);
	i2c_master_enable(&i2c_master_instance);
}


void bno_api_init(void)
{
	configure_i2c_master();

	//delay_ms(1);//BNO requires ~700 ms startup delay or it wont work, per documentation

	myBNO.bus_read = BNO055_I2C_bus_read;
	myBNO.bus_write = BNO055_I2C_bus_write;
	myBNO.delay_msec = BNO055_delay_msek;
	myBNO.dev_addr = SLAVE_ADDRESS;
	//delay_ms(700);
	bno055_init(&myBNO);

	//bno055_set_sys_rst(0);
	

	//delay_ms(700);

	bno055_set_operation_mode(BNO055_OPERATION_MODE_NDOF);
	
	
}


s8 BNO055_I2C_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
{

	
	char extd_data[2] = {reg_addr, *reg_data};
		
		
	packet.address = dev_addr;
	packet.data_length = 2;
	packet.data = extd_data;	

	uint16_t timeout = 10;


	//reg_addr_packet.address = dev_addr;
	//reg_addr_packet.data = &reg_addr;
	/*
	while (i2c_master_write_packet_wait(&i2c_master_instance, &reg_addr_packet) != STATUS_OK) 
	{
		// Increment timeout counter and check if timed out.
		if (timeout++ == TIMEOUT) {
			return -1;
		}
	}
	*/
	while (i2c_master_write_packet_wait(&i2c_master_instance, &packet) != STATUS_OK) 
	{
		/* Increment timeout counter and check if timed out. */
		if (timeout++ == TIMEOUT) {
			return -1;
		}
	}

	return 0;
}

 /*	\Brief: The API is used as I2C bus read
 *	\Return : Status of the I2C read
 *	\param dev_addr : The device address of the sensor
 *	\param reg_addr : Address of the first register,
 *  will data is going to be read
 *	\param reg_data : This data read from the sensor,
 *   which is hold in an array
 *	\param cnt : The no of byte of data to be read
 */
s8 BNO055_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
{
	packet.address = dev_addr;
	packet.data_length = cnt;
	packet.data = reg_data;

	uint16_t timeout = 10;

	reg_addr_packet.address = dev_addr;
	reg_addr_packet.data = &reg_addr;

	while (i2c_master_write_packet_wait(&i2c_master_instance, &reg_addr_packet) != STATUS_OK)
	{
		/* Increment timeout counter and check if timed out. */
		if (timeout++ == TIMEOUT) {
			return -1;
		}
	}

	while (i2c_master_read_packet_wait(&i2c_master_instance, &packet) != STATUS_OK)
	{
		/* Increment timeout counter and check if timed out. */
		if (timeout++ == TIMEOUT) {
			return -1;
		}
	}

	return 0;
}


void BNO055_delay_msek(u32 msek)
{
	delay_ms(msek);
}