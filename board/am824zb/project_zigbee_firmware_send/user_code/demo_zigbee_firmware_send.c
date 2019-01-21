/*******************************************************************************
*                                 AMetal
*                       ----------------------------
*                       innovating embedded platform
*
* Copyright (c) 2001-2018 Guangzhou ZHIYUAN Electronics Stock Co., Ltd.
* All rights reserved.
*
* Contact information:
* web site:    http://www.zlg.cn/
* e-mail:      ametal.support@zlg.cn
*******************************************************************************/

/**
 * \file
 * \brief ZM516X ʵ�����߹̼���������demoʵ�����еķ��͹̼����ܣ��������ص�A����
 *
 *
 * \note
 *    1. LED0 ��Ҫ�̽� J9 ����ñ�����ܱ� PIO0_8 ���ƣ�
 *    2. ����0��Ҫ��PC���ӣ�ͨ���������ַ��͹̼���������Ϊ9600���������ӹ�ϵ���£�
 * <pre>
 *           PIO0_0   <-->  PC_TX
 *           PIO0_4   <-->  PC_RX
 * </pre>
 *        �����Ҫʹ�� ZigBee����Щ IO �ڲ�������������;��
 *
 * \par Դ����
 * \snippet demo_lpc82x_std_zm516x.c src_lpc28x_std_zm516x
 *
 * \internal
 * \par Modification History
 * - 1.01 18-01-16  pea, simplify demo, enable display head
 * - 1.00 17-05-26  mex, first implementation
 * \endinternal
 */

/**
 * \addtogroup demo_if_lpc82x_std_zm516x
 * \copydoc demo_lpc82x_std_zm516x.c
 */

/** [src_lpc28x_std_zm516x] */
#include "ametal.h"
#include "am_zm516x.h"
#include "am_led.h"
#include "am_delay.h"
#include "am_vdebug.h"
#include "am_board.h"
#include <string.h>

#include "am_uart.h"
#include "am_lpc82x_inst_init.h"


#define RNGBUF_SIZE   1024   
#define SEND_BUF_SIZE 32

/**
 * \brief ��˸ LED0
 */
static void flash_led (void)
{
    while (1) {
        am_led_on(LED0);
        am_mdelay(50);

        am_led_off(LED0);
        am_mdelay(500);
    }
}

static void __timer (void *p_arg)
{
	 uint8_t *p_flog = (uint8_t *)p_arg;
	
	 (*p_flog) ++;
}

static uint8_t uart_rx_buf[RNGBUF_SIZE];

static am_uart_rngbuf_dev_t __g_rngbuf_dev;

static uint8_t  buf[64] = {0};

/**
 * \brief �������
 */
void demo_zigbee_firmware_send_entry (void)
{
    uint8_t              my_addr[2] = {0x00, 0x00};
    uint8_t              dst_addr[2] = {0x20, 0x02};
		
	  uint8_t  dat;
		static uint16_t dst_addr_set = 0x0001;

    am_zm516x_cfg_info_t zm516x_cfg_info;
    am_zm516x_handle_t   zm516x_handle    = am_zm516x_inst_init();
		
		am_uart_handle_t        uart_handle;
		am_uart_rngbuf_handle_t rngbuf_handle = NULL;

		
		    /* ��ȡ ZigBee ģ���������Ϣ���������D1�� */
    if (am_zm516x_cfg_info_get(zm516x_handle, &zm516x_cfg_info) != AM_OK) {
        AM_DBG_INFO("am_zm516x_cfg_info_get failed\r\n");
    }

		zm516x_cfg_info.serial_rate = 3; /* ������9600 */
		
		zm516x_cfg_info.my_addr[0] = my_addr[0];
		zm516x_cfg_info.my_addr[1] = my_addr[1];
		
    /* �޸� ZigBee ģ���������Ϣ���������D6�������óɹ��踴λ */
    if (am_zm516x_cfg_info_set(zm516x_handle, &zm516x_cfg_info) != AM_OK) {
        AM_DBG_INFO("am_zm516x_cfg_info_set failed\r\n");
    }
		
    /* ʹ ZigBee ģ�鸴λ���������D9�� */
    am_zm516x_reset(zm516x_handle);
    am_mdelay(10);

		am_uart_ioctl(zm516x_handle->uart_handle->handle, 
		              AM_UART_BAUD_SET, 
		              (void *)9600);
    /**
     * ���� ZigBee ģ�����˯��ģʽ����ʱ���D8������Ҫ�������ԣ�ZM516X ˯�ߺ�
     * ֻ�ܸ�λģ����ģ��� WAKE ��������������ģ��
     */
//    am_zm516x_enter_sleep(zm516x_handle);
		
    /* ���в���ͨ�������� LED0 */
    am_led_on(LED0);
		
    am_softimer_t timer;
		uint8_t       flog       = 0;
		uint8_t       recv[10]   = {0};
		
		uint32_t num = 0; 
		uint32_t count = 0;
		
		uart_handle = am_lpc82x_usart0_inst_init();
    am_uart_ioctl(uart_handle, AM_UART_MODE_SET, (void *)AM_UART_MODE_POLL);
		am_uart_ioctl(uart_handle, AM_UART_BAUD_SET, (void *)9600);
		
		rngbuf_handle = am_uart_rngbuf_init(&__g_rngbuf_dev,
		                                    uart_handle,
		                                    uart_rx_buf,
		                                    sizeof(uart_rx_buf),
																				&dat,
																				1);
																				
		am_uart_rngbuf_ioctl(rngbuf_handle, AM_UART_RNGBUF_TIMEOUT, (void *)20);
		am_softimer_init(&timer, __timer, &flog);
		am_softimer_start(&timer, 10);
		
    AM_FOREVER {
			
			  num = am_uart_rngbuf_receive(rngbuf_handle, buf, sizeof(buf));
				int i = 0;
			 /* ������ܵ��̼�����ͨ��zigbee���͸�Ŀ��� */
			  if (num > 0) {
					
					  flog = 0;
					  count += num;
						
					  am_zm516x_send(zm516x_handle, buf, num);

					  memset(uart_rx_buf, 0xff, sizeof(buf));
					           					
				}
				
				/* ����յ�Ŀ���Ļ�Ӧ */
				if (am_zm516x_receive(zm516x_handle, recv, sizeof(recv)) > 0) {
					  if (strstr((const char *)recv, "recvOK") != NULL) {
							  AM_DBG_INFO("set addr success, next dst_addr is 0x%04x\r\n", ++dst_addr_set);
							  memset(recv, 0, sizeof(recv));
						}
				}
				
				/* 300ms��ʱδ���յ��̼����ݣ����жϹ̼�������� */
				if (flog >= 30 && count > 0) {

					AM_DBG_INFO("\r\n set now dst addr 0x%04x\r\n", dst_addr_set);
					
					/* ������Ҫ�޸ĵ�Ŀ����zigbee�ı��ص�ַ*/
					am_zm516x_send(zm516x_handle, &dst_addr_set, sizeof(dst_addr_set));
					
					AM_DBG_INFO("\r\nsuccessul size = %d bytes\r\n", count);
					
					count     = 0;
				}				
    }
}
/** [src_lpc28x_std_zm516x] */

/* end of file */