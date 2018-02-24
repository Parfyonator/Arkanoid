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
		FrameBuffer Windows implementation

	revision history:
		Jun/21/2001 - Jukka Liimatta - initial revision
		Apr/30/2002 - Jukka Liimatta - fixed memory leak on DX3 framebuffer close
		Oct/07/2002 - Jukka Liimatta - fixed surface restore on ::LockBuffer() method
		                               ALT/TAB should now work correctly when in fullscreen. ;-)
*/
#include <prcore/configure.hpp>
#include <prcore/array.hpp>
#include <prcore/exception.hpp>
#include <prcore/framebuffer.hpp>

#define DIRECTDRAW_VERSION 0x300
#include <ddraw.h>

using namespace prcore;


// =================================================
// DxError()
// =================================================

static void DxError(HRESULT result)
{
	if ( result == DD_OK )
		return;

	char* msg = NULL;
	switch( result )
	{
		case DDERR_ALREADYINITIALIZED:
			msg = "The object has already been initialized."; break;
		case DDERR_BLTFASTCANTCLIP:
			msg = "A DirectDrawClipper object is attached to a source surface that has passed into a call to the IDirectDrawSurface2::BltFast method."; break;
		case DDERR_CANNOTATTACHSURFACE:
			msg = "A surface cannot be attached to another requested surface."; break;
		case DDERR_CANNOTDETACHSURFACE:
			msg = "A surface cannot be detached from another requested surface."; break;
		case DDERR_CANTCREATEDC:
			msg = "Windows cannot create any more device contexts (DCs)."; break;
		case DDERR_CANTDUPLICATE:
			msg = "Primary and 3D surfaces, or surfaces that are implicitly created, cannot be duplicated."; break;
		case DDERR_CANTLOCKSURFACE:
			msg = "Access to this surface is refused because an attempt was made to lock the primary surface without DCI support."; break;
		case DDERR_CANTPAGELOCK:
			msg = "An attempt to page lock a surface failed. Page lock will not work on a display-memory surface or an emulated primary surface."; break;
		case DDERR_CANTPAGEUNLOCK:
			msg = "An attempt to page unlock a surface failed. Page unlock will not work on a display-memory surface or an emulated primary surface."; break;
		case DDERR_CLIPPERISUSINGHWND:
			msg = "An attempt was made to set a clip list for a DirectDrawClipper object that is already monitoring a window handle."; break;
		case DDERR_COLORKEYNOTSET:
			msg = "No source color key is specified for this operation."; break;
		case DDERR_CURRENTLYNOTAVAIL:
			msg = "No support is currently available."; break;
		case DDERR_DCALREADYCREATED:
			msg = "A device context (DC) has already been returned for this surface. Only one DC can be retrieved for each surface."; break;
		case DDERR_DIRECTDRAWALREADYCREATED:
			msg = "A DirectDraw object representing this driver has already been created for this process."; break;
		case DDERR_EXCEPTION:
			msg = "An exception was encountered while performing the requested operation."; break;
		case DDERR_EXCLUSIVEMODEALREADYSET:
			msg = "An attempt was made to set the cooperative level when it was already set to exclusive."; break;
		case DDERR_GENERIC:
			msg = "There is an undefined error condition."; break;
		case DDERR_HEIGHTALIGN:
			msg = "The height of the provided rectangle is not a multiple of the required alignment."; break;
		case DDERR_HWNDALREADYSET:
			msg = "The DirectDraw cooperative level window handle has already been set. It cannot be reset while the process has surfaces or palettes created."; break;
		case DDERR_HWNDSUBCLASSED:
			msg = "DirectDraw is prevented from restoring state because the DirectDraw cooperative level window handle has been subclassed."; break;
		case DDERR_IMPLICITLYCREATED:
			msg = "The surface cannot be restored because it is an implicitly created surface."; break;
		case DDERR_INCOMPATIBLEPRIMARY:
			msg = "The primary surface creation request does not match with the existing primary surface."; break;
		case DDERR_INVALIDCAPS:
			msg = "One or more of the capability bits passed to the callback function are incorrect."; break;
		case DDERR_INVALIDCLIPLIST:
			msg = "DirectDraw does not support the provided clip list."; break;
		case DDERR_INVALIDDIRECTDRAWGUID:
			msg = "The globally unique identifier (GUID) passed to the DirectDrawCreate function is not a valid DirectDraw driver identifier."; break;
		case DDERR_INVALIDMODE:
			msg = "DirectDraw does not support the requested mode."; break;
		case DDERR_INVALIDOBJECT:
			msg = "DirectDraw received a pointer that was an invalid DirectDraw object."; break;
		case DDERR_INVALIDPARAMS:
			msg = "One or more of the parameters passed to the method are incorrect."; break;
		case DDERR_INVALIDPIXELFORMAT:
			msg = "The pixel format was invalid as specified."; break;
		case DDERR_INVALIDPOSITION:
			msg = "The position of the overlay on the destination is no longer legal."; break;
		case DDERR_INVALIDRECT:
			msg = "The provided rectangle was invalid."; break;
		case DDERR_INVALIDSURFACETYPE:
			msg = "The requested operation could not be performed because the surface was of the wrong type."; break;
		case DDERR_LOCKEDSURFACES:
			msg = "One or more surfaces are locked, causing the failure of the requested operation."; break;
		case DDERR_NO3D:
			msg = "No 3D hardware or emulation is present."; break;
		case DDERR_NOALPHAHW:
			msg = "No alpha acceleration hardware is present or available, causing the failure of the requested operation."; break;
		case DDERR_NOBLTHW:
			msg = "No blitter hardware is present."; break;
		case DDERR_NOCLIPLIST:
			msg = "No clip list is available."; break;
		case DDERR_NOCLIPPERATTACHED:
			msg = "No DirectDrawClipper object is attached to the surface object."; break;
		case DDERR_NOCOLORCONVHW:
			msg = "The operation cannot be carried out because no color-conversion hardware is present or available."; break;
		case DDERR_NOCOLORKEY:
			msg = "The surface does not currently have a color key."; break;
		case DDERR_NOCOLORKEYHW:
			msg = "The operation cannot be carried out because there is no hardware support for the destination color key."; break;
		case DDERR_NOCOOPERATIVELEVELSET:
			msg = "A create function is called without the IDirectDraw2::SetCooperativeLevel method being called."; break;
		case DDERR_NODC:
			msg = "No DC has ever been created for this surface."; break;
		case DDERR_NODDROPSHW:
			msg = "No DirectDraw raster operation (ROP) hardware is available."; break;
		case DDERR_NODIRECTDRAWHW:
			msg = "Hardware-only DirectDraw object creation is not possible; the driver does not support any hardware."; break;
		case DDERR_NODIRECTDRAWSUPPORT:
			msg = "DirectDraw support is not possible with the current display driver."; break;
		case DDERR_NOEMULATION:
			msg = "Software emulation is not available."; break;
		case DDERR_NOEXCLUSIVEMODE:
			msg = "The operation requires the application to have exclusive mode, but the application does not have exclusive mode."; break;
		case DDERR_NOFLIPHW:
			msg = "Flipping visible surfaces is not supported."; break;
		case DDERR_NOGDI:
			msg = "No GDI is present."; break;
		case DDERR_NOHWND:
			msg = "Clipper notification requires a window handle, or no window handle has been previously set as the cooperative level window handle."; break;
		case DDERR_NOMIPMAPHW:
			msg = "The operation cannot be carried out because no mipmap texture mapping hardware is present or available."; break;
		case DDERR_NOMIRRORHW:
			msg = "The operation cannot be carried out because no mirroring hardware is present or available."; break;
		case DDERR_NOOVERLAYDEST:
			msg = "The IDirectDrawSurface2::GetOverlayPosition method is called on an overlay that the IDirectDrawSurface2::UpdateOverlay method has not been called on to establish a destination."; break;
		case DDERR_NOOVERLAYHW:
			msg = "The operation cannot be carried out because no overlay hardware is present or available."; break;
		case DDERR_NOPALETTEATTACHED:
			msg = "No palette object is attached to this surface."; break;
		case DDERR_NOPALETTEHW:
			msg = "There is no hardware support for 16- or 256-color palettes."; break;
		case DDERR_NORASTEROPHW:
			msg = "The operation cannot be carried out because no appropriate raster operation hardware is present or available."; break;
		case DDERR_NOROTATIONHW:
			msg = "The operation cannot be carried out because no rotation hardware is present or available."; break;
		case DDERR_NOSTRETCHHW:
			msg = "The operation cannot be carried out because there is no hardware support for stretching."; break;
		case DDERR_NOT4BITCOLOR:
			msg = "The DirectDrawSurface object is not using a 4-bit color palette and the requested operation requires a 4-bit color palette."; break;
		case DDERR_NOT4BITCOLORINDEX:
			msg = "The DirectDrawSurface object is not using a 4-bit color index palette and the requested operation requires a 4-bit color index palette."; break;
		case DDERR_NOT8BITCOLOR:
			msg = "The DirectDrawSurface object is not using an 8-bit color palette and the requested operation requires an 8-bit color palette."; break;
		case DDERR_NOTAOVERLAYSURFACE:
			msg = "An overlay component is called for a non-overlay surface."; break;
		case DDERR_NOTEXTUREHW:
			msg = "The operation cannot be carried out because no texture-mapping hardware is present or available."; break;
		case DDERR_NOTFLIPPABLE:
			msg = "An attempt has been made to flip a surface that cannot be flipped."; break;
		case DDERR_NOTFOUND:
			msg = "The requested item was not found."; break;
		case DDERR_NOTLOCKED:
			msg = "An attempt is made to unlock a surface that was not locked."; break;
		case DDERR_NOTPAGELOCKED:
			msg = "An attempt is made to page unlock a surface with no outstanding page locks."; break;
		case DDERR_NOTPALETTIZED:
			msg = "The surface being used is not a palette-based surface."; break;
		case DDERR_NOVSYNCHW:
			msg = "The operation cannot be carried out because there is no hardware support for vertical blank synchronized operations."; break;
		case DDERR_NOZBUFFERHW:
			msg = "The operation to create a z-buffer in display memory or to perform a blit using a z-buffer cannot be carried out because there is no hardware support for z-buffers."; break;
		case DDERR_NOZOVERLAYHW:
			msg = "The overlay surfaces cannot be z-layered based on the z-order because the hardware does not support z-ordering of overlays."; break;
		case DDERR_OUTOFCAPS:
			msg = "The hardware needed for the requested operation has already been allocated."; break;
		case DDERR_OUTOFMEMORY:
			msg = "DirectDraw does not have enough memory to perform the operation."; break;
		case DDERR_OUTOFVIDEOMEMORY:
			msg = "DirectDraw does not have enough display memory to perform the operation."; break;
		case DDERR_OVERLAYCANTCLIP:
			msg = "The hardware does not support clipped overlays."; break;
		case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:
			msg = "An attempt was made to have more than one color key active on an overlay."; break;
		case DDERR_OVERLAYNOTVISIBLE:
			msg = "The IDirectDrawSurface2::GetOverlayPosition method is called on a hidden overlay."; break;
		case DDERR_PALETTEBUSY:
			msg = "Access to this palette is refused because the palette is locked by another thread."; break;
		case DDERR_PRIMARYSURFACEALREADYEXISTS:
			msg = "This process has already created a primary surface."; break;
		case DDERR_REGIONTOOSMALL:
			msg = "The region passed to the IDirectDrawClipper::GetClipList method is too small."; break;
		case DDERR_SURFACEALREADYATTACHED:
			msg = "An attempt was made to attach a surface to another surface to which it is already attached."; break;
		case DDERR_SURFACEALREADYDEPENDENT:
			msg = "An attempt was made to make a surface a dependency of another surface to which it is already dependent."; break;
		case DDERR_SURFACEBUSY:
			msg = "Access to the surface is refused because the surface is locked by another thread."; break;
		case DDERR_SURFACEISOBSCURED:
			msg = "Access to the surface is refused because the surface is obscured."; break;
		case DDERR_SURFACELOST:
			msg = "Access to the surface is refused because the surface memory is gone. The DirectDrawSurface object representing this surface should have the IDirectDrawSurface2::Restore method called on it."; break;
		case DDERR_SURFACENOTATTACHED:
			msg = "The requested surface is not attached."; break;
		case DDERR_TOOBIGHEIGHT:
			msg = "The height requested by DirectDraw is too large."; break;
		case DDERR_TOOBIGSIZE:
			msg = "The size requested by DirectDraw is too large. However, the individual height and width are OK."; break;
		case DDERR_TOOBIGWIDTH:
			msg = "The width requested by DirectDraw is too large."; break;
		case DDERR_UNSUPPORTED:
			msg = "The operation is not supported."; break;
		case DDERR_UNSUPPORTEDFORMAT:
			msg = "The FourCC format requested is not supported by DirectDraw."; break;
		case DDERR_UNSUPPORTEDMASK:
			msg = "The bitmask in the pixel format requested is not supported by DirectDraw."; break;
		case DDERR_UNSUPPORTEDMODE:
			msg = "The display is currently in an unsupported mode."; break;
		case DDERR_VERTICALBLANKINPROGRESS:
			msg = "A vertical blank is in progress."; break;
		case DDERR_WASSTILLDRAWING:
			msg = "The previous blit operation that is transferring information to or from this surface is incomplete."; break;
		case DDERR_WRONGMODE:
			msg = "This surface cannot be restored because it was created in a different mode."; break;
		case DDERR_XALIGN:
			msg = "The provided rectangle was not horizontally aligned on a required boundary."; break;
		default:
			char buffer[256];
			wsprintf(buffer,"Unknown error: %d ($%x)",result,result);
			msg = buffer;
			break;
	}
	
	PRCORE_EXCEPTION(msg);
};


