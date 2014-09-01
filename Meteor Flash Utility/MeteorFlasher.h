#pragma once

using namespace System;
using namespace System::IO;
using namespace System::IO::Ports;
using namespace System::Threading;

namespace MeteorFlashUtility
{

// This class handles user communication with a Kreyos Meteor's Bootstrap Loader (BSL) module.
public ref class MeteorFlasher
{

	// Methods
public:
	// Default constructor
	MeteorFlasher()
	{
		ErrorWriter = Console::Error;
		BaudRate = 9600;
		Parity = Parity::Even;
		StopBits = StopBits::One;
		DataBits = 8;
		Handshake = Handshake::None;
		ReadTimeout = 1000;
		WriteTimeout = 1000;
		Port = gcnew SerialPort();
		Port->DataReceived += gcnew SerialDataReceivedEventHandler(DataReceivedHandler);
		Port->ErrorReceived += gcnew SerialErrorReceivedEventHandler(ErrorReceivedHandler);
	}

	// Destructor
	~MeteorFlasher()
	{
		if (Connected)
			Port->Close();
	}

	void Test()
	{
		Synchronize();
	}

	// Attempts to connect to the named port
	bool Connect(String^ _portName)
	{
		if (Connected)
		{
			throw gcnew MeteorFlashUtility::AlreadyConnectedException(String::Format("You already are connected to {0}", PortName));
		}
		else
		{
			if (String::IsNullOrEmpty(_portName))
			{
				ReportError("Port name not specified!");
			}
			else
			{
				// Set configurations
				Port->BaudRate = BaudRate;
				Port->Parity = Parity;
				Port->StopBits = StopBits;
				Port->DataBits = DataBits;
				Port->Handshake = Handshake;
				Port->ReadTimeout = ReadTimeout;
				Port->WriteTimeout = WriteTimeout;
				PortName = _portName;
				Port->PortName = _portName;

				// Open the port
				try
				{
					Port->Open();
				}
				catch (TimeoutException ^)
				{
					ReportError("Timed out trying to connect!");
					Connected = false;
				}

				Connected = true;
			}
		}

		return Connected;
	}

	// Executes the Transmit BSL Version command. Unprotected command.
	bool TransmitBSLVersion(array<unsigned char> ^ &ResponseData)
	{
		bool success = true;
		Synchronize();
		Message[0] = DATA_HEADER;
		Message[1] = CMD_TX_VERSION;
		Message[2] = 0x04;
		Message[3] = 0x04;
		for (int i = 4; i < 8; i++)
			Message[i] = 0;
		
		if (!WriteMessage(8, NULL, 0))
		{
			ReportError("Error writing to device!");
			return false;
		}
		
		if (!ReadMessage(4, 16))
		{
			ReportError("Error reading from device!");
			return false;
		}

		// Do check on received data
		if (ResponseCmd[0] != DATA_HEADER ||
			ResponseCmd[2] != 0x10 ||
			ResponseCmd[3] != 0x10)
		{
			ReportError(String::Format("Received malformed response command byte string. {0:X} {1:X} {2:X} {3:X}", ResponseCmd[0], ResponseCmd[1], ResponseCmd[2], ResponseCmd[3]));
			success = false;
		}

		unsigned char chksumLow, chksumHigh;
		CalculateChecksum(ResponseData, 16, chksumLow, chksumHigh);
		if (chksumLow != ResponseChksum[0] || chksumHigh != ResponseChksum[1])
		{
			ReportError(String::Format("Checksum mismatch! Possible data corruption."));
			success = false;
		}

		return success;
	}

private:
	// Reports an error on the error stream
	static void ReportError(String^ message)
	{
		ErrorWriter->WriteLine(message);
	}

	// Inititalizes device and puts it into BSL mode
	void InitializeBSL()
	{
		// here we need to toggle the TEST and RST pins
		// Look at BSL_IO_UART.c in invokeBSL() function for example
	}

	// Performs the device Synchronization sequence. Must be done first before every command.
	bool Synchronize()
	{
		Message[0] = DATA_SYNC;
		for (int i = 1; i < 8; i++)
			Message[i] = 0;

		try
		{
			Port->Write(Message, 0, 1);
		}
		catch (TimeoutException ^)
		{
			ReportError("Write timeout occurred during SYNC!");
			return false;
		}

		try
		{
			unsigned char response = Port->ReadChar();

			if (response == DATA_NACK)
			{
				ReportError("Error reported by device during SYNC");
				return false;
			}
			else if (response != DATA_ACK)
			{
				ReportError(String::Format("Unexpected response from device! {0:X}", response));
				return false;
			}
		}
		catch (TimeoutException ^)
		{
			ReportError("Timeout occurred while waiting for response during SYNC!");
			return false;
		}

		return true;
	}

	// Writes the message to the device (with specified number of bytes)
	// cmdSize is always 8. It should only ever be 1 if performing a Syncronization.
	// data is data to transmit with command if required.
	// dataSize is the number of bytes of data. Set to 0 if no data is associated with the command.
	bool WriteMessage(unsigned int cmdSize, unsigned char data[], unsigned int dataSize)
	{
		try
		{
			if (dataSize == 0)
			{
				Port->Write(Message, 0, cmdSize);
			}
			else
			{
				array<unsigned char> ^ fullMessage = gcnew array<unsigned char>(10 + dataSize);
				for (unsigned int i = 0; i < 8; i++)
					fullMessage[i] = Message[i];

				for (unsigned int i = 8; i < dataSize; i++)
					fullMessage[i] = data[i - 8];

				fullMessage[8 + dataSize] = Chksum[0];
				fullMessage[9 + dataSize] = Chksum[1];

				Port->Write(Message, 0, 10 + dataSize);
			}
		}
		catch (TimeoutException ^)
		{
			ReportError("Timeout occurred while writing to serial device!");
			return false;
		}

		return ReceiveAck();
	}

	// Event handler for receiving data
	static void DataReceivedHandler(Object^ sender, SerialDataReceivedEventArgs^ e)
	{
		SerialPort^ sp = (SerialPort^)sender;
		String^ indata = sp->ReadExisting();
		Console::WriteLine("Data Received:");
		Console::Write(indata);
	}

	// Event handler for error
	static void ErrorReceivedHandler(Object^ sender, SerialErrorReceivedEventArgs^ e)
	{
		SerialPort^ sp = (SerialPort^)sender;
		switch (e->EventType)
		{
		case SerialError::Frame:
			ReportError("Framing error detected!");
			break;
		case SerialError::Overrun:
			ReportError("Character buffer overrun detected!");
			break;
		case SerialError::RXOver:
			ReportError("Input buffer overflow detected!");
			break;
		case SerialError::RXParity:
			ReportError("Parity error detected!");
			break;
		case SerialError::TXFull:
			ReportError("Attempt to transmit but output buffer is full!");
			break;
		}
	}

	// Reads back the ack byte from device
	bool ReceiveAck()
	{
		try
		{
			ResponseAck = Port->ReadChar();
		}
		catch (TimeoutException ^)
		{
			ReportError("Timeout occurred while waiting for ACK from device!");
			return false;
		}

		if (ResponseAck == DATA_NACK)
		{
			ReportError("Error reported by device!");
		}
		else if (ResponseAck != DATA_ACK)
		{
			ReportError(String::Format("Unexpected response from device! {0:X}", ResponseAck));
		}
		return true;
	}

	// Does a raw serial read of command + data if numDataBytes > 0.
	bool ReadMessage(unsigned int numCmdBytes, unsigned int numDataBytes)
	{
		array<unsigned char> ^ fullResponse = gcnew array<unsigned char>(numCmdBytes + numDataBytes + 2);
		try
		{
			Port->Read(fullResponse, 0, numCmdBytes + numDataBytes + 2);
		}
		catch (TimeoutException ^)
		{
			ReportError("Timeout occurred while reading from serial device!");
			return false;
		}

		for (unsigned int i = 0; i < numCmdBytes + numDataBytes + 2; i++)
		{
			if (i < numCmdBytes)
				ResponseCmd[i] = fullResponse[i];
			else if ((numDataBytes > 0) && (i < numCmdBytes + numDataBytes))
			{
				ResponseData = gcnew array<unsigned char>(numDataBytes);
				ResponseData[i - numCmdBytes] = fullResponse[i];
			}
			else
				ResponseChksum[i - numCmdBytes - numDataBytes] = fullResponse[i];
		}

		return true;
	}

	// Calculates the checksum on an array of data. Returns low and high values on chksumLow and chksumHigh
	// Formula:
	//    CKL = INV [ B1 XOR B3 XOR … XOR Bn–1 ]
	//    CKH = INV[B2 XOR B4 XOR … XOR Bn]
	void CalculateChecksum(array<unsigned char> ^ data, unsigned int dataSize, unsigned char &chksumLow, unsigned char &chksumHigh)
	{
		unsigned char cLow, cHigh;
		for (unsigned int i = 0; i < dataSize; i++)
		{
			if (i == 0)
				cLow = data[i];
			else if (i < dataSize - 1)
				cLow = cLow ^ data[i];

			if (i == 1)
				cHigh = data[i];
			else if (i > 1)
				cHigh = cHigh ^ data[i];
		}
		chksumLow = ~cLow;
		chksumHigh = ~cHigh;
	}


	// Types, constants, and data members
private:
	static System::IO::TextWriter^ ErrorWriter;


private:
	array<unsigned char> ^ Message = gcnew array<unsigned char>(8);
	array<unsigned char> ^ Chksum = gcnew array<unsigned char>(2);
	array<unsigned char> ^ ResponseCmd = gcnew array<unsigned char>(8);
	array<unsigned char> ^ ResponseChksum = gcnew array<unsigned char>(2);
	array<unsigned char> ^ ResponseData;
	unsigned char ResponseAck;
	SerialPort^ Port;
	bool Connected = false;
	bool Initialized = false;

private:
	static const unsigned char DATA_SYNC = 0x80;
	static const unsigned char DATA_ACK = 0x90;
	static const unsigned char DATA_NACK = 0xA0;
	static const unsigned char DATA_HEADER = 0x80;

public:
	static const ProtectedCmd CMD_RX_DATA = 0x12, CMD_ERASE = 0x16, CMD_ERASE_CHECK = 0x1C, CMD_SET_OFFSET = 0x21, CMD_LOAD_PC = 0x1A, CMD_TX_DATA = 0x14;
	static const UnprotectedCmd CMD_RX_PASSWD = 0x10, CMD_MASS_ERASE = 0x18, CMD_TX_VERSION = 0x1E;

	unsigned int BaudRate;
	Ports::Parity Parity;
	Ports::StopBits StopBits;
	unsigned int DataBits;
	Ports::Handshake Handshake;
	unsigned int ReadTimeout;
	unsigned int WriteTimeout;
	String^ PortName;

};

}
