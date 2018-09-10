#include "spi_uart.h"
#include "queue.h"
#include "ucos_ii.h"
#include "chip_communication.h"

uint32_t spi_uart_1_recv_buffer[128];
uint32_t spi_uart_1_send_buffer[128];

uint32_t spi_uart_2_recv_buffer[128];
uint32_t spi_uart_2_send_buffer[128];

uint32_t spi_uart_3_recv_buffer[128];
uint32_t spi_uart_3_send_buffer[128];

uint8_t spi_int;
uint8_t spi_uart_buffer[32];
void TaskSpiUart(void *pdata)
{
    s_uart_st_t uart_st;
    int len;
    int i;
    pdata = pdata;
    QueueCreate((void *)spi_uart_1_recv_buffer,sizeof(spi_uart_1_recv_buffer),NULL,NULL);
    QueueCreate((void *)spi_uart_1_send_buffer,sizeof(spi_uart_1_send_buffer),NULL,NULL);
    QueueCreate((void *)spi_uart_2_recv_buffer,sizeof(spi_uart_2_recv_buffer),NULL,NULL);
    QueueCreate((void *)spi_uart_2_send_buffer,sizeof(spi_uart_2_send_buffer),NULL,NULL);
    QueueCreate((void *)spi_uart_3_recv_buffer,sizeof(spi_uart_3_recv_buffer),NULL,NULL);
    QueueCreate((void *)spi_uart_3_send_buffer,sizeof(spi_uart_3_send_buffer),NULL,NULL);
    //if (spi_int == 1)
    while (1)
    {
        OSTimeDly(5);
        if (ChipReadFrame(1, CH2_UART_ST_ADDR, sizeof(uart_st), &uart_st) == 0)
        {
            spi_int = 0;
            //  处理uart1
            while (uart_st.uart1_in_num)
            {
                len = uart_st.uart1_in_num/32;
                if (len == 0)
                {
                    len = uart_st.uart1_in_num%32;
                }
                else
                {
                    len = 32;
                }
                //  确定uart1的buffer足够才读取
                i = QueueSize(spi_uart_1_recv_buffer) - QueueNData(spi_uart_1_recv_buffer);
                if (i < len)
                {
                    len = i;  //  有多少buffer读多少数据
                }
                if (len)
                {
                    if (ChipReadFrame(2, 0, len, spi_uart_buffer) == 0)
                    {
                        uart_st.uart1_in_num -= len;
                        for (i = 0; i<len; i++)
                        {
                            QueueWrite(spi_uart_1_recv_buffer, spi_uart_buffer[i]);
                        }
                        
                    }
                }
                else
                {
                    break;
                }
            }
            //  处理uart2
            while (uart_st.uart2_in_num)
            {
                len = uart_st.uart2_in_num/32;
                if (len == 0)
                {
                    len = uart_st.uart2_in_num%32;
                }
                else
                {
                    len = 32;
                }
                //  确定uart1的buffer足够才读取
                i = QueueSize(spi_uart_2_recv_buffer) - QueueNData(spi_uart_2_recv_buffer);
                if (i < len)
                {
                    len = i;  //  有多少buffer读多少数据
                }
                if (len)
                {
                    if (ChipReadFrame(3, 0, len, spi_uart_buffer) == 0)
                    {
                        //ChipReadFrame(3, 0, len, spi_uart_buffer);
                        uart_st.uart2_in_num -= len;
                        for (i = 0; i<len; i++)
                        {
                            QueueWrite(spi_uart_2_recv_buffer, spi_uart_buffer[i]);
                        }
                        
                    }
                }
                else
                {
                    break;
                }
            }
            //  处理uart3
            while (uart_st.uart3_in_num)
            {
                len = uart_st.uart3_in_num/32;
                if (len == 0)
                {
                    len = uart_st.uart3_in_num%32;
                }
                else
                {
                    len = 32;
                }
                //  确定uart1的buffer足够才读取
                i = QueueSize(spi_uart_3_recv_buffer) - QueueNData(spi_uart_3_recv_buffer);
                if (i < len)
                {
                    len = i;  //  有多少buffer读多少数据
                }
                if (len)
                {
                    if (ChipReadFrame(4, 0, len, spi_uart_buffer) == 0)
                    {
                        uart_st.uart3_in_num -= len;
                        for (i = 0; i<len; i++)
                        {
                            QueueWrite(spi_uart_3_recv_buffer, spi_uart_buffer[i]);
                        }
                        
                    }
                }
                else
                {
                    break;
                }
            }
            
        }
        while (QueueNData(spi_uart_1_send_buffer))
        {
            i = 32;
            //  检查芯片buffer的剩余空间
            if (uart_st.uart1_out_empty == 0)
            {
                break;
            }
            else if (uart_st.uart1_out_empty < 32)
            {
                i = uart_st.uart1_out_empty;
            }
            for (len=0; len<i; len++)
            {
                if (QueueRead(&spi_uart_buffer[len], spi_uart_1_send_buffer) != QUEUE_OK)
                {
                    break;
                }
            }
            while (ChipWriteFrame(2, 0, len, spi_uart_buffer) != 0)
            {
                OSTimeDly(10);
            }
        }
        while (QueueNData(spi_uart_2_send_buffer))
        {
            i = 32;
            //  检查芯片buffer的剩余空间
            if (uart_st.uart2_out_empty == 0)
            {
                break;
            }
            else if (uart_st.uart2_out_empty < 32)
            {
                i = uart_st.uart2_out_empty;
            }
            for (len=0; len<i; len++)
            {
                if (QueueRead(&spi_uart_buffer[len], spi_uart_2_send_buffer) != QUEUE_OK)
                {
                    break;
                }
            }
            while (ChipWriteFrame(3, 0, len, spi_uart_buffer) != 0)
            {
                OSTimeDly(10);
            }
        }
        while (QueueNData(spi_uart_3_send_buffer))
        {
            i = 32;
            //  检查芯片buffer的剩余空间
            if (uart_st.uart3_out_empty == 0)
            {
                break;
            }
            else if (uart_st.uart3_out_empty < 32)
            {
                i = uart_st.uart3_out_empty;
            }
            for (len=0; len<i; len++)
            {
                if (QueueRead(&spi_uart_buffer[len], spi_uart_3_send_buffer) != QUEUE_OK)
                {
                    break;
                }
            }
            while (ChipWriteFrame(4, 0, len, spi_uart_buffer) != 0)
            {
                OSTimeDly(10);
            }
        }
    }
}

