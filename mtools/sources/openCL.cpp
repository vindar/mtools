/** @file openCL.cpp */
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

#include "extensions/openCL.hpp"

// only if openCL is installed.
#if (MTOOLS_USE_OPENCL)

#include "misc/error.hpp"
#include "io/console.hpp"
#include "io/fileio.hpp"




namespace mtools
	{


	cl::Platform openCL_selectPlatform(bool selectdefault, bool output, bool showextensions)
		{
		try
			{
			std::vector<cl::Platform> platformList;		// list of all installed platforms
			cl::Platform::get(&platformList);
			if (platformList.size() == 0) { MTOOLS_ERROR("No OpenCL platform found."); }
			if ((platformList.size() > 1) && (!selectdefault)) { output = true; }
			if (output)
				{
				mtools::cout << "-----------------------------------------------\n";
				mtools::cout << "OpenCL platforms found: " << platformList.size() << "\n\n";
				for (int i = 0; i < (int)platformList.size(); ++i)
					{
					mtools::cout << "(" << i << ") Platform name: [" << mtools::troncateAfterNullChar(platformList[i].getInfo<CL_PLATFORM_NAME>()) << "]\n";
					mtools::cout << "           vendor: [" << mtools::troncateAfterNullChar(platformList[i].getInfo<CL_PLATFORM_VENDOR>()) << "]\n";
					mtools::cout << "          version: [" << mtools::troncateAfterNullChar(platformList[i].getInfo<CL_PLATFORM_VERSION>()) << "]\n";
					mtools::cout << "          profile: [" << mtools::troncateAfterNullChar(platformList[i].getInfo<CL_PLATFORM_PROFILE>()) << "]\n";
					if (showextensions) { mtools::cout << "       extensions: [" << mtools::troncateAfterNullChar(platformList[i].getInfo<CL_PLATFORM_EXTENSIONS>()) << "]\n"; }
					mtools::cout << "\n";
					}
				}
			if ((selectdefault) || (platformList.size() == 1))
				{
				auto platform = cl::Platform::getDefault();
				if (output) { mtools::cout << "Selecting platform : [" << mtools::troncateAfterNullChar(platform.getInfo<CL_PLATFORM_NAME>()) << "]\n"; }
				return platform;
				}
			int i = -1;
			while ((i < 0) || (i >= (int)platformList.size())) { i = mtools::cout.ask("Which platform do you want to use ? ", 0); }
			mtools::cout << "Selecting platform : [" << mtools::troncateAfterNullChar(platformList[i].getInfo<CL_PLATFORM_NAME>()) << "]\n";
			return platformList[i];
			}
		catch (const cl::Error & e) { MTOOLS_ERROR(std::string("OpenCL error :[") + e.what() + "]\n"); }
		throw ""; // used to remove warning
		}


	cl::Device openCL_selectDevice(const cl::Platform  & platform, bool selectdefault, bool output, bool showextensions)
		{
		try {
			std::vector<cl::Device> deviceList;
			platform.getDevices(CL_DEVICE_TYPE_GPU, &deviceList);
			if (deviceList.size() == 0) { MTOOLS_ERROR("No GPU device found on the openCL platform."); }
			if ((deviceList.size() > 1) && (!selectdefault)) { output = true; }
			if (output)
				{
				mtools::cout << "-----------------------------------------------\n";
				mtools::cout << "GPU device found: " << deviceList.size() << "\n\n";
				for (int i = 0; i < (int)deviceList.size(); ++i)
					{
					mtools::cout << "(" << i << ")        name: [" << mtools::troncateAfterNullChar(deviceList[i].getInfo<CL_DEVICE_NAME>()) << "]\n";
					mtools::cout << "       mem size: [" << deviceList[i].getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / (1024 * 1024) << "Mo]\n";
					mtools::cout << "      available: [" << (int)deviceList[i].getInfo<CL_DEVICE_AVAILABLE>() << "]\n";
					if (showextensions) { mtools::cout << "     extensions: [" << mtools::troncateAfterNullChar(deviceList[i].getInfo<CL_DEVICE_EXTENSIONS>()) << "]\n"; }
					mtools::cout << "\n";
					}
				}
			int i = -1;
			if ((selectdefault) || (deviceList.size() == 1)) { i = 0; }
			while ((i < 0) || (i >= (int)deviceList.size())) { i = mtools::cout.ask("Which device do you want to use ? ", 0); }
			if ((int)deviceList[i].getInfo<CL_DEVICE_AVAILABLE>() == 0) { MTOOLS_ERROR("The openCL device selected is not available"); }
			if (output) { mtools::cout << "Selecting device : [" << mtools::troncateAfterNullChar(deviceList[i].getInfo<CL_DEVICE_NAME>()) << "]\n"; }
			return deviceList[i];
			}
		catch (const cl::Error & e) { MTOOLS_ERROR(std::string("OpenCL error :[") + e.what() + "]\n"); }
		throw ""; // used to remove warning
		}


	cl::Context openCL_createContext(const cl::Device & device, bool output)
		{
		try {
			cl::Context context(device, 0, 0, 0);
			if (output)
				{
				mtools::cout << "-----------------------------------------------\n";
				mtools::cout << "OpenCL context created\n";
				}
			return context;
			}
		catch (const cl::Error & e) { MTOOLS_ERROR(std::string("OpenCL error :[") + e.what() + "]\n"); }
		throw ""; // used to remove warning
		}


	cl::CommandQueue openCL_createQueue(const cl::Device & device, const cl::Context & context, bool output)
		{
		try {
			cl::CommandQueue queue(context, device, CL_QUEUE_PROFILING_ENABLE);
			if (output)
				{
				mtools::cout << "-----------------------------------------------\n";
				mtools::cout << "Command queue (with profiling) created\n";
				}
			return queue;
			}
		catch (const cl::Error & e) { MTOOLS_ERROR(std::string("OpenCL error :[") + e.what() + "]\n"); }
		throw ""; // used to remove warning
		}




	OpenCLBundle::OpenCLBundle(bool selectdefault, bool output, bool showextensions)
		{
		try {
			platform = openCL_selectPlatform(selectdefault, output, showextensions);
			device = openCL_selectDevice(platform, selectdefault, output, showextensions);
			context = openCL_createContext(device, output);
			queue = openCL_createQueue(device, context, output);
			}
		catch (const cl::Error & e) { MTOOLS_ERROR(std::string("OpenCL error :[") + e.what() + "]\n"); }
		throw ""; // used to remove warning
		}


	cl::Program OpenCLBundle::createProgramFromFile(const std::string & filename, std::string compileroptions, bool output)
		{
		try
			{
			if (output)
				{
				mtools::cout << "-----------------------------------------------\n";
				mtools::cout << "Building program : [" << filename << "]\n";
				mtools::cout << "    with options : [" << compileroptions << "]\n";
				}
			std::string text = mtools::loadStringFromFile(filename);
			if (text.length() == 0) { MTOOLS_ERROR(std::string("error loading file [") + filename + "]"); }
			cl::Program prog(context, text);
			std::vector<cl::Device> listdevice; listdevice.push_back(device);
			try {
				prog.build(listdevice, compileroptions.c_str());
				}
			catch (const cl::Error & e)
				{
				std::string textlog = mtools::troncateAfterNullChar(prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
				mtools::saveStringToFile(filename + ".log", textlog);
				if (output)
					{
					mtools::cout << "*** build error ***\nlog file:\n";
					mtools::cout << textlog << "\n\n";
					mtools::cout.getKey();
					}
				MTOOLS_ERROR(std::string("OpenCL error [") + e.what() + "]\nwhile building file [" + filename + "]\nwith options [" + compileroptions + "]\n");
				}
			std::string textlog = mtools::troncateAfterNullChar(prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
			mtools::saveStringToFile(filename + ".log", textlog);
			if (output)
				{
				mtools::cout << "Build successful.\n";
				mtools::cout << "Compiler log:\n" << textlog << "\n";
				}
			return prog;
			}
		catch (const cl::Error & e) { MTOOLS_ERROR(std::string("OpenCL error :[") + e.what() + "]\n"); }
		throw ""; // used to remove warning
		}


	cl::Program OpenCLBundle::createProgramFromString(const std::string & source, std::string & log, std::string compileroptions, bool output)
		{
		try
			{
			if (output)
				{
				mtools::cout << "-----------------------------------------------\n";
				mtools::cout << "Building program from a std::string\n";
				mtools::cout << "    with options : [" << compileroptions << "]\n";
				}
			if (source.length() == 0) { MTOOLS_ERROR("error empty source string"); }
			cl::Program prog(context, source);
			std::vector<cl::Device> listdevice; listdevice.push_back(device);
			try {
				prog.build(listdevice, compileroptions.c_str());
				}
			catch (const cl::Error & e)
				{
				log = mtools::troncateAfterNullChar(prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
				mtools::cout << "*** build error ***\nlog file:\n";
				mtools::cout << log << "\n\n";
				mtools::cout.getKey();
				MTOOLS_ERROR(std::string("OpenCL error [") + e.what() + "]\nwhile building program from std::string\nwith options [" + compileroptions + "]\n");
				}
			log = mtools::troncateAfterNullChar(prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
			if (output)
				{
				mtools::cout << "Build successful.\n";
				mtools::cout << "Compiler log:\n" << log << "\n";
				}
			return prog;
			}
		catch (const cl::Error & e) { MTOOLS_ERROR(std::string("OpenCL error :[") + e.what() + "]\n"); }
		throw ""; // used to remove warning
		}


	cl::Kernel OpenCLBundle::createKernel(cl::Program & prog, const std::string & kernelName, bool output)
		{
		try {
			if (output)
				{
				mtools::cout << "-----------------------------------------------\n";
				mtools::cout << "Extracting kernel : [" << kernelName << "]\n";
				}
			cl::Kernel kernel(prog, kernelName.c_str());
			if (output) { mtools::cout << "Extraction successful.\n"; }
			return kernel;
			}
		catch (const cl::Error & e) { MTOOLS_ERROR(std::string("OpenCL error :[") + e.what() + "]\n"); }
		throw ""; // used to remove warning
		}



	}


#endif

/* end of file */