// =================================================
// mics. functions
// =================================================

static bool FindModeDX3(const Array<VideoMode>& modelist, const VideoMode& mode)
{
	int count = modelist.GetSize();
	for ( int i=0; i<count; ++i )
	{
		VideoMode cmode = modelist[i];

		if ( mode.width==cmode.width && mode.height==cmode.height && mode.bits==cmode.bits )
			return true;
	}
	return false;
}


static HRESULT WINAPI ModeEnumCB(LPDDSURFACEDESC srdesc, LPVOID user)
{
	// videomode
	VideoMode mode;
	mode.width  = srdesc->dwWidth;
	mode.height = srdesc->dwHeight;
	mode.bits   = srdesc->ddpfPixelFormat.dwRGBBitCount;

	// modelist
	Array<VideoMode>& modelist = *(Array<VideoMode>*)user;

	// append mode
	if ( !FindModeDX3(modelist,mode) )
		modelist.PushBack(mode);

	return DDENUMRET_OK;
}


static PixelFormat GetPixelFormatDX3(LPDIRECTDRAWSURFACE surface)
{
	DDPIXELFORMAT pxf;
	pxf.dwSize = sizeof(pxf);
	surface->GetPixelFormat(&pxf);

	// indexed
	if ( pxf.dwRGBBitCount == 8 )
		return PIXELFORMAT_PALETTE8(NULL);

	// mask
	int32 mask = ~0;
	if ( pxf.dwRGBBitCount != 32 ) mask = (1 << pxf.dwRGBBitCount) - 1;
	uint32 AlphaMask = (~(pxf.dwRBitMask | pxf.dwGBitMask | pxf.dwBBitMask)) & mask;

	// create pixelformat
	return PixelFormat(pxf.dwRGBBitCount,pxf.dwRBitMask,pxf.dwGBitMask,pxf.dwBBitMask,AlphaMask);
}


