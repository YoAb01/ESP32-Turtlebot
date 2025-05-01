#ifndef IMU_GY521_H
#define IMU_GY521_H

#include "esp_err.h"
#include <cstdint>
#include <stdint.h>
#include <string.h>
#include <vector>

void imu_gy521_init(void);

void imu_gy521_read_raw(double* accelX, double* accelY, double* accelZ,
                        double* gyroX, double* gyroY, double* gyroZ, double* tempC);

void imu_gyro521_rpy(double* roll, double* pitch, double* yaw);

#endif  // IMU_GY521_H
