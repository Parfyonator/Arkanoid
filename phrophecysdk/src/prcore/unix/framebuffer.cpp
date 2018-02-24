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
  FrameBuffer UNIX implementation

  revision history:
  Apr/20/2003 - Jukka Liimatta - initial revision
  Jan/10/2004 - Edmond Cote - inital unix revision (XFree86/XSHM)
*/

#include <iostream>
using namespace std;

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <prcore/framebuffer.hpp>
#include <prcore/exception.hpp>
#include <prcore/surface.hpp>

using namespace prcore;

Surface *mSurface;

FrameBuffer::FrameBuffer()
{
 
}


FrameBuffer::~FrameBuffer()
{
 
}

int FrameBuffer::GetModeCount() const
{
  // TODO
  return 0;
}


VideoMode FrameBuffer::GetMode(int index) const
{
  // TODO
  VideoMode mode;
  return mode;
}


bool FrameBuffer::OpenBuffer(WindowHandle handle)
{
  // TODO
  PRCORE_EXCEPTION("Please implement me!");
  return false;
}


bool FrameBuffer::OpenBuffer(WindowHandle xparent, int width, int height)
{
  // TODO
  PRCORE_EXCEPTION("Please implement me!");
  return false;
}


bool FrameBuffer::OpenBuffer(int width, int height, const String& name, uint32 windowstyle)
{

  // all the work will be done here

  if(!IsWindowOpen())
    OpenWindow(width,height,name,windowstyle & !WINDOW_USES3D);

  if (!XShmQueryExtension(mWindowHandle.display))
    PRCORE_EXCEPTION("MIT-SHM extension NOT supported");

  // create shared memory XImage structure

  XShmSegmentInfo *shmSegmentInfo = new XShmSegmentInfo;

  XImage *xImage = XShmCreateImage(mWindowHandle.display, 
				   mWindowHandle.visual.visual,
				   mWindowHandle.visual.depth, ZPixmap, 0,
				   shmSegmentInfo, width, height);

  if(!xImage)
    PRCORE_EXCEPTION("Unable to create shared memory XImage structure");

  // create the shared memory segment

  shmSegmentInfo->shmid = shmget(IPC_PRIVATE, 
				xImage->bytes_per_line * xImage->height, 
				IPC_CREAT | 0777);

  if(shmSegmentInfo->shmid < 0)
    PRCORE_EXCEPTION("Unable to create shared memory segment");
  
  // attach shared memory segment to process
  
  shmSegmentInfo->shmaddr = xImage->data =
    (char *)shmat(shmSegmentInfo->shmid, 0, 0);

  if(!shmSegmentInfo->shmaddr)
    PRCORE_EXCEPTION("Unable to attach shared memory segment to process");

  // set segment ot read/write

  shmSegmentInfo->readOnly = False;
  
  // attach the shared memory segment to the display
  
  if(!XShmAttach(mWindowHandle.display, shmSegmentInfo))
    PRCORE_EXCEPTION("Unable to attach shared memory segment to display");

  // describe surface
  
  PixelFormat pxf;

  int size = xImage->bytes_per_line / xImage->width;
  
  switch(size)
    {
      
      //case 2:
      //break;
    case 3:
      pxf = PIXELFORMAT_RGB888;
      break;
    case 4:
      pxf = PIXELFORMAT_ARGB8888;
      break;
    default:
      PRCORE_EXCEPTION("fixme!: only 24 and 32 bpp displays supported");
      break;
    }

  mSurface = new Surface(xImage->width, xImage->height,
			 xImage->bytes_per_line, pxf,xImage->data);
  
  // set global variables

  mWindowHandle.xImage = xImage;
  mWindowHandle.shmSegmentInfo = shmSegmentInfo;
  
  return true;
}

bool FrameBuffer::OpenBuffer(int width, int height, int bits, 
			     const String& name, uint32 windowstyle)
{
			    
  if(bits != mWindowHandle.visual.depth)
    PRCORE_EXCEPTION("Requested bit depth does not match X11 bit depth");
  
  return OpenBuffer(width,height,bits,name,windowstyle);

}




bool FrameBuffer::CloseBuffer(bool mainbreak)
{
  
  // detach the segment from the display

  XShmDetach(mWindowHandle.display, mWindowHandle.shmSegmentInfo);

  // destroy the XShmImage

  XDestroyImage(mWindowHandle.xImage);

  // detach the buffer from the segment

  shmdt(mWindowHandle.shmSegmentInfo->shmaddr);

  // remove the segment

  shmctl(mWindowHandle.shmSegmentInfo->shmid, IPC_RMID, 0);

  if(mWindowHandle.shmSegmentInfo)
    delete mWindowHandle.shmSegmentInfo;

  return true;
}


void FrameBuffer::SetPalette(const color32 palette[])
{
  // TODO
  PRCORE_EXCEPTION("Function not supported");
}


void FrameBuffer::BlitWrite(int x, int y, const Surface& source)
{

  // Automatic format conversion

  Surface *surface = mSurface;

  surface->BlitImage(x,y,source);

}

bool FrameBuffer::ResizeBuffer(int width, int height)
{
  // TODO
  return false;
}


void FrameBuffer::ClearBuffer(color32 color)
{
  // TODO
}


Surface* FrameBuffer::LockBuffer()
{
  // TODO

  return NULL;
}


void FrameBuffer::UnlockBuffer()
{
}

void FrameBuffer::PageFlip(bool retrace)
{
  Surface *surface = mSurface;

  // TODO
  XSync (mWindowHandle.display, 0);
  
  // Blit buffer onto the window
  
  XShmPutImage(mWindowHandle.display, mWindowHandle.window,
	       mWindowHandle.gc, mWindowHandle.xImage, 
	       0, 0, 0, 0, // src_x, src_y, dest_x, dest_y
	       surface->GetWidth(), surface->GetHeight(), 
	       False);
    
}

