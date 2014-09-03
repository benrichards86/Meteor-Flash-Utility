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

	flasher->Test();

#if 0
	// Here, issue a the version check command
	array<unsigned char> ^ VersionBytes;
	if (!flasher->TransmitBSLVersion(VersionBytes))
	{
		Console::Error->WriteLine("Error retrieving version info from BSL!");
	}
#endif
	return 0;
}

