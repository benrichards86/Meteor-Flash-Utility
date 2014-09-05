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