// =================================================
// DirectX3
// =================================================

struct DirectX3
{
	Array<VideoMode>	modelist;
	LPDIRECTDRAW		lpDD;
	LPDIRECTDRAWSURFACE	lpDDSfront;
	LPDIRECTDRAWSURFACE	lpDDSback;
	LPDIRECTDRAWCLIPPER	lpDDclipper;
	LPDIRECTDRAWPALETTE	lpPalette;
	PALETTEENTRY		paldata[256];
	HWND				hwnd;
	Surface				surface;

	DirectX3()
	{
		lpDD        = NULL;
		lpDDSfront  = NULL;
		lpDDSback   = NULL;
		lpDDclipper = NULL;
		lpPalette   = NULL;
		hwnd        = NULL;

		char* error = NULL;

		// create directdraw com object
		DxError( DirectDrawCreate(NULL,&lpDD,NULL) );

		// enumerate supported resolutions
		if ( FAILED(lpDD->EnumDisplayModes(0,NULL,&modelist,&ModeEnumCB)) )
		{
			// release
			lpDD->Release();
			lpDD = NULL;
			PRCORE_EXCEPTION("Cannot enumerate display modes.");
		}
	}

	~DirectX3()
	{
		// release
		if ( lpDD )
		{
			lpDD->Release();
			lpDD = NULL;
		}
	}

