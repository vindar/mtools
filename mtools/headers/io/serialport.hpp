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


#include <vector>
#include <memory>


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
			int read(char * buffer, size_t len);


			/**
			 * Query the number of bytes ready in the RX buffer. 
			 *
			 * @return	The number of bytes available (>= 0) or a negative number if an error occured.
			 **/
			int available();


			/**
			* Send serial data.
			*
			* @param	buffer	buffer containing the data to send.
			* @param	len   	buffer size
			*
			* @return	number of byte written (len if everything is ok).
			**/
			int write(const char * buffer, size_t len);


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

			
			struct SerialPortHandle; // forward declaration

			std::unique_ptr<SerialPortHandle> _phandle;

			/* no copy */
			SerialPort(const SerialPort &) = delete;
			SerialPort & operator=(const SerialPort &) = delete;

		};







	}


/* end of file */




