
	#ifndef PRCORE_CONFIGURE_HPP
	#error "NEVER manually include this header."
	#endif

	#define PRCORE_CONFIGURED

	#error BeOS headers out-of-date, e-mail: support@twilight3d.com

	/*
	The BeOS configuration for reference purposes for potential future updates:
	*/
	/*

	#include <SupportDefs.h>
	#include <string.h>
	#include <stdlib.h>

	// interface
	#define PREXPORT __declspec(dllexport)
	#define PRIMPORT __declspec(dllimport)
	#define PRCALL

	#ifdef PR_COMPILE_RUNTIME
	#define PRAPI PREXPORT
	#else
	#define PRAPI PRIMPORT
	#endif

	#include "../src/extlib/openexr/Half/half.h"
	typedef half  float16;
	typedef float float32;

	#define max max_c
	#define min min_c

	#ifdef PRMAIN
	#include <Application.h>
	#include <Entry.h>
	#include <Path.h>
	#include <Roster.h>

	struct Pr3DArgs
	{
		int argc;
		char **argv;
	};

	static int32 PrThreadWrapper(Pr3DArgs *args);
	
	class PRApplication : public BApplication
	{
		public:

		PRApplication(Pr3DArgs *a)
		: BApplication("application/x-vnd.ProphecyApp")
		{
			args = a;	
		}

		~PRApplication()
		{
		}

		void ReadyToRun()
		{
			thread_id prTID;
			prTID = spawn_thread( PrThreadWrapper, "Prophecy application thread", B_DISPLAY_PRIORITY, (void *)args );	
			resume_thread(prTID);
		}

		private:
		Pr3DArgs *args;
	};

	extern int prMain(int argc, char** argv);
	static BApplication *prApp;
	static int32 PrThreadWrapper(Pr3DArgs *args)
	{
		int32 rc = PrMain( args->argc, args->argv );	
		prApp->Lock();
		prApp->Quit();
		return rc;
	}

	int main(int argc, char *argv[])
	{
		Pr3DArgs *args = new Pr3DArgs;

		args->argc = argc;
		args->argv = argv;

		prApp = new PRApplication(args);
		prApp->Run();
		delete args;
		return 1;
	}

	#endif

	// flags
	#define PR_LITTLE_ENDIAN
	#define PR_X86_SIMD

	// types
	typedef signed long long	int64;
	typedef unsigned long long	uint64;
	typedef unsigned short		uchar16; // TODO: unicode char

	struct AccelMode
	{
		int width;
		int height;
		int depth;
		int color;
		int stencil;
		int accum;
	};
	
	struct AccelDevice
	{
		int dev_id;
		int monitor;
		bool depth;
		bool stencil;
		bool accum;
		AccelMode* modes;
		int numModes;
	};
	
	struct AccelDeviceInfo
	{
		AccelDevice *devs;
		int numDevs;
	};
	
	// window handle
	struct WinHandle
	{
		BDirectWindow*		hwnd;
		//AccelDeviceInfo*	devs;
	};

	*/