	bool OpenBuffer(WindowHandle handle, int width, int height, int bits)
	{
		lpPalette   = NULL;
		lpDDclipper = NULL;
		hwnd = handle.hwnd;

		// set cooperative level
		DxError( lpDD->SetCooperativeLevel(handle.hwnd,DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN) );

		// enter display mode
		DxError( lpDD->SetDisplayMode(width,height,bits) );

		// primary with two back buffers
		DDSURFACEDESC desc;
		desc.dwSize  = sizeof(desc);
		desc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		desc.dwBackBufferCount = 2;
		desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY | DDSCAPS_COMPLEX | DDSCAPS_FLIP;

		if ( FAILED( lpDD->CreateSurface(&desc,&lpDDSfront,0) ) )
		{
			// primary with one back buffer
			desc.dwBackBufferCount = 1;
			if ( FAILED( lpDD->CreateSurface(&desc,&lpDDSfront,0) ) )
			{
				// primary with no back buffers
				desc.dwFlags = DDSD_CAPS;
				desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;
				DxError( lpDD->CreateSurface(&desc,&lpDDSfront,0) );
			}
		}

		// get back surface
		DDSCAPS caps;
		caps.dwCaps = DDSCAPS_BACKBUFFER;
		DxError( lpDDSfront->GetAttachedSurface(&caps,&lpDDSback) );

		// palette
		if ( bits == 8 )
		{
			// setup palette
			for ( int i=0; i<256; ++i )
			{
				paldata[i].peRed = 0;
				paldata[i].peGreen = 0;
				paldata[i].peBlue = 0;
				paldata[i].peFlags = 0;
			}

			// create palette
			DxError( lpDD->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256,paldata,&lpPalette,NULL) );

			// set palette
			lpDDSfront->SetPalette(lpPalette);
			lpDDSback->SetPalette(lpPalette);
		}

