//	Meteor Flash Utility - A firmware flasher for Kreyos Meteor 
//	Copyright(C) 2014  Benjamin Richards
//
//	This program is free software : you can redistribute it and / or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.If not, see <http://www.gnu.org/licenses/>.

// Meteor Flash Utility.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace MeteorFlashUtility;
using namespace System;
using namespace System::IO::Ports;

int _tmain(int argc, _TCHAR* argv[])
{
	PrintVersion(NAME, VERSION);
	MeteorFlasher^ flasher = gcnew MeteorFlasher();

	bool connected = false;
	while (!connected)
	{
		String^ portName;

		Console::WriteLine("Ports Available:");
		for each (String^ s in SerialPort::GetPortNames())
		{
			Console::WriteLine("   {0}", s);
			// Set portName to first port found, as a default selection
			if (!portName)
				portName = String::Copy(s);
		}

		// Null if no ports found. Quit.
		if (!portName)
		{
			Console::Error->WriteLine("No serial devices found!\n");
			return 1;
		}

		Console::Write("Com Port {0}: ", portName);
		String^ selection = Console::ReadLine();
		if (!String::IsNullOrEmpty(selection))
			portName = selection;

		connected = flasher->Connect(portName);

		if (!connected)
		{
			Console::Error->WriteLine("Could not connect.");
		}
	}

#if 1
	// Here, issue a the version check command
	array<unsigned char> ^ VersionBytes;
	if (!flasher->TransmitBSLVersion(VersionBytes))
	{
		Console::Error->WriteLine("Error retrieving version info from BSL!");
	}
#endif
	return 0;
}

