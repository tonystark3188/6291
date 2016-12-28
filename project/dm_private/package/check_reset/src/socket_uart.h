#ifndef __SOCKET_UART_H__
#define __SOCKET_UART_H__

/******************************************************************************
 *                               INCLUDES                                     *
 ******************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */


#define SOCKET_UART_PORT 		13131
#define SOCKET_UART_WAIT_COUNT 	5
#define SOCKET_UART_TIMEOUT 	1000

//GET CMD
#define SOCKET_UART_GET_START				0x30

#define SOCKET_UART_GET_POWER_PERCENT		0x30
#define SOCKET_UART_GET_BOTTON_STATUS		0x31
#define SOCKET_UART_GET_CHARGE_STATUS		0x32
#define SOCKET_UART_GET_STORAGE_POWER		0x33
#define SOCKET_UART_GET_PC_DETEC			0x34
#define SOCKET_UART_GET_FIMRWARE_VERSION	0x35
#define SOCKET_UART_GET_KEY				    0x36
#define SOCKET_UART_GET_IF_POWEROFF			0x37
#define SOCKET_UART_GET_LINE_IN				0x38
#define SOCKET_UART_GET_STORAGE_DIRECTION	0x39
#define SOCKET_UART_GET_POWER_STATUS        0x40

#define SOCKET_UART_GET_END					0x6F

//SET CMD
#define SOCKET_UART_SET_START				0x50

#define SOCKET_UART_SET_RESET				0x50
#define SOCKET_UART_SET_POWEROFF			0x51
#define SOCKET_UART_SET_STORAGE_POWER		0x52
#define SOCKET_UART_SET_STORAGE_DIRECTION	0x53
#define SOCKET_UART_SET_STORAGE_MODE		0x54
#define SOCKET_UART_SET_VERIFY_KEY			0x55
#define SOCKET_UART_SET_AMP					0x56
#define SOCKET_UART_SET_MUTE				0x57
#define SOCKET_UART_SET_SURE_POWEROFF		0x58

#define SOCKET_UART_SET_END					0x7F


#define SOCKET_UART_STM8_UPLOAD				0xAA   //NO ADDR



enum error_code{
	ERROR_CLIENT_DATA = -100,
	ERROR_CLIENT_INIT = -101,
	ERROR_CLIENT_SEND = -102,
	ERROR_CLIENT_RECV = -103,
	ERROR_CLIENT_TIMEOUT = -104,
	ERROR_CLIENT_PARSE = -105,
	
	ERROR_SERVER_PARSE = -110,
	ERROR_SERVER_UART_HANDLE = -111,
	ERROR_SERVER_DATA = -112
};

enum uart_rw_mode{
	UART_W,
	UART_R
};

enum button_status{
	BUTTON_UP = 0,
	BUTTON_DOWN_LED_OFF = 1,
	BUTTON_DOWN_RESET = 0X80,
	BUTTON_DOUBLE = 0x800
};

enum pc_detect_status{
	DETECT_BOARD,
	DETECT_PC
};


enum storage_direction_status{
	STORAGE_PC,
	STORAGE_BOARD
};

enum storage_power_status{
	STORAGE_POWER_OFF,
	STORAGE_POWER_ON
};

enum power_charge_status{
	POWER_NORMAL,
	POWER_CHARGING,
	POWER_DISCHARGING,
	POWER_LOW
};

enum power_status {
	NOT_NORMAL_POWER,
	NORMAL_POWER
};

typedef struct socket_uart_cmd
{
	unsigned char regaddr;
	unsigned short data;
	unsigned char mode;   // 0 --read  1--write
	//unsigned char error;  //error code : 0--success   -1--fail
}socket_uart_cmd_t;

int SocketUartServerStart();

int SocketUartClientStart(socket_uart_cmd_t *p_socket_uart_cmd);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __MSG_H__ */