		// disable screensaver
		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE,0,0,0);

		return true;
	}


	bool OpenBuffer(WindowHandle handle)
	{
		char* error = NULL;

		// set cooperative level
		DxError( lpDD->SetCooperativeLevel(handle.hwnd,DDSCL_NORMAL) );

		// primary with two back buffers
		DDSURFACEDESC desc;
		desc.dwSize            = sizeof(desc);
		desc.dwFlags           = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		desc.dwBackBufferCount = 2;
		desc.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY | DDSCAPS_COMPLEX | DDSCAPS_FLIP;

		if ( FAILED(lpDD->CreateSurface(&desc,&lpDDSfront,0)) )
		{
			// primary with one back buffer
			desc.dwBackBufferCount = 1;
			if ( FAILED( lpDD->CreateSurface(&desc,&lpDDSfront,0) ) )
			{
				// primary with no back buffers
				desc.dwFlags = DDSD_CAPS;
				desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;
				DxError( lpDD->CreateSurface(&desc,&lpDDSfront,0) );
			}
		}

		// get window size
		RECT rect;
		GetClientRect(handle.hwnd,&rect);
		int width  = rect.right - rect.left + 0;
		int height = rect.bottom - rect.top + 0;
		
		// create an offscreen surface
		desc.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		desc.dwWidth        = width;
		desc.dwHeight       = height;
		desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		
		DxError( lpDD->CreateSurface(&desc,&lpDDSback,NULL) );
		DxError( lpDD->CreateClipper(0,&lpDDclipper,NULL) );
		DxError( lpDDclipper->SetHWnd(0,handle.hwnd) );
		DxError( lpDDSfront->SetClipper(lpDDclipper) );

		hwnd = handle.hwnd;

		// disable screensaver
		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE,0,0,0);
		
		return true;
	}

	bool CloseBuffer()
	{
		if ( lpPalette )
		{
			lpPalette->Release();
			lpPalette = NULL;
		}

		if ( lpDDclipper )
		{
			lpDDclipper->Release();
			lpDDclipper = NULL;
		}

		if ( lpDDSback )
		{
			// release offscreen surface
			lpDDSback->Release();
			lpDDSback = NULL;
		}

		if ( lpDDSfront )
		{
			// release primary
			lpDDSfront->Release();
			lpDDSfront = NULL;
		}

		if ( lpDD )
		{
			// leave display mode
			lpDD->RestoreDisplayMode();

			// leave exclusive mode
			lpDD->SetCooperativeLevel(hwnd,DDSCL_NORMAL);
		}

		// enable screensaver
		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE,1,0,0);

		return true;
	}

	void ClearBuffer(uint32 color)
	{
		DDBLTFX blt;
		blt.dwSize = sizeof(blt);
		blt.dwFillColor = color;

		lpDDSback->Blt(NULL,NULL,NULL,DDBLT_COLORFILL,&blt);
	}

	Surface* LockBuffer()
	{
		DDSURFACEDESC desc;
		desc.dwSize = sizeof(desc);

		for ( ;; )
		{
			// lock
			HRESULT r = lpDDSback->Lock(NULL,&desc,NULL,NULL);
			if ( r == DD_OK )
				break;

			if ( r == DDERR_SURFACELOST )
			{
				if ( lpDDSback->Restore() != DD_OK )
					return NULL;
			}
			else if ( r != DDERR_WASSTILLDRAWING )
				return NULL;
		}

		PixelFormat pxf = GetPixelFormatDX3(lpDDSback);
		int pitch = (desc.dwFlags & DDSD_PITCH) ? desc.lPitch : desc.dwWidth*pxf.GetBytes();
		surface = Surface(desc.dwWidth,desc.dwHeight,pitch,pxf,desc.lpSurface);

		return &surface;
	}

	void UnlockBuffer()
	{
		lpDDSback->Unlock(NULL);
	}

	void SetPalette(const color32 palette[])
	{
		if ( lpPalette == NULL )
			return;

		// setup palette
		for ( int i=0; i<256; ++i )
		{
			paldata[i].peRed   = palette[i].r;
			paldata[i].peGreen = palette[i].g;
			paldata[i].peBlue  = palette[i].b;
			paldata[i].peFlags = 0;
		}

		// set palette
		lpPalette->SetEntries(0,0,256,paldata);
	}

	void PageFlip(bool retrace)
	{
		if ( lpDDclipper )
		{
			RECT rect;
			POINT p;
			p.x = 0;
			p.y = 0;

			// find out where on the primary surface our window lives
			ClientToScreen(hwnd,&p);
			GetClientRect(hwnd,&rect);
			OffsetRect(&rect,p.x,p.y);

			// blit
			lpDDSfront->Blt(&rect,lpDDSback,NULL,DDBLT_WAIT,NULL);
		}
		else
		{
			HRESULT r = retrace ?
				lpDDSfront->Flip(NULL,DDFLIP_WAIT) :
				lpDDSfront->Flip(NULL,0);

			if ( r == DDERR_SURFACELOST )
				lpDDSfront->Restore();
		}
	}

}; // struct DirectX3


