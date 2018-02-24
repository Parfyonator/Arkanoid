/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.


    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.


	source:
		win32 joystick

	revision history:
		Jul/20/2000 - Timo Saarinen - initial revision
		Jul/22/2002 - Mikko Nurmi - fixed problem if there is no joystick installed, added POV support

*/
#include <cstddef>
#include <prcore/configure.hpp>
#include <prcore/exception.hpp>
#include <prcore/joystick.hpp>

#define DIRECTINPUT_VERSION 0x700
#include <dinput.h>

using namespace prcore;


// =================================================
// linker
// =================================================

#pragma comment(lib,"dinput")


// =================================================
// locals
// =================================================

namespace
{

	#define RELEASE( x ) if ( x != NULL ) { x->Release(); x = NULL; }

	LPDIRECTINPUT				pDirectInput = 0;
	Array<LPDIRECTINPUTDEVICE2>	devices;	
	Array<String>				deviceNames;

	// absolute maximums
	const int MAX_BUTTONS	= 64;
	const int MAX_AXES		= 16;
	const int MAX_POVS		= 8;

	// range
	const float JOY_RANGE	= 8192.0f;

	// data format for device data (note: must be dword multiple in size)
	typedef struct
	{
		LONG	axis[ MAX_AXES ];
		BYTE	button[ MAX_BUTTONS ];
		LONG	pov[ MAX_POVS ];
	} DeviceData;

	
	//
	//	object to make sure DirectInput objects are released (after the application terminates)
	//
	class Uniniter
	{
	public:
		~Uniniter()
		{
			if( pDirectInput )
			{
				for( int n=0; n < devices.GetSize(); n++ )
				{
					RELEASE( devices[n] );
				}
				RELEASE( pDirectInput );
				pDirectInput = 0;
			}
		}
	} uniniter;


	//
	//	DirectInput device enumeration callback
	//
	BOOL FAR PASCAL EnumJoystick(LPCDIDEVICEINSTANCE i, LPVOID v)
	{
		switch ( i->dwDevType )
		{
			case DIDEVTYPEJOYSTICK_TRADITIONAL:		// A traditional joystick.
			case DIDEVTYPEJOYSTICK_FLIGHTSTICK:		// A joystick optimized for flight simulation.
			case DIDEVTYPEJOYSTICK_GAMEPAD:			// A device whose primary purpose is to provide button input.
			case DIDEVTYPEJOYSTICK_RUDDER:			// A device for yaw control.
			case DIDEVTYPEJOYSTICK_WHEEL:			// A steering wheel.
			case DIDEVTYPEJOYSTICK_HEADTRACKER:		// A device that tracks the movement of the luser's head
			case DIDEVTYPEJOYSTICK_UNKNOWN:
			default:
				// do nothing for now..
				break;
		}

		LPDIRECTINPUTDEVICE device = 0;
		if( pDirectInput->CreateDevice( i->guidInstance, &device, NULL ) == DI_OK )
		{
			LPDIRECTINPUTDEVICE2 device2;
			if( device->QueryInterface( IID_IDirectInputDevice2, (LPVOID *)&device2 ) == DI_OK )
			{
				// device is ok, add to the list
				devices.PushBack( device2 );
				deviceNames.PushBack( i->tszInstanceName );	// TODO: or i->tszProductName?
			}
		}

		if( device ) RELEASE( device );

		return( DIENUM_CONTINUE );
	}


	//
	//	load and initialize DirectInput
	//
	bool InitDirectInput()
	{
		if( pDirectInput ) return true; // already initialized?

		// create DirectInput COM object
		HINSTANCE module = LoadLibrary( "DINPUT.DLL" );
		if( !module ) return false;
	
		typedef HRESULT (WINAPI *DIRECTINPUTCREATE)(HINSTANCE, DWORD, LPDIRECTINPUTA *, LPUNKNOWN);

		DIRECTINPUTCREATE create = (DIRECTINPUTCREATE) GetProcAddress( module, "DirectInputCreateA" );
		if( !create ) return false;

		HRESULT result = create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, &pDirectInput, NULL );
		if( result != DI_OK )
		{
			RELEASE( pDirectInput );
			pDirectInput = 0;
			return false;
		}

		// enumerate attached joysticks
		result = pDirectInput->EnumDevices( DIDEVTYPE_JOYSTICK, EnumJoystick, 0, DIEDFL_ATTACHEDONLY );

