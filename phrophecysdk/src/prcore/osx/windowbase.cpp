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
 WindowBase Carbon/OSX implementation
 
	revision history:
 December/25/2003 - Stefan Karlsson
 */
#include <prcore/timer.hpp>
#include <prcore/keyboard.hpp>
#include <prcore/mouse.hpp>
#include <prcore/windowbase.hpp>
#include <prcore/exception.hpp>
#include <prcore/endian.hpp>

#include <Carbon/Carbon.h> 
#include <OpenGL/gl.h> 
#include <AGL/agl.h> 

namespace prcore
{
    
    
    // ======================================
    // globals
    // ======================================

    
    int remapKeyCodes(int code)
    {
        switch(code)
        {
            // Misc utilitykeys
            case 53:        return KEYCODE_ESC;
            case 48:        return KEYCODE_TAB;

            // LEFT
            case 36:        return KEYCODE_ENTER;
            // RIGHT
            case 76:        return KEYCODE_ENTER;
                
            // Function keys
            case 122:       return KEYCODE_F1;
            case 120:       return KEYCODE_F2;
            case 99:        return KEYCODE_F3;
            case 118:       return KEYCODE_F4;
            case 96:        return KEYCODE_F5;
            case 97:        return KEYCODE_F6;
            case 98:        return KEYCODE_F7;
            case 100:       return KEYCODE_F8;
            case 101:       return KEYCODE_F9;
            case 109:       return KEYCODE_F10;
            case 103:       return KEYCODE_F11;
            case 111:       return KEYCODE_F12;
                
            // Navigation Keys
            case 115:       return KEYCODE_HOME;
            case 116:       return KEYCODE_PAGEUP;
            case 121:       return KEYCODE_PAGEDOWN;
            case 119:       return KEYCODE_END;
            case 117:       return KEYCODE_DELETE;
            case 51:        return KEYCODE_BACKSPACE;
            case 124:       return KEYCODE_RIGHT;
            case 123:       return KEYCODE_LEFT;
            case 125:       return KEYCODE_DOWN;
            case 126:       return KEYCODE_UP;
                
            // Numpads
            case 82:        return KEYCODE_NUMPAD0;
            case 83:        return KEYCODE_NUMPAD1;
            case 84:        return KEYCODE_NUMPAD2;
            case 85:        return KEYCODE_NUMPAD3;
            case 86:        return KEYCODE_NUMPAD4;
            case 87:        return KEYCODE_NUMPAD5;
            case 88:        return KEYCODE_NUMPAD6;
            case 89:        return KEYCODE_NUMPAD7;
            case 91:        return KEYCODE_NUMPAD8;
            case 92:        return KEYCODE_NUMPAD9;

            // Not a familair "extended" key.
            default:        return 0;
        }
    }
    
    int remapChar(int code)
    {
        code = std::toupper(code);

		switch ( code )
		{
			case '0':			return KEYCODE_0;
			case '1':			return KEYCODE_1;
			case '2':			return KEYCODE_2;
			case '3':			return KEYCODE_3;
			case '4':			return KEYCODE_4;
			case '5':			return KEYCODE_5;
			case '6':			return KEYCODE_6;
			case '7':			return KEYCODE_7;
			case '8':			return KEYCODE_8;
			case '9':			return KEYCODE_9;
			case 'A':			return KEYCODE_A;
			case 'B':			return KEYCODE_B;
			case 'C':			return KEYCODE_C;
			case 'D':			return KEYCODE_D;
			case 'E':			return KEYCODE_E;
			case 'F':			return KEYCODE_F;
			case 'G':			return KEYCODE_G;
			case 'H':			return KEYCODE_H;
			case 'I':			return KEYCODE_I;
			case 'J':			return KEYCODE_J;
			case 'K':			return KEYCODE_K;
			case 'L':			return KEYCODE_L;
			case 'M':			return KEYCODE_M;
			case 'N':			return KEYCODE_N;
			case 'O':			return KEYCODE_O;
			case 'P':			return KEYCODE_P;
			case 'Q':			return KEYCODE_Q;
			case 'R':			return KEYCODE_R;
			case 'S':			return KEYCODE_S;
			case 'T':			return KEYCODE_T;
			case 'U':			return KEYCODE_U;
			case 'V':			return KEYCODE_V;
			case 'W':			return KEYCODE_W;
			case 'X':			return KEYCODE_X;
			case 'Y':			return KEYCODE_Y;
			case 'Z':			return KEYCODE_Z;
            case '+':           return KEYCODE_ADDITION;
            case '-':           return KEYCODE_SUBTRACT;
            case '/':           return KEYCODE_DIVIDE;
            case '*':           return KEYCODE_MULTIPLY;
    
			default:			return 0;
		}
    }
    
