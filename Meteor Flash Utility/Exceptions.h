#pragma once

using namespace System;

namespace MeteorFlashUtility
{

	public ref class AlreadyConnectedException : System::Exception
	{
	public:
		static String^ AlreadyConnectedMessage = "Connect attempted while already connected to the device!";

		AlreadyConnectedException(String^ auxMessage)
			: Exception(String::Format("{0} - {1}", AlreadyConnectedMessage, auxMessage))
		{
		}

	};

}