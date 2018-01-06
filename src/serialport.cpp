/** @file serialport.hpp */
//
// Copyright 2015 Arvind Singh
//
// This file is part of the mtools library.
//
// mtools is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with mtools  If not, see <http://www.gnu.org/licenses/>.


#include "stdafx_mtools.h"

#include "io/serialport.hpp"

#ifdef _WIN32 
#include <windows.h>
#endif 


namespace mtools
	{

	/******************************************************

	TODO : Add Linux / OSX specific code. 

	******************************************************/


// WINDOWS SPECIFIC CODE
#ifdef _WIN32 




	struct SerialPort::SerialPortHandle
		{
		HANDLE x;
		};



	SerialPort::SerialPort() : _phandle(new SerialPortHandle)
				{
				_phandle->x = INVALID_HANDLE_VALUE;
				}


	SerialPort::~SerialPort()
				{
				close();
				}


	int SerialPort::open(std::string portName, int baudRate, bool parityCheck, int parity, int stopBits)
		{
		if (_phandle->x != INVALID_HANDLE_VALUE) { return -1; }
		portName = std::string("\\\\.\\") + portName;
		_phandle->x = CreateFile(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, NULL, NULL);
		if (_phandle->x == INVALID_HANDLE_VALUE) { return -2; }
		DCB dcb;
		memset(&dcb, 0, sizeof(dcb));
		dcb.DCBlength = sizeof(dcb);
		if (!GetCommState(_phandle->x , &dcb)) { close(); return -3; }
		dcb.BaudRate = (DWORD)baudRate;
		dcb.ByteSize = 8;
		dcb.fParity = (parityCheck) ? 1 : 0;
		dcb.fDtrControl = DTR_CONTROL_ENABLE;  // DTR flow control
		dcb.fRtsControl = RTS_CONTROL_ENABLE;  // RTS flow control
		dcb.fBinary = 1;
		dcb.fAbortOnError = TRUE;        // Do not abort reads/writes on error
		switch (parity)
			{
			case SERIALPORT_PARITY_NONE: { dcb.Parity = NOPARITY; break; }
			case SERIALPORT_PARITY_ODD: { dcb.Parity = ODDPARITY; break; }
			case SERIALPORT_PARITY_EVEN: { dcb.Parity = EVENPARITY; break; }
			case SERIALPORT_PARITY_MARK: { dcb.Parity = MARKPARITY; break; }
			case SERIALPORT_PARITY_SPACE: { dcb.Parity = SPACEPARITY; break; }
			}
		switch (stopBits)
			{
			case SERIALPORT_STOPBITS_1: { dcb.StopBits = ONESTOPBIT; break; }
			case SERIALPORT_STOPBITS_1_5: { dcb.StopBits = ONE5STOPBITS; break; }
			case SERIALPORT_STOPBITS_2: { dcb.StopBits = TWOSTOPBITS; break; }
			}
		if (!SetCommState(_phandle->x , &dcb)) { close(); return -4; }
		COMMTIMEOUTS comTimeOut;
		comTimeOut.ReadIntervalTimeout = 5;
		comTimeOut.ReadTotalTimeoutMultiplier = 5;
		comTimeOut.ReadTotalTimeoutConstant = 5;
		comTimeOut.WriteTotalTimeoutMultiplier = 3;
		comTimeOut.WriteTotalTimeoutConstant = 2;
		if (!SetCommTimeouts(_phandle->x , &comTimeOut)) { close(); return -5; }
		if (!PurgeComm(_phandle->x , PURGE_RXCLEAR | PURGE_TXCLEAR)) { close(); return -6; }
		COMSTAT stat;
		DWORD error;
		if (!ClearCommError(_phandle->x , &error, &stat)) { close(); return -7; }
		if (error != 0) { close(); return -8; }
		return 0;
		}


	void SerialPort::close()
		{
		if (_phandle->x  == INVALID_HANDLE_VALUE) return;
		COMSTAT stat;
		DWORD error;
		ClearCommError(_phandle->x , &error, &stat);
		CloseHandle(_phandle->x );
		_phandle->x  = INVALID_HANDLE_VALUE;
		}


	bool SerialPort::clear()
		{
		if (!status()) return false;
		if (!PurgeComm(_phandle->x , PURGE_RXCLEAR | PURGE_TXCLEAR)) { close(); return false; }
		return true;
		}


	int SerialPort::read(char * buffer, size_t len)
		{
		if (_phandle->x  == INVALID_HANDLE_VALUE) return -1;
		COMSTAT stat;
		DWORD error;
		if (!ClearCommError(_phandle->x , &error, &stat)) { close(); return -2; }
		if (error != 0) { close(); return -3; }
		if (stat.cbInQue > 0)
			{
			if (len < (size_t)stat.cbInQue) { stat.cbInQue = (DWORD)len; }
			DWORD nbread = 0;
			if (!ReadFile(_phandle->x , (LPVOID)buffer, stat.cbInQue, &nbread, NULL)) { close(); return -4; }
			return((int)nbread);
			}
		return 0;
		}


	int SerialPort::available()
		{
		if (_phandle->x == INVALID_HANDLE_VALUE) return -1;
		COMSTAT stat;
		DWORD error;
		if (!ClearCommError(_phandle->x, &error, &stat)) { close(); return -2; }
		if (error != 0) { close(); return -3; }
		return (int)stat.cbInQue;
		}


	int SerialPort::write(const char * buffer, size_t len)
		{
		if (!status()) return -1;
		DWORD nbwritten = 0;
		if (!WriteFile(_phandle->x , buffer, (DWORD)len, &nbwritten, NULL)) { close(); return -2; }
		return((int)nbwritten);
		}


	bool SerialPort::status()
		{
		if (_phandle->x  == INVALID_HANDLE_VALUE) return false;
		COMSTAT stat;
		DWORD error;
		if (!ClearCommError(_phandle->x , &error, &stat)) { close(); return false; }
		if (error != 0) { close(); return false; }
		return true;
		}


	std::vector<std::string> SerialPort::getPortList()
		{
		std::vector<std::string> portList;
		const size_t buffer_size = 1024 * 1024; // 1MB 
		std::vector<char> buffer(buffer_size);
		if (!QueryDosDevice(NULL, buffer.data(), buffer_size)) return portList;
		size_t i = 0;
		do
			{
			std::string devname(buffer.data() + i);
			if ((devname.find("COM") == 0) || (devname.find("com") == 0)) { portList.push_back(devname); }
			i += devname.size() + 1;
			}
		while ((i < buffer_size) && (buffer[i] != 0));
		return portList;
		}


#endif // end of #ifdef _WIN32


	}


/* end of file */




