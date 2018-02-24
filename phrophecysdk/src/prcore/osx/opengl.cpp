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
	WindowGL / OpenGL OSX implementation
		
	revision history:
	December/25/2003 - Stefan Karlsson
*/

#include <prcore/opengl.hpp>
#include <prcore/exception.hpp>

#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <AGL/agl.h>

#include <iostream>

namespace prcore
{
    
    struct ContextGLX
    {
        AGLPixelFormat  aglPixFmt;
        AGLContext      aglContext;
        GLint           attributes[32];

        int             nrAttributes;
        uint32          color;
        uint32          depth; 
        uint32          stencil;
        uint32          accum;
        
        GDHandle        device;
    };


    static Array<ModeGL>			ModeList;
    static bool						isFullscreen;

    static void EnumModeList()
    {
        CFArrayRef modes = CGDisplayAvailableModes(kCGDirectMainDisplay);
        if (!modes) return;
        
        CFIndex modeCount = CFArrayGetCount(modes);
        
        for (uint32 i = 0;  i < (uint32) modeCount;  i++)
        {
            CFDictionaryRef refmode = (CFDictionaryRef) CFArrayGetValueAtIndex(modes, i);
            
			ModeGL mode;            
            CFNumberGetValue( (CFNumberRef) CFDictionaryGetValue(refmode, kCGDisplayWidth), 
                              kCFNumberIntType, &mode.width);
            CFNumberGetValue( (CFNumberRef) CFDictionaryGetValue(refmode, kCGDisplayHeight), 
                              kCFNumberIntType, &mode.height);
            CFNumberGetValue( (CFNumberRef) CFDictionaryGetValue(refmode, kCGDisplayBitsPerPixel), 
                              kCFNumberIntType, &mode.bits);
            CFNumberGetValue( (CFNumberRef) CFDictionaryGetValue(refmode, kCGDisplayRefreshRate), 
                              kCFNumberIntType, &mode.frequency);

			ModeList.PushBack(mode);
            
        }
    }

