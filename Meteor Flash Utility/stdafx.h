// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <windows.h>

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#define NAME "Meteor Flash Utility (unofficial)"
#define VERSION "0.1"
#define PrintVersion(name,version) 	Console::WriteLine("{0} ver. {1}", (name), (version))

typedef unsigned char UnprotectedCmd;
typedef unsigned char ProtectedCmd;

#using <System.dll>

#include "CP210xRuntime\CP210xRuntimeDLL.h"

#include "Exceptions.h"
#include "MeteorFlasher.h"
