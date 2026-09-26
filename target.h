#pragma once

#define TARGET_BOARD_IDENTIFIER "BEFH"

#define USBD_PRODUCT_STRING "BETAFPVH743"

#define LED0                    PC13
#define LED1                    PC14

#define BEEPER                  PD15

#define GYRO_1_CS               SPI1_NSS
#define GYRO_1_EXTI             PC4

#define USE_GYRO
#define USE_GYRO_SPI_ICM42688P
#define GYRO_1_ALIGN            CW180_DEG

#define USE_ACC
#define USE_ACC_SPI_ICM42688P
#define ACC_1_ALIGN             CW180_DEG

#define USE_BARO
#define USE_BARO_DPS310

#define USE_MAX7456
#define MAX7456_CS              SPI2_NSS

#define USE_SDCARD
#define SDCARD_SPI_INSTANCE     SPI3
#define SDCARD_DETECT_PIN       PD3
#define SDCARD_DETECT_INVERTED

#define USE_FLASH
#define FLASH_CS                SPI4_NSS

#define USE_UART1
#define UART1_RX_PIN            PA10
#define UART1_TX_PIN            PA9

#define USE_UART2
#define UART2_RX_PIN            PD6
#define UART2_TX_PIN            PD5

#define USE_UART3
#define UART3_RX_PIN            PD9
#define UART3_TX_PIN            PD8

#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

#define USE_I2C_DEVICE_2
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11

#define DEFAULT_I2C_INSTANCE    I2C2