// =================================================
// FrameBuffer
// =================================================

FrameBuffer::FrameBuffer()
{
	// create context
	DirectX3* context = new DirectX3();
	mpContext = context;
}


FrameBuffer::~FrameBuffer()
{
	CloseBuffer();

	// release context
	DirectX3* context = reinterpret_cast<DirectX3*>(mpContext);
	delete context;
}


int FrameBuffer::GetModeCount() const
{
	DirectX3* context = reinterpret_cast<DirectX3*>(mpContext);
	return context->modelist.GetSize();
}


VideoMode FrameBuffer::GetMode(int index) const
{
	DirectX3* context = reinterpret_cast<DirectX3*>(mpContext);
	return context->modelist[index];
}


bool FrameBuffer::OpenBuffer(WindowHandle handle)
{
	DirectX3* context = reinterpret_cast<DirectX3*>(mpContext);

	BindWindow(handle);
	return context->OpenBuffer(mWindowHandle);
}

bool FrameBuffer::OpenBuffer(WindowHandle xparent, int width, int height)
{
	DirectX3* context = reinterpret_cast<DirectX3*>(mpContext);

	OpenWindow(xparent, width, height);
	return context->OpenBuffer(mWindowHandle);
}


bool FrameBuffer::OpenBuffer(int width, int height, const String& name, uint32 windowstyle)
{
	DirectX3* context = reinterpret_cast<DirectX3*>(mpContext);

	OpenWindow(width,height,name,windowstyle);
	return context->OpenBuffer(mWindowHandle);
}