    // Prophecy string to Pascal string
    static void cstr2pstr ( char * outString, const String &inString)
    {
        unsigned short x = 0;
        do {
            outString [x + 1] = inString [x];
            x++;
        } while ((inString [x] != 0)  && (x < 256));
        outString [0] = x;
    }
    
    // Data we need for our window.. To be identified with the eventhandler.
    typedef struct{
        EventHandlerRef     EventHandlerReference;
        EventLoopTimerRef   EventMainLoopReference;
        
        EventLoopTimerUPP   MainLoopEventHandler;        
        EventHandlerUPP     WindowEventHandler;
        
        WindowBase          *WindowBaseOwner;  
        bool                StartedApplicationLoop;
    }OSXWindowData;
    
    static OSStatus handleKeyInput (EventHandlerCallRef myHandler, EventRef event, 
                                    Boolean keyDown, OSXWindowData* oW)
    {
        WindowBase *wB = oW->WindowBaseOwner;
        WindowRef window = NULL;
        window = wB->GetWindowHandle();
      
        OSStatus result = eventNotHandledErr;
        result = CallNextEventHandler(myHandler, event);	
        if (eventNotHandledErr == result) { 
            UInt32 keyCode;
            char keyChar;
            GetEventParameter (event, kEventParamKeyCode, typeUInt32, NULL, sizeof(UInt32), NULL, &keyCode);
            GetEventParameter (event, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(char), NULL, &keyChar);
            // handle keyboard input here
            int prophecyKeyCode = remapChar(keyChar);
            int prophecyKeyCodeExtended = prophecyKeyCode;
            if(prophecyKeyCode == 0)
                    prophecyKeyCodeExtended = prophecyKeyCode = remapKeyCodes(keyCode);
            else
                prophecyKeyCodeExtended = remapKeyCodes(keyCode);
            if( prophecyKeyCodeExtended != prophecyKeyCode && prophecyKeyCodeExtended != 0 )
                prophecyKeyCode = prophecyKeyCodeExtended;

            wB->DispatchEventKeyboard(prophecyKeyCode,keyChar,keyDown);
        }
        return result;
    }
     
    static pascal void MainLoopEventHandler (EventLoopTimerRef inTimer, void* userData)
    {
        OSXWindowData *oW = (OSXWindowData*)userData;
        WindowBase *wB = oW->WindowBaseOwner;
        wB->EventMain();
    }
    
    static pascal OSStatus WindowEventHandler (EventHandlerCallRef myHandler, EventRef event, void* userData)
    {
        WindowRef			window = NULL;
        OSStatus			result = eventNotHandledErr;
        UInt32 				msgClass = GetEventClass (event);
        UInt32 				kind = GetEventKind (event);
        OSXWindowData *oW = (OSXWindowData*)userData;
        
        WindowBase *wB = oW->WindowBaseOwner;
        window = wB->GetWindowHandle();
        ::Rect rectPort;
        
        switch (msgClass) {
            case kEventClassMouse:
                {
                    EventMouseButton button;
                    GetEventParameter (event, kEventParamMouseButton, typeMouseButton, 
                                       NULL, sizeof(EventMouseButton), NULL, &button);
                
                    HIPoint location;
                    GetEventParameter (event, kEventParamWindowMouseLocation, typeHIPoint, 
                                       NULL, sizeof(HIPoint), NULL, &location);

                    UInt32 clickCount;
                    GetEventParameter (event, kEventParamClickCount, typeUInt32, 
                                       NULL, sizeof(UInt32), NULL, &clickCount);

                    if(kind == kEventMouseDown || kind == kEventMouseUp)
                    {
                        MouseButton mB;
                        switch(button)
                        {
                            case kEventMouseButtonPrimary:
                                mB = MOUSE_LEFT;
                                break;
                            case kEventMouseButtonSecondary:
                                mB = MOUSE_MIDDLE;
                                break;
                            case kEventMouseButtonTertiary:
                                mB = MOUSE_RIGHT;
                                break;
                        }
                        wB->EventMouseButton((int)location.x,(int)location.y, mB, clickCount);
                    }
                    else
                        wB->EventMouseMove((int)location.x,(int)location.y);
                }
                break;
            case kEventClassKeyboard:
                switch (kind) {
                    case kEventRawKeyDown:
                        result = handleKeyInput  (myHandler, event, true, oW);
                        break;
                    case kEventRawKeyUp:
                        result = handleKeyInput  (myHandler, event, false, oW);
                        break;
                }
                break;
            case kEventClassWindow:
                GetEventParameter(event, kEventParamDirectObject, typeWindowRef, 
                                  NULL, sizeof(WindowRef), NULL, &window);
                switch (kind) {
                    case kEventWindowDeactivated:
                        wB->EventFocus(false);
                        break;
                    case kEventWindowActivated:
                        wB->EventFocus(true);
                        break;
                    case kEventWindowDrawContent:
                        wB->EventDraw();
                        break;
                    case kEventWindowBoundsChanged:
                    {
                        Rect rect = wB->GetWindowRect();
                        wB->EventSize(rect.width, rect.height);
                        break;                        
                    }
                    case kEventWindowClose:
                        wB->EventClose();
                        wB->DisableEvent();
                        break;
                    // WARNING: This is not called if windows is unminimized!
                    case kEventWindowShown:
                        SetUserFocusWindow (window);
                        GetWindowPortBounds (window, &rectPort);
                        InvalWindowRect (window, &rectPort);
                        wB->EventMain();
                        break;
                }
                break;
        }
        return result;
    }
    
    
    WindowBase::WindowBase()
        : mMainFreq(0.001),mpPrevProc(NULL),mIsOpen(false),mIsBind(false),
        mIsEventEnable(false)
    {
            mWindowParent = 0;
            for ( int i=0; i<256; ++i )
                mKeyDown[i] = false;
    }
    
    
    WindowBase::WindowBase(WindowHandle parent)
    : mMainFreq(0.001),mpPrevProc(NULL),mIsOpen(false),mIsBind(false),
    mIsEventEnable(false)
    {
        mWindowParent = parent;
        for ( int i=0; i<256; ++i )
            mKeyDown[i] = false;

    }


