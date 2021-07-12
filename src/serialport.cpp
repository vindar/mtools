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



#include "mtools/misc/error.hpp"

#include "io/serialport.hpp"

#ifdef _WIN32 
#include <windows.h>
#endif 


#include "mtools/io/console.hpp"


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
				_clearqueue();
				_phandle->x = INVALID_HANDLE_VALUE;
				}


	SerialPort::~SerialPort()
				{
				close();
				}


	int SerialPort::reconnect()
		{
		close();
		std::string portName = std::string("\\\\.\\") + _portname;
		_phandle->x = CreateFile(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, NULL, NULL);
		if (_phandle->x == INVALID_HANDLE_VALUE) { return -2; }
		DCB dcb;
		memset(&dcb, 0, sizeof(dcb));
		dcb.DCBlength = sizeof(dcb);
		if (!GetCommState(_phandle->x , &dcb)) { close(); return -3; }
		dcb.BaudRate = (DWORD)_baudrate;
		dcb.ByteSize = 8;
		dcb.fParity = (_paritycheck) ? 1 : 0;
		dcb.fDtrControl = DTR_CONTROL_ENABLE;  // DTR flow control
		dcb.fRtsControl = RTS_CONTROL_ENABLE;  // RTS flow control
		dcb.fBinary = 1;
		dcb.fAbortOnError = TRUE;        // Do not abort reads/writes on error
		switch (_parity)
			{
			case SERIALPORT_PARITY_NONE: { dcb.Parity = NOPARITY; break; }
			case SERIALPORT_PARITY_ODD: { dcb.Parity = ODDPARITY; break; }
			case SERIALPORT_PARITY_EVEN: { dcb.Parity = EVENPARITY; break; }
			case SERIALPORT_PARITY_MARK: { dcb.Parity = MARKPARITY; break; }
			case SERIALPORT_PARITY_SPACE: { dcb.Parity = SPACEPARITY; break; }
			}
		switch (_stopbits)
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


	int SerialPort::open(std::string portName, int baudRate, bool parityCheck, int parity, int stopBits)
		{
		if (_phandle->x != INVALID_HANDLE_VALUE) { return -1; }
		_portname = portName;
		_baudrate = baudRate;
		_paritycheck = parityCheck;
		_parity = parity;
		_stopbits = stopBits;
		return reconnect();
		}


	void SerialPort::close()
		{
		_clearqueue();
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



	void SerialPort::flush()
		{
		FlushFileBuffers(_phandle->x);
		}




	int64_t SerialPort::read(char * buffer, int64_t len)
		{
		if (_phandle->x == INVALID_HANDLE_VALUE) return -1;
		if (_queueavail() == 0) poll();
		if (len < 0) return -1;
		if (len == 0) return 0;
		const int64_t av = _queueavail();
		const int64_t l = (av < len) ? av : len;
		_popfromqueue(buffer, l);
		return l;
		}


	int64_t SerialPort::poll()
		{
		if (_phandle->x == INVALID_HANDLE_VALUE) return -1;
		const int buf_size = 32*1024;
		char buf[buf_size];
		while (1)
			{
			COMSTAT stat;
			stat.cbInQue = 0;
			DWORD error;
			if (!ClearCommError(_phandle->x, &error, &stat)) { close(); return -2; }
			if (error != 0) { close(); return -2; }
			int av = stat.cbInQue;
			if (av <= 0) return _queueavail();
			while (av > 0)
				{
				const int tr = (av > buf_size) ? buf_size : av;
				DWORD nbread = 0;
				if (!ReadFile(_phandle->x, (LPVOID)buf, tr, &nbread, NULL)) { close(); return -2; }
				if (nbread == 0) return _queueavail();
				if (nbread > 0) _pushtoqueue(buf, nbread);				
				av -= nbread;
				}
			}
		}


	int64_t SerialPort::available()
		{
		if (_phandle->x == INVALID_HANDLE_VALUE) return -1;
		if (_queueavail() == 0) poll();
		return _queueavail();		
		}


	int64_t SerialPort::write(const char * buffer, int64_t len)
		{
		if (!status()) return -1;
		DWORD nbwritten = 0;
		if (!WriteFile(_phandle->x , buffer, (DWORD)len, &nbwritten, NULL)) { close(); return -2; }
		return((int64_t)nbwritten);
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



	void  SerialPort::_pushtoqueue(const char* buffer, int64_t len)
		{
		if ((len <= 0) || (buffer == nullptr)) return;
		_queuesize += len;
		while (len > 0)
			{
			const int64_t avail = QUEUE_BUFFER_SIZE - ind_queue_write; // available in the current array
			if (avail <= 0)
				{
				MTOOLS_INSURE(avail == 0);
				_queue.push(new std::array<char, QUEUE_BUFFER_SIZE>);
				ind_queue_write = 0;
				}
			else
				{
				const int64_t w = (len > avail) ? avail : len; // number of bytes we can write
				auto pa = _queue.back();
				memcpy(pa->data() + ind_queue_write, buffer, w);
				ind_queue_write += w;
				len -= w;
				buffer += w;
				}
			}
		}


	void  SerialPort::_popfromqueue(char* dest, int64_t len)
		{
		MTOOLS_INSURE(dest != nullptr);
		MTOOLS_INSURE(len <= _queuesize);
		_queuesize -= len;
		while (len > 0)
			{
			const int64_t avail = QUEUE_BUFFER_SIZE - ind_queue_read; // available in the current array
			if (avail <= 0)
				{
				MTOOLS_INSURE(avail == 0);
				MTOOLS_INSURE(_queue.size() > 1);
				auto ar = _queue.front();
				delete ar;
				_queue.pop();
				ind_queue_read = 0;
				}
			else
				{
				const int64_t r = (len > avail) ? avail : len; // number of bytes we can read
				auto pa = _queue.front();
				memcpy(dest, pa->data() + ind_queue_read, r);
				ind_queue_read += r;
				len -= r;
				dest += r;
				}
			}
		}


	void SerialPort::_clearqueue()
		{
		while (_queue.size() > 0)
			{
			auto ar = _queue.front();
			delete ar;
			_queue.pop();
			}
		_queue.push(new std::array<char, QUEUE_BUFFER_SIZE>);
		_queuesize = 0;
		ind_queue_read = 0;
		ind_queue_write = 0;
		}



#endif // end of #ifdef _WIN32


	}


/* end of file */




