#include <raspi_batmon/batmon_driver.h>
batmon_driver::batmon_driver(int adapter_nr, int addr) {
    file_num = adapter_nr;
    this->addr = addr;
}
__s32 batmon_driver::read_16bit_word(int reg) {
    int file;
    char filename[20];
    //------ Open I2C BUS ------

    snprintf(filename, 19, "/dev/i2c-%d", file_num); // "/dev/i2c-3"

    file = open(filename, O_RDWR);

    if (file < 0) {
        ROS_ERROR("Failed to open the i2c bus.\n");
	close(file);
        return -1;
    }

    //int addr = I2CADDRESS1; //addess of the I2C port got it from  'i2cdetect -y 3'

    if (ioctl(file, I2C_SLAVE, addr) < 0) {
        ROS_ERROR("Failed to acquire buc access.\n");
	close(file);
        return -1;
    }

    //------ READ BYTES with SMBus ----
    __s32 result_volt = i2c_smbus_read_word_data(file, reg);
    if (result_volt < 0) {
	ROS_ERROR("Failed to read from register: 0x%02X", reg);
	close(file);
	return -1;
    }
    close(file);
    return result_volt;
}
float batmon_driver::get_voltage() {
    // divide by 1000 to convert mV to V
    return read_16bit_word(SMBUS_VOLTAGE) / ((float) 1000);
}

float batmon_driver::get_current() {
    __s16 curr = read_16bit_word(SMBUS_CURRENT);
    //divide by 1000 to convert mAh to Ah
    return curr / ((float) 1000);
}

float batmon_driver::get_charge() {
    //divide by 1000 to convert mAh to Ah
    return read_16bit_word(SMBUS_REMAIN_CAP) / ((float) 1000);
}

float batmon_driver::get_capacity() {
    //divide by 1000 to convert mAh to Ah
    return read_16bit_word(SMBUS_FULL_CAP) / ((float) 1000);
}

float batmon_driver::get_percentage() {
    int charge_rem = read_16bit_word(SMBUS_REMAIN_CAP);
    int charge_tot = read_16bit_word(SMBUS_FULL_CAP);
    //calculates the battery percentage
    if (charge_rem == -1 || charge_tot == -1)
        return -1;
    else
        return charge_rem > charge_tot ? (float) 1 : (float) charge_rem / ((float) charge_tot);
}

unsigned int batmon_driver::get_status() {
    //todo: wait until feature becomes available
    return POWER_SUPPLY_STATUS_UNKNOWN;
}

unsigned int batmon_driver::get_health() {
    //todo: wait until feature becomes available
    return POWER_SUPPLY_HEALTH_UNKNOWN;
}

unsigned int batmon_driver::get_technology() {
    //todo: wait unil feat becomes available
    return POWER_SUPPLY_TECHNOLOGY_UNKNOWN;
}

bool batmon_driver::is_present() {
    return get_voltage() != -1;
}

std::vector<float> batmon_driver::get_cell_voltage() {
    int num_cells = read_16bit_word(SMBUS_CELL_COUNT);
    std::vector<float> cell_volts(num_cells);
    int addr = SMBUS_VCELL1;
    for (int i = 0; i < num_cells; i++) {
        float value = read_16bit_word(addr--);
	//divide by 1000 to convert mV to V
        cell_volts[i] = value / ((float) 1000);
    }
    return cell_volts;
}

std::string batmon_driver::get_serialnumber() {
    int inStr = read_16bit_word(SMBUS_SERIAL_NUM);
    std::stringstream ss;
    ss << inStr;
    return ss.str();
}