    ContextGL* CreateContextWorkAround(WindowHandle hwnd,uint32 color, uint32 depth, 
                                       uint32 stencil, uint32 accum, 
                                       bool fullscreen)
    {
        
        ContextGLX *rVal = new ContextGLX();
        memset(rVal,0,sizeof(ContextGLX));
        
        if(fullscreen)
            rVal->device = DMGetFirstScreenDevice(true);
        
        rVal->color = color;
        rVal->depth = depth;
        rVal->stencil = stencil;
        rVal->accum = accum;
        
        Rect rectPort;
        
#define PRCORE_ADD_ATTRIBUTE_WINDOWGL(x) \
        do{ \
            rVal->attributes[rVal->nrAttributes++] = x; \
        }while(0)
        
        PRCORE_ADD_ATTRIBUTE_WINDOWGL(AGL_RGBA);
        PRCORE_ADD_ATTRIBUTE_WINDOWGL(AGL_DOUBLEBUFFER);
        PRCORE_ADD_ATTRIBUTE_WINDOWGL(AGL_ACCELERATED);
        
        if(fullscreen)
            PRCORE_ADD_ATTRIBUTE_WINDOWGL(AGL_FULLSCREEN);
        
        PRCORE_ADD_ATTRIBUTE_WINDOWGL(AGL_PIXEL_SIZE);
        PRCORE_ADD_ATTRIBUTE_WINDOWGL(color);
        
        PRCORE_ADD_ATTRIBUTE_WINDOWGL(AGL_STENCIL_SIZE);
        PRCORE_ADD_ATTRIBUTE_WINDOWGL(stencil);
        
        PRCORE_ADD_ATTRIBUTE_WINDOWGL(AGL_DEPTH_SIZE);
        PRCORE_ADD_ATTRIBUTE_WINDOWGL(depth);
        
        // This is stuff that somehow should be added to WindowGL IMO, but somehow
        // via parameters or so i guess..
        //PRCORE_ADD_ATTRIBUTE_WINDOWGL(AGL_SAMPLE_BUFFERS_ARB);
        //PRCORE_ADD_ATTRIBUTE_WINDOWGL(1);
        //PRCORE_ADD_ATTRIBUTE_WINDOWGL(AGL_SAMPLES_ARB);
        //PRCORE_ADD_ATTRIBUTE_WINDOWGL(4);
        
        PRCORE_ADD_ATTRIBUTE_WINDOWGL(AGL_NO_RECOVERY);
        PRCORE_ADD_ATTRIBUTE_WINDOWGL(AGL_NONE);
        
#undef PRCORE_ADD_ATTRIBUTE_WINDOWGL
		
        // build context
        rVal->aglContext = NULL;
        rVal->aglPixFmt = aglChoosePixelFormat(&rVal->device, rVal->device?1:0, rVal->attributes);
        if (!rVal->aglPixFmt) {
            PRCORE_EXCEPTION( String((const char*)aglErrorString(aglGetError()) ) );
        }
        rVal->aglContext = aglCreateContext(rVal->aglPixFmt, NULL);
        if (rVal->aglContext) {
            SetPort ((GrafPtr) GetWindowPort (hwnd));
            
            PRCORE_ASSERT_EXCEPTION( !aglSetDrawable(rVal->aglContext, GetWindowPort (hwnd)) );
            PRCORE_ASSERT_EXCEPTION( !aglSetCurrentContext(rVal->aglContext) );
            
            // VBL SYNC
            GLint swap = 1;
            PRCORE_ASSERT_EXCEPTION( !aglSetInteger (rVal->aglContext, AGL_SWAP_INTERVAL, &swap) );            
            glEnable (GL_MULTISAMPLE_ARB);
            glHint (GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
            
            return rVal;
        }
        delete rVal;
        return NULL;        
    }
    
    
    WindowGL::WindowGL()
    {
        if ( !ModeList.GetSize() )
        {
            EnumModeList();
        }
        
        isFullscreen = false;
    }


    WindowGL::~WindowGL()
    {
        RestoreMode();
    }


    int WindowGL::GetModeCount() const
    {
        return ModeList.GetSize();
    }

    const ModeGL& WindowGL::GetMode(int index) const
    {
        return ModeList[index];
    }

        
    bool WindowGL::SetMode(const ModeGL& mode)
    {
        isFullscreen = true;
        ContextGLX *glXC = (ContextGLX *)mCurrentContext;
        ContextGLX *glXCt = (ContextGLX*)CreateContextWorkAround(mWindowHandle,
                                                    glXC->color,
                                                    glXC->depth,
                                                    glXC->stencil,
                                                    glXC->accum,
                                                    true);
        glXC->aglContext = glXCt->aglContext;
        glXC->device = glXCt->device;

        delete glXCt;
        
        aglSetFullScreen (glXC->aglContext, mode.width, mode.height, mode.frequency, 0);

        SetContext(glXC);
        return true;
    }


    bool WindowGL::RestoreMode()
    {
        ContextGLX *glXC = (ContextGLX *)mCurrentContext;
		if(!glXC) return false;
        
        aglSetCurrentContext (NULL);
		aglSetDrawable (glXC->aglContext, NULL);
		aglDestroyContext (glXC->aglContext);
		glXC->aglContext = NULL;
        
        // create us a new , NONFULLSCREEN context
        ContextGLX *glXCt = (ContextGLX*)
                                CreateContextWorkAround(mWindowHandle,
                                                        glXC->color, 
                                                        glXC->depth, 
                                                        glXC->stencil, 
                                                        glXC->accum, 
                                                        false);
        
        glXC->aglContext = glXCt->aglContext;
        
        // delete the container for the nonfullscreen context
        delete glXCt;

        // set us back..
        SetContext(glXC);
        
        return true;
    }

    ContextGL* WindowGL::CreateContext(uint32 color, uint32 depth, 
                                       uint32 stencil, uint32 accum)
    {
        return CreateContextWorkAround(mWindowHandle,color, depth, 
                                 stencil, accum, false);
    }


    void WindowGL::DeleteContext(ContextGL* context)
    {
        if(context == mCurrentContext)
            mCurrentContext=NULL;
        
        ContextGLX *glXC = (ContextGLX *)context;
		aglSetCurrentContext (NULL);
		aglSetDrawable (glXC->aglContext, NULL);
		if (glXC->aglContext) {
			aglDestroyContext (glXC->aglContext);
			glXC->aglContext = NULL;
		}
		if (glXC->aglPixFmt) {
			aglDestroyPixelFormat (glXC->aglPixFmt);
			glXC->aglPixFmt = NULL;
		}
    }


    void WindowGL::SetContext(ContextGL* context)
    {
        ContextGLX *glXC = (ContextGLX *)context;
        aglSetCurrentContext(glXC->aglContext);
        mCurrentContext = context;
    }

    void WindowGL::PageFlip(bool retrace)
    {
        ContextGLX *glXC = (ContextGLX *)mCurrentContext;
        
        GrafPtr portSave;
        GetPort (&portSave);
        SetPort (GetWindowPort (mWindowHandle));
        aglSwapBuffers (glXC->aglContext);
        SetPort (portSave);        
    }
    
}

