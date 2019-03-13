#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#define Max_Cycles 10000
extern uint8_t ReadByte(uint8_t slaveAddr, uint8_t ReadAddr); // creando variable no definida
extern void ReadBit(uint8_t slaveAddr, uint8_t regAddr, uint8_t bitNum, uint8_t *data);
extern void ReadWord(uint8_t slaveAddr, uint16_t* data, uint8_t ReadAddr);
extern uint16_t MLX90614_ReadAmbientTemp(void);
extern uint16_t MLX90614_ReadObjectTemp(void);
extern uint8_t SMBus_CRC8(uint32_t data);

int main(void)
{
RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOD, ENABLE);

GPIO_InitTypeDef GPIO_InitStruct;
GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 ;
GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
GPIO_Init (GPIOD, &GPIO_InitStruct);

I2C_InitTypeDef I2CInitStruct;
GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);



I2CInitStruct.I2C_Mode = I2C_Mode_SMBusHost;
I2CInitStruct.I2C_ClockSpeed = 50000;
I2CInitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
I2CInitStruct.I2C_OwnAddress1 = 0x00;
I2CInitStruct.I2C_Ack = I2C_Ack_Enable;
I2CInitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
I2C_Init(I2C1, &I2CInitStruct);


void ReadByte(uint8_t slaveAddr, uint16_t* data, uint8_t ReadAddr)
{
    uint8_t buff[2]; //generando buffers
	volatile uint8_t pec, expected_pec;
	uint32_t counter = Max_Cycles; // Contador para generar bucle

    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) && counter); // Creando bandera de estado
    if(counter == 0) {
	    return;
    }

    I2C_GenerateSTART(I2C1, ENABLE);
    counter = Max_Cycles;
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)&& counter);
    if(counter==0){
    	return;
    }
    I2C_Send7bitAddress(I2C1, 0x5a, I2C_Direction_Transmitter);// I2C_Direction_Transmitter 0x00
    counter = Max_Cycles;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && counter);
    if(counter == 0) {
        return;
     }

    I2C_Cmd(I2C1, ENABLE);
    I2C_SendData(I2C1, ReadAddr);
    counter = Max_Cycles;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) && counter);
    if(counter == 0) {
        return;
        }
    I2C_GenerateSTART(I2C1, ENABLE);
    counter = Max_Cycles;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) && counter);
    if(counter == 0) {
       return;
        }
    I2C_Send7bitAddress(I2C1, 0x5a, I2C_Direction_Receiver);
    counter = Max_Cycles;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && counter);
    if(counter == 0) {
        return;
    }

    counter = Max_Cycles;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) && counter);
    if(counter == 0) {
        return;
    }
    buff[0] = I2C_ReceiveData(I2C1);
    counter = Max_Cycles;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) && counter);
    if(counter == 0) {
        return;
    }
    buff[1] = I2C_ReceiveData(I2C1);
    counter = Max_Cycles;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) && counter);
    if(counter == 0) {
        return;
    }
    pec = I2C_ReceiveData(I2C1);
    expected_pec = SMBus_CRC8(((uint32_t)ReadAddr << 16) | ((uint32_t)buff[0] << 8) | buff[1]);
    I2C_GenerateSTOP(I2C1, ENABLE);
    *data = ((uint16_t)buff[1] << 8) | buff[0];
}

void ReadBit(uint8_t slaveAddr, uint8_t regAddr, uint8_t bitNum, uint8_t *data)
    {
    uint8_t tmp = ReadByte(slaveAddr, regAddr);
    *data = tmp & (1 << bitNum);

    uint16_t MLX90614_ReadAmbientTemp(void)
    {
    	uint16_t tmp;
    	ReadWord(0x5a, &tmp, 0x06);
    	uint16_t res = tmp;
    	return res;
    }

    uint16_t MLX90614_ReadObjectTemp(void)
    {
    	uint16_t tmp;
    	ReadWord(0x5a, &tmp, 0x07);
    	uint16_t res = tmp;

    	return res;
    }
 int main(void)
 {
	I2C1_init();

	printf("ok!\r\n");

	while(1)
	{
		uint16_t temp_ambient, temp_object;
		temp_ambient = MLX90614_ReadAmbientTemp();
		temp_object = MLX90614_ReadObjectTemp();
		float temp1 = (temp_ambient - 0x2DE4) * 2.0f - 3820.0f;
		float temp2 = (temp_object) * 2.0f - 27315.0f;
		printf("MLX90614: %d.%d %d.%d\r\n", (int)temp1, (int)(temp1 * 100) % 100, (int)temp2, (int)(temp2 * 100) % 100);
		DelayMs(100);
	}
	return 0;
  }
 }