bool FrameBuffer::OpenBuffer(int width, int height, int bits, const String& name, uint32 windowstyle)
{
	DirectX3* context = reinterpret_cast<DirectX3*>(mpContext);

	OpenWindow(width,height,name,windowstyle);
	return context->OpenBuffer(mWindowHandle,width,height,bits);
}


bool FrameBuffer::CloseBuffer(bool mainbreak)
{
	DirectX3* context = reinterpret_cast<DirectX3*>(mpContext);

	context->CloseBuffer();
	CloseWindow(mainbreak);
	UnbindWindow();

	return true;
}


void FrameBuffer::SetPalette(const color32 palette[])
{
	DirectX3* context = reinterpret_cast<DirectX3*>(mpContext);
	context->SetPalette(palette);
}


void FrameBuffer::BlitWrite(int x, int y, const Surface& source)
{
	Surface* surface = LockBuffer();
	if ( surface )
	{
		surface->BlitImage(x,y,source);
		UnlockBuffer();
	}
}


bool FrameBuffer::ResizeBuffer(int width, int height)
{
	ResizeWindow(width,height);

	DirectX3* context = reinterpret_cast<DirectX3*>(mpContext);
	if ( !context->lpDDclipper )
		return false;

	context->CloseBuffer();
	context->OpenBuffer(mWindowHandle);

	return true;	
}


void FrameBuffer::ClearBuffer(color32 color)
{
	DirectX3* context = reinterpret_cast<DirectX3*>(mpContext);
	context->ClearBuffer(color);
}


Surface* FrameBuffer::LockBuffer()
{
	DirectX3* context = reinterpret_cast<DirectX3*>(mpContext);
	return context->LockBuffer();
}


void FrameBuffer::UnlockBuffer()
{
	DirectX3* context = reinterpret_cast<DirectX3*>(mpContext);
	context->UnlockBuffer();
}


void FrameBuffer::PageFlip(bool retrace)
{
	DirectX3* context = reinterpret_cast<DirectX3*>(mpContext);
	context->PageFlip(retrace);
}