		// ok
		return true;
	}


	//
	//	build appropriate data format desc		
	//
	void BuildDataFormat( DIDATAFORMAT& df, int numAxes, int numButtons, int numPOVs )
	{
		df.dwSize		= sizeof( DIDATAFORMAT );
		df.dwObjSize	= sizeof( DIOBJECTDATAFORMAT );
		df.dwFlags		= DIDF_ABSAXIS;
		df.dwDataSize	= sizeof( DeviceData );
	
		int numObjs = numAxes + numButtons + numPOVs;
		DIOBJECTDATAFORMAT* pObjectData = new DIOBJECTDATAFORMAT[ numObjs ];

		int index = 0;
		for( int n=0; n < numAxes; ++n, ++index )
		{
			DIOBJECTDATAFORMAT& f = pObjectData[index];
			f.pguid		= NULL;
			f.dwOfs		= FIELD_OFFSET( DeviceData, axis ) + n*sizeof(LONG);
			f.dwType	= DIDFT_AXIS | DIDFT_ANYINSTANCE;
//			f.dwType	= DIDFT_AXIS | DIDFT_MAKEINSTANCE(n);	// TODO: or DIDFT_ABSAXIS
			f.dwFlags	= 0;
		}

		for( int n=0; n < numButtons; ++n, ++index )
		{
			DIOBJECTDATAFORMAT& f = pObjectData[index];
			f.pguid		= &GUID_Button;
			f.dwOfs		= FIELD_OFFSET( DeviceData, button ) + n;
			f.dwType	= DIDFT_BUTTON | DIDFT_ANYINSTANCE;
//			f.dwType	= DIDFT_BUTTON | DIDFT_MAKEINSTANCE(n);
			f.dwFlags	= 0;
		}

		for( int n=0; n < numPOVs; ++n, ++index )
		{
			DIOBJECTDATAFORMAT& f = pObjectData[index];
			f.pguid		= &GUID_POV;
			f.dwOfs		= FIELD_OFFSET( DeviceData, pov ) + n*sizeof(LONG);
			f.dwType	= DIDFT_POV | DIDFT_ANYINSTANCE;
//			f.dwType	= DIDFT_POV | DIDFT_MAKEINSTANCE(n);
			f.dwFlags	= 0;
		}

		df.dwNumObjs	= numObjs;
		df.rgodf		= pObjectData;
	}

	void ReleaseDataFormat( DIDATAFORMAT& df )
	{
		delete[] df.rgodf;
	}

} // namespace


// =================================================
// JoystickRep
// =================================================

class prcore::JoystickRep
{
public:
	LPDIRECTINPUTDEVICE2	mpDevice;
	DIDEVCAPS				mCaps;
	int						mNumAxes;
	int						mNumButtons;
	int						mNumPOVs;		// # of Point-of-View controllers on the device
	DeviceData				mData;

public:
	JoystickRep( HWND hwnd, LPDIRECTINPUTDEVICE2 pDevice );
	~JoystickRep();

	void Read();
};