    WindowBase::~WindowBase()
    {
    }


    bool WindowBase::BindWindow(WindowHandle handle)
    {
        // TODO
        return false;
    }


    bool WindowBase::UnbindWindow()
    {
        // TODO
        return false;
    }

    bool WindowBase::OpenWindow(int width, int height, const String& name, uint32 windowstyle, bool show)
    {
        
        if ( IsWindowOpen() )
            CloseWindow();
        
        if ( IsWindowBind() )
            UnbindWindow();

        
        mWindowHandle = NULL; 
        ::Rect rectWin = {0, 0, height, width }; 
        
        WindowAttributes winAttribute = (windowstyle & WINDOW_FRAME) 
                                        ? kWindowStandardDocumentAttributes 
                                        : 0;
        
        if ( !(windowstyle & WINDOW_SIZE) )
            winAttribute &= ~kWindowResizableAttribute;
        if ( !(windowstyle & WINDOW_MINIMIZE) )	winAttribute &= ~kWindowFullZoomAttribute;
        
        winAttribute |= kWindowStandardHandlerAttribute|kWindowMetalAttribute;
        PRCORE_ASSERT_EXCEPTION( noErr !=  CreateNewWindow (kDocumentWindowClass,winAttribute,
                                                            &rectWin,&mWindowHandle) );
        
        mIsOpen = true;
        
        RenameWindow(name);
     
        ::SetWindowBounds ( mWindowHandle,kWindowContentRgn,&rectWin);
        EnableEvent();
        
        if(show)
            ShowWindow();
        ResizeWindow(width,height);
        if(show)
        {
            ::SetPortWindowPort(mWindowHandle); 
            ::InvalWindowRect (mWindowHandle, &rectWin);
            MoveWindow(50,50);            
        }
            
        return true;
    }


    bool WindowBase::CloseWindow(bool mainbreak)
    {
        if ( IsWindowOpen() )
        {
            EventClose();
            
            if( mIsEventEnable )
                DisableEvent();
                
            ::DisposeWindow(mWindowHandle);
            return true;
        }
        return false;
    }


    void WindowBase::RenameWindow(const String& name)
    {
        char pascalStringTitle[1024];
        cstr2pstr( pascalStringTitle, name);
        ::SetWTitle(mWindowHandle, (ConstStr255Param)pascalStringTitle);  
    }


    void WindowBase::ResizeWindow(int width, int height)
    {
        ::Rect rect;
        
        ::GetWindowBounds (mWindowHandle,kWindowContentRgn,&rect);
        
        rect.bottom = rect.top + height;
        rect.right = rect.left + width;
        
        ::SetWindowBounds ( mWindowHandle,kWindowContentRgn,&rect);
    }


    void WindowBase::MoveWindow(int x, int y)
    {
        ::MoveWindow (mWindowHandle, x,y,true);
    }


    void WindowBase::ShowCursor()
    {
        ::ShowCursor();
    }


    void WindowBase::HideCursor()
    {
        ::HideCursor();
    }


    void WindowBase::ShowWindow()
    {
        ::ShowWindow(mWindowHandle);
    }


