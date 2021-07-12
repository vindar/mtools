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


#pragma once

#include "../misc/internal/mtools_export.hpp"

#include <vector>
#include <memory>
#include <string>
#include <queue>
#include <array>

namespace mtools
	{


	/**
	* Simple class for performing serial communication
	*
	* Whenever an error occurs, the connexion is closed and must be re-opened to continue.
	* Use the static method getPortList() to list all available serial ports.
	**/
	class SerialPort
		{

		public:


			/** Default constructor. */
			SerialPort();



			/** Destructor. */
			~SerialPort();


			enum { SERIALPORT_PARITY_NONE, SERIALPORT_PARITY_ODD, SERIALPORT_PARITY_EVEN, SERIALPORT_PARITY_MARK, SERIALPORT_PARITY_SPACE };   ///< serial port parity types.

			enum { SERIALPORT_STOPBITS_1, SERIALPORT_STOPBITS_1_5, SERIALPORT_STOPBITS_2 }; ///< serial port stop bits configuration


			/**
			 * Connect to a serial port.
			 * 
			 * If a connection is already established, do nothing and return -1.
			 *
			 * @param	portName   	Name of the port (such as "COM11"), do not prepend it with "\\.\".
			 * @param	baudRate   	The baud rate.
			 * @param	parityCheck	true to check parity.
			 * @param	parity	   	parity to use (one of SERIALPORT_PARITY_NONE,  SERIALPORT_PARITY_ODD,
			 * 						SERIALPORT_PARITY_EVEN, SERIALPORT_PARITY_MARK, SERIALPORT_PARITY_SPACE)
			 * @param	stopBits   	The stop bits. (one of SERIALPORT_STOPBITS_1, SERIALPORT_STOPBITS_1_5,
			 * 						SERIALPORT_STOPBITS_2)
			 *
			 * @return	0 if the connexion is succesfully established or a negative value if the connexion failed. 
			 **/
			int open(std::string portName, int baudRate = 115200, bool parityCheck = false, int parity = SERIALPORT_PARITY_NONE, int stopBits = SERIALPORT_STOPBITS_1);


			/**
			* Try to reconnect on the same port open previously with open()
            * Use this only after calling open() and in case of connexion failure.
			**/
			int reconnect();


			/** Closes the serial port. */
			void close();


			/**
			* Clears the RX and TX buffers.
			*
			* @return	true if ok and false if an error occured.
			**/
			bool clear();


			/**
			 * Read incoming serial data. This method does not wait for data and return immediately if no
			 * data is available.
			 *
			 * @param [out]	buffer	buffer to receive the data.
			 * @param	len		  	buffer size.
			 *
			 * @return	Number of bytes written in the buffer or a negative number if an error occured.
			 **/
			int64_t read(char * buffer, int64_t len);


			/**
			* Read a single char, arduino like.
			**/
			inline int read()
				{
				if (available() > 0) 
					{
					uint8_t c = 0;
					read((char *)&c, 1);
					return c;
					}
				return -1;
				}



			/** Flushes any ongoing buffered data. */
			void flush();



			/**
			 * Query the number of bytes ready to be read.
			 *
			 * @return	The number of bytes available (>= 0) or a negative number if an error occurred.
			 **/
			int64_t available();


            /**
             * poll the serial port.
             * 
             * Similar to available() but tries to increase the number of received bytes even when the
             * number of bytes currently available is non-zero. Calling this too often instead of
             * available() may slow down the stream.
             *
             * @returns The number of bytes available (>= 0) or a negative number if an error occurred.
            **/
			int64_t poll();


			/**
			* Send serial data.
			*
			* @param	buffer	buffer containing the data to send.
			* @param	len   	buffer size
			*
			* @return	number of byte written (len if everything is ok).
			**/
			int64_t write(const char * buffer, int64_t len);


			/**
			* Write a single char. 
			**/
			inline int64_t write(char c)
				{
				return write(&c, 1);
				}


			/**
			* Query the connection status.
			*
			* @return	True if connected and false if disconnected.
			**/
			bool status();


			/**
			* Return a list of all available COM port.
			**/
			static std::vector<std::string> getPortList();


		private:



			/** for the receive queue */

			static const int64_t QUEUE_BUFFER_SIZE = 64 * 1024; // 64K buffers

			std::queue<std::array<char, QUEUE_BUFFER_SIZE>*  > _queue;	// the queue
			int64_t _queuesize;	// number of bytes available
			int64_t ind_queue_read; // read index in the current (front) array
			int64_t ind_queue_write; // write index in the current (back) array

			/** add a buffer to the queue */
			void _pushtoqueue(const char* buffer, int64_t len);
		
			/** remove a buffer from the queue */
			void _popfromqueue(char* dest, int64_t len);
	
			/** clear the queue completely */
			void _clearqueue();

			/** return the number of bytes available in the queue */
			int64_t _queueavail() const { return _queuesize; }


			struct SerialPortHandle; // forward declaration

			std::unique_ptr<SerialPortHandle> _phandle;

			/* no copy */
			SerialPort(const SerialPort &) = delete;
			SerialPort & operator=(const SerialPort &) = delete;

			std::string	_portname;
			int _baudrate;
			bool _paritycheck;
			int _parity;
			int _stopbits;

		};







	}


/* end of file */