JoystickRep::JoystickRep( HWND hwnd, LPDIRECTINPUTDEVICE2 pDevice )
{
	mpDevice = pDevice;

	// get joystick caps
	mCaps.dwSize = sizeof(DIDEVCAPS);
	mpDevice->GetCapabilities(&mCaps);

	mNumAxes	= mCaps.dwAxes;
	mNumButtons	= mCaps.dwButtons;
	mNumPOVs	= mCaps.dwPOVs;
	
	// set data format
	DIDATAFORMAT df;
	BuildDataFormat(df,mNumAxes,mNumButtons,mNumPOVs);

	if( mpDevice->SetDataFormat(&df) != DI_OK ) 
		PRCORE_EXCEPTION("Failed.");

	ReleaseDataFormat(df);

	// set non-exclusive cooperative mode (TODO: force-feedback requires exclusive)
	mpDevice->SetCooperativeLevel( hwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND );

	//
	//	set ranges
	//

	DIPROPRANGE diprg;
	diprg.diph.dwSize       = sizeof( diprg );
	diprg.diph.dwHeaderSize = sizeof( diprg.diph );
	diprg.diph.dwHow        = DIPH_BYOFFSET;
	diprg.lMin              = -int(JOY_RANGE);
	diprg.lMax              = +int(JOY_RANGE);

	diprg.diph.dwObj = DIJOFS_X;
	mpDevice->SetProperty( DIPROP_RANGE, &diprg.diph );
	
	diprg.diph.dwObj = DIJOFS_Y;
	mpDevice->SetProperty( DIPROP_RANGE, &diprg.diph );

	diprg.diph.dwObj = DIJOFS_Z;
	mpDevice->SetProperty( DIPROP_RANGE, &diprg.diph );

	diprg.diph.dwObj = DIJOFS_RX;
	mpDevice->SetProperty( DIPROP_RANGE, &diprg.diph );

	diprg.diph.dwObj = DIJOFS_RY;
	mpDevice->SetProperty( DIPROP_RANGE, &diprg.diph );

	diprg.diph.dwObj = DIJOFS_RZ;
	mpDevice->SetProperty( DIPROP_RANGE, &diprg.diph );

	// x/y/z axis dead zone 10%
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize       = sizeof( dipdw );
	dipdw.diph.dwHeaderSize = sizeof( dipdw.diph );
	dipdw.diph.dwObj        = DIJOFS_X;
	dipdw.diph.dwHow        = DIPH_BYOFFSET;
	dipdw.dwData            = 1000;
	mpDevice->SetProperty(DIPROP_DEADZONE,&dipdw.diph);

	dipdw.diph.dwObj = DIJOFS_Y;
	mpDevice->SetProperty(DIPROP_DEADZONE,&dipdw.diph);

	dipdw.diph.dwObj = DIJOFS_Z;
	mpDevice->SetProperty(DIPROP_DEADZONE,&dipdw.diph);

	dipdw.diph.dwObj = DIJOFS_RX;
	mpDevice->SetProperty(DIPROP_DEADZONE,&dipdw.diph);

	dipdw.diph.dwObj = DIJOFS_RY;
	mpDevice->SetProperty(DIPROP_DEADZONE,&dipdw.diph);
	
	dipdw.diph.dwObj = DIJOFS_RZ;
	mpDevice->SetProperty(DIPROP_DEADZONE,&dipdw.diph);


	//
	//	and now can acquire the device..
	//

	if( mpDevice->Acquire() != DI_OK ) 
		PRCORE_EXCEPTION("Can't acquire joystick.");
}


JoystickRep::~JoystickRep()
{
}


void JoystickRep::Read()
{
	mpDevice->Poll();

	if( mpDevice->GetDeviceState( sizeof(mData), &mData ) == (DIERR_INPUTLOST | DIERR_NOTACQUIRED) )
	{
		mpDevice->Acquire();
		mpDevice->GetDeviceState( sizeof(mData), &mData );
	}
}


// =================================================
// Joystick
// =================================================

Joystick::Joystick(WindowHandle handle, int index)
: mpRep(0)
{
	InitDirectInput();
	if ( index >= devices.GetSize() )
		PRCORE_EXCEPTION("Index out of range.");

	mpRep = new JoystickRep(handle,devices[index]);
}


Joystick::~Joystick()
{
	if ( mpRep )
	{
		delete mpRep;
		mpRep = NULL;
	}
}


int Joystick::GetAxisCount() const
{
	return mpRep->mNumAxes;
}


int Joystick::GetButtonCount() const
{
	return mpRep->mNumButtons;
}


int Joystick::GetPOVCount() const
{
	return mpRep->mNumPOVs;
}


float Joystick::GetAxis(int index)
{
	if( index < mpRep->mNumAxes )
	{
		mpRep->Read();
		return (mpRep->mData.axis[ index ] / JOY_RANGE);
	} else return 0.f;
}


int Joystick::GetButton(int index)
{
	if( index < mpRep->mNumButtons )
	{
		mpRep->Read();
		return (mpRep->mData.button[ index ]);
	} else return 0;
}


int Joystick::GetPOV(int index)
{
	// return values:
	// -1 = centered or not available
	// The position is indicated in hundredths of a degree clockwise from north (away from the user)
	if( index < mpRep->mNumPOVs )
	{
		mpRep->Read();
		LONG value = mpRep->mData.pov[ index ];
		if ( LOWORD(value) == 0xFFFF )	// centered
			return -1;
		else
			return mpRep->mData.pov[ index ];
	} else return -1;
}


int	Joystick::GetJoystickCount()
{
	InitDirectInput();
	return devices.GetSize();
}


const char* Joystick::GetJoystickName(int index)
{
	if( (index < 0) || (index >= deviceNames.GetSize()) )
		PRCORE_EXCEPTION("Index out of range.");

	return deviceNames[index];
}
