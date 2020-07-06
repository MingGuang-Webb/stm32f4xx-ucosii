/*
 * @Author: your name
 * @Date: 2020-06-25 12:52:42
 * @LastEditTime: 2020-06-25 15:20:52
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: \stm32f4xx-ucosii\App\app_imu.c
 */ 
#include "app_imu.h"
#include "stm32f4xx_hal.h"
#include "ucos_ii.h"
#include "sd_hal_mpu6050.h"
#include "i2c.h"

extern I2C_HandleTypeDef hi2c1;
static SD_MPU6050 imu;

static void calculate_standard_deviation(float *pdata)
{
}

void imu_task(void *arg)
{
    SD_MPU6050_Result rc;
    
    rc = SD_MPU6050_Init(&hi2c1, &imu, SD_MPU6050_Device_0, SD_MPU6050_Accelerometer_2G, SD_MPU6050_Gyroscope_2000s);
    if (rc == SD_MPU6050_Result_Ok)
    {
       // ucos_kprintf("Mpu 6050 Init Success.\r\n");
    }
    for (;;)
    {

        OSTimeDly(2000);
    }
}
