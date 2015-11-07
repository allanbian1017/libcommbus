/***************************************************************************
 *   Copyright (C) 2015 by Tse-Lun Bien                                    *
 *   allanbian@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "commbus.h"
#include "uart.h"

HANDLE uart_fd[COM_MAX];

int uart_open(int com, int baudrate, int parity, int databits, int stopbits)
{
    char path[32];
    DCB setting;
    COMMTIMEOUTS timeouts;
    DWORD br;
    BYTE par;
    BYTE dbits;
    BYTE sbits;
    
    if (com >= COM_MAX || com < 0)
		return -LIBCOMMBUS_ERROR_NO_DEVICE;
		
    switch (baudrate) {
		case 110: 
			br = CBR_110;
			break;
		case 300: 
			br = CBR_300;
			break;
		case 600: 
			br = CBR_600;
			break;
		case 1200: 
			br = CBR_1200;
			break;
		case 2400: 
			br = CBR_2400;
			break;
		case 4800: 
			br = CBR_4800;
			break;
		case 9600: 
			br = CBR_9600;
			break;
		case 19200: 
			br = CBR_19200;
			break;
		case 38400: 
			br = CBR_38400;
			break;
		case 57600: 
			br = CBR_57600;
			break;
		case 115200: 
			br = CBR_115200;
			break;
		case 128000:
            br = CBR_128000;
            break;
		case 256000: 
			br = CBR_256000;
			break;
		default: 
			return -LIBCOMMBUS_ERROR_NOT_SUPPORT;
	}
	
	switch (parity) {
		case PAR_NONE:
			par = NOPARITY;
			break;         
		case PAR_ODD:
			par = ODDPARITY;
			break;         
		case PAR_EVEN:
			par = EVENPARITY;
			break;         
		default:
			return -LIBCOMMBUS_ERROR_NOT_SUPPORT;
	}
	
	switch (databits) {
		case DATBITS_6:
			dbits = 6;
			break;         
		case DATBITS_7:
			dbits = 7;
			break;         
		case DATBITS_8:
			dbits = 8;
			break;         
		default:
			return -LIBCOMMBUS_ERROR_NOT_SUPPORT;
	}
	
	switch (stopbits) {
		case STOPBITS_1:
			sbits = ONESTOPBIT;
			break;         
		case STOPBITS_2:
			sbits = TWOSTOPBITS;
			break;         
		default:
			return -LIBCOMMBUS_ERROR_NOT_SUPPORT;
	}
	
	sprintf(path, "\\\\.\\COM%d", com);

    uart_fd[com] = CreateFileA(path,
                      GENERIC_READ|GENERIC_WRITE,
                      0,                          /* no share  */
                      NULL,                       /* no security */
                      OPEN_EXISTING,
                      0,                          /* no threads */
                      NULL);                      /* no templates */

    if (uart_fd[com] == INVALID_HANDLE_VALUE)
        return -LIBCOMMBUS_ERROR_ACCESS;

    
    memset(&setting, 0, sizeof(setting));
    setting.DCBlength = sizeof(setting);

    if (!GetCommState(uart_fd[com], &setting)) {
        CloseHandle(uart_fd[com]);
        return -LIBCOMMBUS_ERROR_ACCESS;
    }

    setting.BaudRate		= br;
	setting.ByteSize		= dbits;
	setting.Parity	        = par;
	setting.StopBits		= sbits;
	setting.fBinary	        = TRUE;
	if (parity == PARITY_NONE) {
       setting.fParity	        = FALSE;
    } else {
       setting.fParity	        = TRUE;
    }

	setting.fOutxDsrFlow	= FALSE;
	setting.fOutxCtsFlow	= FALSE;
	setting.fDtrControl	    = DTR_CONTROL_DISABLE;
    setting.fDsrSensitivity   = FALSE;
    setting.fTXContinueOnXoff = FALSE;
    setting.fOutX			= FALSE;
    setting.fInX			= FALSE;
    setting.fErrorChar      = FALSE;
    setting.fNull           = FALSE;
	setting.fRtsControl     = RTS_CONTROL_DISABLE;
    setting.fAbortOnError   = FALSE;	

    if (!SetCommState(uart_fd[com], &setting)) {
        CloseHandle(uart_fd[com]);
        return -LIBCOMMBUS_ERROR_ACCESS;
    }

    if (!SetupComm(uart_fd[com], 32, 32)) {
        CloseHandle(uart_fd[com]);
        return -LIBCOMMBUS_ERROR_ACCESS;
    }

    if (!GetCommTimeouts(uart_fd[com], &timeouts)) {
        CloseHandle(uart_fd[com]);
        return -LIBCOMMBUS_ERROR_ACCESS;
    } 

    /* blocking read/write operation */
    timeouts.ReadIntervalTimeout         = 0;
    timeouts.ReadTotalTimeoutMultiplier  = 0;
    timeouts.ReadTotalTimeoutConstant    = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant   = 0;

    if (!SetCommTimeouts(uart_fd[com], &timeouts)) {
        CloseHandle(uart_fd[com]);
        return -LIBCOMMBUS_ERROR_ACCESS; 
    }

    return LIBCOMMBUS_SUCCESS;
}

int uart_read(int com, unsigned char *data, int len)
{
    int n;

    ReadFile(uart_fd[com], data, len, (LPDWORD)((void *)&n), NULL);

    return n;
}

int uart_write(int com, unsigned char *data, int len)
{
    int n;

    WriteFile(uart_fd[com], data, len, (LPDWORD)((void *)&n), NULL);

    return n;
}

int uart_flush(int com)
{
    if (!PurgeComm(uart_fd[com], PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR)) {
       return -LIBCOMMBUS_ERROR_ACCESS;
    }
    
    return LIBCOMMBUS_SUCCESS;
}

int uart_close(int com)
{
    if (!CloseHandle(uart_fd[com])) {
       return -LIBCOMMBUS_ERROR_ACCESS;
    }
    
    return LIBCOMMBUS_SUCCESS;
}