int32_t SpiUart1Write(int32_t port, void *buf, uint32_t buf_len)
{
    uint8_t * data = (uint8_t *)buf;
    uint32_t i = 0;
    for (i=0; i<buf_len; i++)
    {
        if (QueueWrite(spi_uart_1_send_buffer, data[i]) != QUEUE_OK)
        {
            if (i)   i--;
            OSTimeDly(5);
        }
    }
    return buf_len;
}

int32_t SpiUart1Read(int32_t port, void *buf, uint32_t buf_len)
{
    uint8_t * data = (uint8_t *)buf;
    uint32_t i = 0;
    for (i=0; i<buf_len; i++)
    {
        if (QueueRead(&data[i], spi_uart_1_recv_buffer) != QUEUE_OK)
        {
            break;
        }
    }
    return i;
}


int32_t SpiUart2Write(int32_t port, void *buf, uint32_t buf_len)
{
    uint8_t * data = (uint8_t *)buf;
    uint32_t i = 0;
    for (i=0; i<buf_len; i++)
    {
        if (QueueWrite(spi_uart_2_send_buffer, data[i]) != QUEUE_OK)
        {
            if (i)   i--;
            OSTimeDly(5);
        }
    }
    return buf_len;
}

int32_t SpiUart2Read(int32_t port, void *buf, uint32_t buf_len)
{
    uint8_t * data = (uint8_t *)buf;
    uint32_t i = 0;
    for (i=0; i<buf_len; i++)
    {
        if (QueueRead(&data[i], spi_uart_2_recv_buffer) != QUEUE_OK)
        {
            break;
        }
    }
    return i;
}


int32_t SpiUart3Write(int32_t port, void *buf, uint32_t buf_len)
{
    uint8_t * data = (uint8_t *)buf;
    uint32_t i = 0;
    for (i=0; i<buf_len; i++)
    {
        if (QueueWrite(spi_uart_3_send_buffer, data[i]) != QUEUE_OK)
        {
            if (i)   i--;
            OSTimeDly(5);
        }
    }
    return buf_len;
}

int32_t SpiUart3Read(int32_t port, void *buf, uint32_t buf_len)
{
    uint8_t * data = (uint8_t *)buf;
    uint32_t i = 0;
    for (i=0; i<buf_len; i++)
    {
        if (QueueRead(&data[i], spi_uart_3_recv_buffer) != QUEUE_OK)
        {
            break;
        }
    }
    return i;
}
#define TaskSpiUartSize     128
OS_STK TaskSpiUartStk[TaskSpiUartSize];
void InitSpiUart(uint32_t prio)
{
    OSTaskCreate(TaskSpiUart, (void *)0, &TaskSpiUartStk[TaskSpiUartSize - 1], prio);
}

