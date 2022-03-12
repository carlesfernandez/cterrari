// Old-school, plain C program for reading the SHT31 sensor
//
// Magic numbers from
// https://cdn-shop.adafruit.com/product-files/2857/Sensirion_Humidity_SHT3x_Datasheet_digital-767294.pdf
//
// Build with gcc sht31-dual.c -o sht31-dual-reader
//
// (C) 2022 C. Fernandez-Prades

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define POLYNOMIAL 0x131
// P(x) = x^8 + x^5 + x^4 + 1 = 100110001

static uint8_t SHT31_Crc(uint8_t *data, uint8_t nbrOfBytes)
{
    uint8_t bit;         // bit mask
    uint8_t crc = 0xFF;  // calculated checksum
    uint8_t byteCtr;     // byte counter

    // calculates 8-bit checksum with given polynomial
    for (byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++)
        {
            crc ^= (data[byteCtr]);
            for (bit = 8; bit > 0; --bit)
                {
                    if (crc & 0x80)
                        crc = (crc << 1) ^ POLYNOMIAL;
                    else
                        crc = (crc << 1);
                }
        }
    return crc;
}


void printHelp()
{
    printf("Usage: sht31-dual-reader [OPTION]...\n");
    printf("  no arguments: Print readings in terminal.\n");
    printf("  -raw          Print temperature and Humidity in terminal without text.\n");
    printf("  -h, --help    Print this message and exit.\n");
}


void main(int argc, char **argv)
{
    if ((argc == 2 &&
            (strcmp("-h", argv[1]) == 0 || strcmp("--help", argv[1]) == 0)) ||
        argc > 2)
        {
            printHelp();
            exit(1);
        }

    bool sensor1_reading = true;
    bool sensor2_reading = true;

    // Create I2C bus for internal sensor
    int file;
    char *bus = "/dev/i2c-1";
    if ((file = open(bus, O_RDWR)) < 0)
        {
            printf("Failed to open the bus for sensor 1 at /dev/i2c-1\n");
            sensor1_reading = false;
        }
    // Create I2C bus for external sensor
    int file2;
    char *bus2 = "/dev/i2c-3";
    if ((file2 = open(bus2, O_RDWR)) < 0)
        {
            printf("Failed to open the bus for sensor 2 at /dev/i2c-3\n");
            sensor2_reading = false;
        }
    // Get I2C devices, SHT31 I2C address is 0x44(68)
    if (sensor1_reading)
        {
            ioctl(file, I2C_SLAVE, 0x44);
        }

    if (sensor2_reading)
        {
            ioctl(file2, I2C_SLAVE, 0x44);
        }

    // Send high repeatability measurement command
    // Command msb, command lsb(0x2C, 0x06)
    char config[2] = {0};
    config[0] = 0x2C;
    config[1] = 0x06;
    if (sensor1_reading)
        {
            write(file, config, 2);
        }
    if (sensor2_reading)
        {
            write(file2, config, 2);
        }
    sleep(1);  // take a nap of 1 second

    // Read 6 bytes of data
    // temp msb, temp lsb, temp CRC, humidity msb, humidity lsb, humidity CRC
    char data[6] = {0};
    char data2[6] = {0};
    if (sensor1_reading && read(file, data, 6) != 6)
        {
            printf("Input/output error in sensor 1.\n");
            sensor1_reading = false;
        }
    if (sensor2_reading && read(file2, data2, 6) != 6)
        {
            printf("Input/output error in sensor 2.\n");
            sensor2_reading = false;
        }

    // Convert the data
    double cTemp = 100.0;
    double humidity = 0.0;
    double cTemp2 = 100.0;
    double humidity2 = 0.0;
    if (sensor1_reading)
        {
            cTemp = (((data[0] * 256) + data[1]) * 175.0) / 65535.0 - 45.0;
            humidity = (((data[3] * 256) + data[4])) * 100.0 / 65535.0;
        }
    if (sensor2_reading)
        {
            cTemp2 = (((data2[0] * 256) + data2[1]) * 175.0) / 65535.0 - 45.0;
            humidity2 = (((data2[3] * 256) + data2[4])) * 100.0 / 65535.0;
        }
    // data[2] is the CRC of Temperature
    uint8_t crc_temp = SHT31_Crc(&data[0], 2);
    uint8_t crc_temp2 = SHT31_Crc(&data2[0], 2);

    // data[5] is the CRC of Relative Humidity
    uint8_t crc_hum = SHT31_Crc(&data[3], 2);
    uint8_t crc_hum2 = SHT31_Crc(&data2[3], 2);

    if (argc < 2)  // no arguments were passed
        {
            if (sensor1_reading)
                {
                    if (crc_temp != data[2])
                        {
                            printf("Temperature CRC in sensor 1 is not OK.\n");
                        }
                    else
                        {
                            printf("Temperature in sensor 1 is %.2f C\n", cTemp);
                        }
                }
            if (sensor2_reading)
                {
                    if (crc_temp2 != data2[2])
                        {
                            printf("Temperature CRC in sensor 2 is not OK.\n");
                        }
                    else
                        {
                            printf("Temperature in sensor 2 is %.2f C\n", cTemp2);
                        }
                }
            if (sensor1_reading)
                {
                    if (crc_hum != data[5])
                        {
                            printf("Relative Humidity CRC in sensor 1 is not OK.\n");
                        }
                    else
                        {
                            printf("Relative Humidity in sensor 1 is %.2f RH\n", humidity);
                        }
                }
            if (sensor2_reading)
                {
                    if (crc_hum2 != data2[5])
                        {
                            printf("Relative Humidity CRC in sensor 2 is not OK.\n");
                        }
                    else
                        {
                            printf("Relative Humidity in sensor 2 is %.2f RH\n", humidity2);
                        }
                }
        }

    if (argc == 2)
        {
            if (strcmp("-raw", argv[1]) == 0)
                {
                    if (sensor1_reading && sensor2_reading)
                        {
                            if (crc_temp != data[2] || crc_hum != data[5] ||
                                crc_temp2 != data2[2] || crc_hum2 != data2[5])
                                {
                                    // out of range
                                    printf("100 0 100 0\n");
                                }
                            else
                                {
                                    printf("%.2f %.2f %.2f %.2f\n", cTemp, humidity, cTemp2, humidity2);
                                }
                        }
                    else
                        {
                            if (sensor1_reading && !sensor2_reading)
                                {
                                    if (crc_temp != data[2] || crc_hum != data[5])
                                        {
                                            // out of range
                                            printf("100 0 100 0\n");
                                        }
                                    else
                                        {
                                            printf("%.2f %.2f 100 0\n", cTemp, humidity);
                                        }
                                }
                            if (!sensor1_reading && sensor2_reading)
                                {
                                    if (crc_temp2 != data2[2] || crc_hum2 != data2[5])
                                        {
                                            // out of range
                                            printf("100 0 100 0\n");
                                        }
                                    else
                                        {
                                            printf("100 0 %.2f %.2f\n", cTemp2, humidity2);
                                        }
                                }
                            if (!sensor1_reading && !sensor2_reading)
                                {
                                    // out of range
                                    printf("100 0 100 0\n");
                                }
                        }
                }
            else
                {
                    printf("Unknown argument %s\n\n", argv[1]);
                    printHelp();
                }
        }
}