    void WindowBase::HideWindow()
    {
        ::HideWindow(mWindowHandle);
    }

    void WindowBase::EnableEvent()
    {
        
        OSXWindowData *windowData = new OSXWindowData;

        windowData->WindowBaseOwner = this;
            
        EventTypeSpec list[] = {    { kEventClassWindow, kEventWindowShown },
                                    { kEventClassWindow, kEventWindowActivated },
                                    { kEventClassWindow, kEventWindowDeactivated },
                                    { kEventClassWindow, kEventWindowClose },
                                    { kEventClassWindow, kEventWindowDrawContent },
                                    { kEventClassWindow, kEventWindowBoundsChanged },
            
                                    { kEventClassKeyboard, kEventRawKeyDown },
                                    { kEventClassKeyboard, kEventRawKeyUp },
            
                                    { kEventClassMouse, kEventMouseMoved },
                                    { kEventClassMouse, kEventMouseDown },
                                    { kEventClassMouse, kEventMouseUp }
                                };
                                    
        windowData->WindowEventHandler = NewEventHandlerUPP(WindowEventHandler);
        PRCORE_ASSERT_EXCEPTION(
                                noErr != InstallWindowEventHandler (
                                               mWindowHandle, 
                                               windowData->WindowEventHandler, 
                                               GetEventTypeCount (list), list, 
                                               (void*)windowData, 
                                               &windowData->EventHandlerReference
                                            )
            );
        
        windowData->StartedApplicationLoop = false;
        windowData->EventMainLoopReference = NULL;
        SetWRefCon (mWindowHandle,(long)windowData);
        mIsEventEnable=true;
    }


    void WindowBase::DisableEvent()
    {
        if( !mIsEventEnable )
            return;
        OSXWindowData *windowData = (OSXWindowData *) GetWRefCon(mWindowHandle);
        if(windowData != NULL)
        {
            if( windowData->StartedApplicationLoop )
                QuitApplicationEventLoop(); 
            PRCORE_ASSERT_EXCEPTION( noErr != RemoveEventHandler (windowData->EventHandlerReference) );
            
            PRCORE_ASSERT_EXCEPTION( noErr != RemoveEventLoopTimer (windowData->EventMainLoopReference) );
            windowData->EventMainLoopReference = NULL;
            
            delete windowData;
        }

        mIsEventEnable=false;
    }


    void WindowBase::SetMainFrequency(float freq)
    {
        assert( freq >= 0 );
        mMainFreq = freq;
    }


    float WindowBase::GetMainFrequency() const
    {
      	return mMainFreq;
    }


    void WindowBase::MainBreak()
    {
        CloseWindow();
    }


    void WindowBase::MainLoop()
    {
        OSXWindowData *windowData = (OSXWindowData*) GetWRefCon (mWindowHandle);
        windowData->StartedApplicationLoop = true;
        windowData->MainLoopEventHandler = NewEventLoopTimerUPP(MainLoopEventHandler);
        PRCORE_ASSERT_EXCEPTION( noErr !=  InstallEventLoopTimer (GetCurrentEventLoop(),
                                        0,
                                        mMainFreq,
                                        windowData->MainLoopEventHandler,
                                        (void *) windowData,
                                        &windowData->EventMainLoopReference
                                        ));
        
        
        RunApplicationEventLoop();
    }
    
    
    bool WindowBase::IsWindowOpen() const
    {
        return mIsOpen;
    }


    bool WindowBase::IsWindowBind() const
    {
        return mIsBind;
    }


    bool WindowBase::IsEventEnable() const
    {
        return mIsEventEnable;
    }


    uint32 WindowBase::GetWindowStyle() const
    {
        return mWindowStyle;
    }


    WindowHandle WindowBase::GetWindowHandle() const
    {
        return mWindowHandle;
    }


    Rect WindowBase::GetWindowRect(bool frame) const
    {
        ::Rect rect;
        ::GetWindowBounds (mWindowHandle,kWindowContentRgn,&rect);
        return Rect(rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top);
    }


    float WindowBase::GetWindowAspect() const
    {
        Rect rect = GetWindowRect();
        return rect.GetAspect();
    }


    bool WindowBase::IsDown(int keycode) const
    {
       	return mKeyDown[keycode & 0xff];
    }


    void WindowBase::DispatchEventKeyboard(int keycode, char charcode, bool press)
    {
        // update the keymap
        mKeyDown[keycode & 0xff] = press;
        
        // tell the client key was pressed or released
        EventKeyboard(keycode,charcode,press);
    }


    void* WindowBase::GetPrevProc() const
    {
        return mpPrevProc;
    }
}