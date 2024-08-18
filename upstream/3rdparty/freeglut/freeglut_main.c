/* +---------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)               |
   |                          https://www.mrpt.org/                            |
   |                                                                           |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file        |
   | See: https://www.mrpt.org/Authors - All rights reserved.                  |
   | Released under BSD License. See details in https://www.mrpt.org/License   |
   +---------------------------------------------------------------------------+ */

#include <GL/freeglut.h>
#include "freeglut_internal.h"
#include <errno.h>
#include <stdarg.h>
#if TARGET_HOST_WIN32
#    define VFPRINTF(s,f,a) vfprintf((s),(f),(a))
#else
#    if HAVE_VPRINTF
#        define VFPRINTF(s,f,a) vfprintf((s),(f),(a))
#    elif HAVE_DOPRNT
#        define VFPRINTF(s,f,a) _doprnt((f),(a),(s))
#    else
#        define VFPRINTF(s,f,a)
#    endif
#endif

#if TARGET_HOST_WINCE

typedef struct GXDisplayProperties GXDisplayProperties;
typedef struct GXKeyList GXKeyList;
#include <gx.h>

typedef struct GXKeyList (*GXGETDEFAULTKEYS)(int);
typedef int (*GXOPENINPUT)();

GXGETDEFAULTKEYS GXGetDefaultKeys_ = NULL;
GXOPENINPUT GXOpenInput_ = NULL;

struct GXKeyList gxKeyList;

#endif

/*
 * Try to get the maximum value allowed for ints, falling back to the minimum
 * guaranteed by ISO C99 if there is no suitable header.
 */
#if HAVE_LIMITS_H
#    include <limits.h>
#endif
#ifndef INT_MAX
#    define INT_MAX 32767
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#endif


/*
 * TODO BEFORE THE STABLE RELEASE:
 *
 * There are some issues concerning window redrawing under X11, and maybe
 * some events are not handled. The Win32 version lacks some more features,
 * but seems acceptable for not demanding purposes.
 *
 * Need to investigate why the X11 version breaks out with an error when
 * closing a window (using the window manager, not glutDestroyWindow)...
 */

/* -- PRIVATE FUNCTIONS ---------------------------------------------------- */

/*
 * Handle a window configuration change. When no reshape
 * callback is hooked, the viewport size is updated to
 * match the new window size.
 */
static void fghReshapeWindow ( SFG_Window *window, int width, int height )
{
    SFG_Window *current_window = fgStructure.CurrentWindow;

    freeglut_return_if_fail( window != NULL );


#if TARGET_HOST_UNIX_X11

    XResizeWindow( fgDisplay.Display, window->Window.Handle,
                   width, height );
    XFlush( fgDisplay.Display ); /* XXX Shouldn't need this */

#elif TARGET_HOST_WIN32
    {
        RECT winRect;
        int x, y, w, h;

        /*
         * For windowed mode, get the current position of the
         * window and resize taking the size of the frame
         * decorations into account.
         */

        /* "GetWindowRect" returns the pixel coordinates of the outside of the window */
        GetWindowRect( window->Window.Handle, &winRect );
        x = winRect.left;
        y = winRect.top;
        w = width;
        h = height;

        if ( window->Parent == NULL )
        {
            if ( ! window->IsMenu && !window->State.IsGameMode )
            {
                w += GetSystemMetrics( SM_CXSIZEFRAME ) * 2;
                h += GetSystemMetrics( SM_CYSIZEFRAME ) * 2 +
                     GetSystemMetrics( SM_CYCAPTION );
            }
        }
        else
        {
            RECT parentRect;
            GetWindowRect( window->Parent->Window.Handle, &parentRect );
            x -= parentRect.left + GetSystemMetrics( SM_CXSIZEFRAME ) * 2;
            y -= parentRect.top  + GetSystemMetrics( SM_CYSIZEFRAME ) * 2 +
                                   GetSystemMetrics( SM_CYCAPTION );
        }

        /*
         * SWP_NOACTIVATE      Do not activate the window
         * SWP_NOOWNERZORDER   Do not change position in z-order
         * SWP_NOSENDCHANGING  Supress WM_WINDOWPOSCHANGING message
         * SWP_NOZORDER        Retains the current Z order (ignore 2nd param)
         */

        SetWindowPos( window->Window.Handle,
                      HWND_TOP,
                      x, y, w, h,
                      SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING |
                      SWP_NOZORDER
        );
    }
#endif

    /*
     * XXX Should update {window->State.OldWidth, window->State.OldHeight}
     * XXX to keep in lockstep with UNIX_X11 code.
     */
    if( FETCH_WCB( *window, Reshape ) )
        INVOKE_WCB( *window, Reshape, ( width, height ) );
    else
    {
        fgSetWindow( window );
        glViewport( 0, 0, width, height );
    }

    /*
     * Force a window redraw.  In Windows at least this is only a partial
     * solution:  if the window is increasing in size in either dimension,
     * the already-drawn part does not get drawn again and things look funny.
     * But without this we get this bad behaviour whenever we resize the
     * window.
     */
    window->State.Redisplay = GL_TRUE;

    if( window->IsMenu )
        fgSetWindow( current_window );
}

/*
 * Calls a window's redraw method. This is used when
 * a redraw is forced by the incoming window messages.
 */
static void fghRedrawWindow ( SFG_Window *window )
{
    SFG_Window *current_window = fgStructure.CurrentWindow;

    freeglut_return_if_fail( window );
    freeglut_return_if_fail( FETCH_WCB ( *window, Display ) );

    window->State.Redisplay = GL_FALSE;

    freeglut_return_if_fail( window->State.Visible );

    fgSetWindow( window );

    if( window->State.NeedToResize )
    {
        fghReshapeWindow(
            window,
            window->State.Width,
            window->State.Height
        );

        window->State.NeedToResize = GL_FALSE;
    }

    INVOKE_WCB( *window, Display, ( ) );

    fgSetWindow( current_window );
}

/*
 * A static helper function to execute display callback for a window
 */
static void fghcbDisplayWindow( SFG_Window *window,
                                SFG_Enumerator *enumerator )
{
    if( window->State.Redisplay &&
        window->State.Visible )
    {
        window->State.Redisplay = GL_FALSE;

#if TARGET_HOST_UNIX_X11
        fghRedrawWindow ( window ) ;
#elif TARGET_HOST_WIN32 || TARGET_HOST_WINCE
        RedrawWindow(
            window->Window.Handle, NULL, NULL,
            RDW_NOERASE | RDW_INTERNALPAINT | RDW_INVALIDATE | RDW_UPDATENOW
        );
#endif
    }

    fgEnumSubWindows( window, fghcbDisplayWindow, enumerator );
}

/*
 * Make all windows perform a display call
 */
static void fghDisplayAll( void )
{
    SFG_Enumerator enumerator;

    enumerator.found = GL_FALSE;
    enumerator.data  =  NULL;

    fgEnumWindows( fghcbDisplayWindow, &enumerator );
}

/*
 * Window enumerator callback to check for the joystick polling code
 */
static void fghcbCheckJoystickPolls( SFG_Window *window,
                                     SFG_Enumerator *enumerator )
{
    long int checkTime = fgElapsedTime( );

    if( window->State.JoystickLastPoll + window->State.JoystickPollRate <=
        checkTime )
    {
#if !TARGET_HOST_WINCE
        fgJoystickPollWindow( window );
#endif /* !TARGET_HOST_WINCE */
        window->State.JoystickLastPoll = checkTime;
    }

    fgEnumSubWindows( window, fghcbCheckJoystickPolls, enumerator );
}

/*
 * Check all windows for joystick polling
 */
static void fghCheckJoystickPolls( void )
{
    SFG_Enumerator enumerator;

    enumerator.found = GL_FALSE;
    enumerator.data  =  NULL;

    fgEnumWindows( fghcbCheckJoystickPolls, &enumerator );
}

/*
 * Check the global timers
 */
static void fghCheckTimers( void )
{
    long checkTime = fgElapsedTime( );

    while( fgState.Timers.First )
    {
        SFG_Timer *timer = fgState.Timers.First;

        if( timer->TriggerTime > checkTime )
            break;

        fgListRemove( &fgState.Timers, &timer->Node );
        fgListAppend( &fgState.FreeTimers, &timer->Node );

        timer->Callback( timer->ID );
    }
}

/*
 * Elapsed Time
 */
long fgElapsedTime( void )
{
    if ( fgState.Time.Set )
    {
#if TARGET_HOST_UNIX_X11
        struct timeval now;
        long elapsed;

        gettimeofday( &now, NULL );

        elapsed = (now.tv_usec - fgState.Time.Value.tv_usec) / 1000;
        elapsed += (now.tv_sec - fgState.Time.Value.tv_sec) * 1000;

        return elapsed;
#elif TARGET_HOST_WIN32
        return timeGetTime() - fgState.Time.Value;
#elif TARGET_HOST_WINCE
        return GetTickCount() - fgState.Time.Value;
#endif
    }
    else
    {
#if TARGET_HOST_UNIX_X11
        gettimeofday( &fgState.Time.Value, NULL );
#elif TARGET_HOST_WIN32
        fgState.Time.Value = timeGetTime ();
#elif TARGET_HOST_WINCE
        fgState.Time.Value = GetTickCount();
#endif
        fgState.Time.Set = GL_TRUE ;

        return 0 ;
    }
}

/*
 * Error Messages.
 */
void fgError( const char *fmt, ... )
{
    va_list ap;

    va_start( ap, fmt );

    fprintf( stderr, "freeglut ");
    if( fgState.ProgramName )
        fprintf( stderr, "(%s): ", fgState.ProgramName );
    VFPRINTF( stderr, fmt, ap );
    fprintf( stderr, "\n" );

    va_end( ap );

    if ( fgState.Initialised )
        fgDeinitialize ();

    exit( 1 );
}

void fgWarning( const char *fmt, ... )
{
    va_list ap;

    va_start( ap, fmt );

    fprintf( stderr, "freeglut ");
    if( fgState.ProgramName )
        fprintf( stderr, "(%s): ", fgState.ProgramName );
    VFPRINTF( stderr, fmt, ap );
    fprintf( stderr, "\n" );

    va_end( ap );
}

/*
 * Indicates whether Joystick events are being used by ANY window.
 *
 * The current mechanism is to walk all of the windows and ask if
 * there is a joystick callback.  We have a short-circuit early
 * return if we find any joystick handler registered.
 *
 * The real way to do this is to make use of the glutTimer() API
 * to more cleanly re-implement the joystick API.  Then, this code
 * and all other "joystick timer" code can be yanked.
 *
 */
static void fghCheckJoystickCallback( SFG_Window* w, SFG_Enumerator* e)
{
    if( FETCH_WCB( *w, Joystick ) )
    {
        e->found = GL_TRUE;
        e->data = w;
    }
    fgEnumSubWindows( w, fghCheckJoystickCallback, e );
}
static int fghHaveJoystick( void )
{
    SFG_Enumerator enumerator;

    enumerator.found = GL_FALSE;
    enumerator.data = NULL;
    fgEnumWindows( fghCheckJoystickCallback, &enumerator );
    return !!enumerator.data;
}
static void fghHavePendingRedisplaysCallback( SFG_Window* w, SFG_Enumerator* e)
{
    if( w->State.Redisplay )
    {
        e->found = GL_TRUE;
        e->data = w;
    }
    fgEnumSubWindows( w, fghHavePendingRedisplaysCallback, e );
}
static int fghHavePendingRedisplays (void)
{
    SFG_Enumerator enumerator;

    enumerator.found = GL_FALSE;
    enumerator.data = NULL;
    fgEnumWindows( fghHavePendingRedisplaysCallback, &enumerator );
    return !!enumerator.data;
}
/*
 * Returns the number of GLUT ticks (milliseconds) till the next timer event.
 */
static long fghNextTimer( void )
{
    long ret = INT_MAX;
    SFG_Timer *timer = fgState.Timers.First;

    if( timer )
        ret = timer->TriggerTime - fgElapsedTime();
    if( ret < 0 )
        ret = 0;

    return ret;
}
/*
 * Does the magic required to relinquish the CPU until something interesting
 * happens.
 */
static void fghSleepForEvents( void )
{
    long msec;

    if( fgState.IdleCallback || fghHavePendingRedisplays( ) )
        return;

    msec = fghNextTimer( );
    /* XXX Use GLUT timers for joysticks... */
    /* XXX Dumb; forces granularity to .01sec */
    if( fghHaveJoystick( ) && ( msec > 10 ) )     
        msec = 10;

#if TARGET_HOST_UNIX_X11
    /*
     * Possibly due to aggressive use of XFlush() and friends,
     * it is possible to have our socket drained but still have
     * unprocessed events.  (Or, this may just be normal with
     * X, anyway?)  We do non-trivial processing of X events
     * after the event-reading loop, in any case, so we
     * need to allow that we may have an empty socket but non-
     * empty event queue.
     */
    if( ! XPending( fgDisplay.Display ) )
    {
        fd_set fdset;
        int err;
        int socket;
        struct timeval wait;

        socket = ConnectionNumber( fgDisplay.Display );
        FD_ZERO( &fdset );
        FD_SET( socket, &fdset );
        wait.tv_sec = msec / 1000;
        wait.tv_usec = (msec % 1000) * 1000;
        err = select( socket+1, &fdset, NULL, NULL, &wait );

        if( ( -1 == err ) && ( errno != EINTR ) )
            fgWarning ( "freeglut select() error: %d", errno );
    }
#elif TARGET_HOST_WIN32 || TARGET_HOST_WINCE
    MsgWaitForMultipleObjects( 0, NULL, FALSE, msec, QS_ALLEVENTS );
#endif
}

#if TARGET_HOST_UNIX_X11
/*
 * Returns GLUT modifier mask for an XEvent.
 */
static int fghGetXModifiers( XEvent *event )
{
    int ret = 0;

    if( event->xkey.state & ( ShiftMask | LockMask ) )
        ret |= GLUT_ACTIVE_SHIFT;
    if( event->xkey.state & ControlMask )
        ret |= GLUT_ACTIVE_CTRL;
    if( event->xkey.state & Mod1Mask )
        ret |= GLUT_ACTIVE_ALT;

    return ret;
}
#endif


/* -- INTERFACE FUNCTIONS -------------------------------------------------- */

/*
 * Executes a single iteration in the freeglut processing loop.
 */
void FGAPIENTRY glutMainLoopEvent( void )
{
#if TARGET_HOST_UNIX_X11
    SFG_Window* window;
    XEvent event;

    /* This code was repeated constantly, so here it goes into a definition: */
#define GETWINDOW(a)                             \
    window = fgWindowByHandle( event.a.window ); \
    if( window == NULL )                         \
        break;

#define GETMOUSE(a)                              \
    window->State.MouseX = event.a.x;            \
    window->State.MouseY = event.a.y;

    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutMainLoopEvent" );

    while( XPending( fgDisplay.Display ) )
    {
        XNextEvent( fgDisplay.Display, &event );

        switch( event.type )
        {
        case ClientMessage:
            /* Destroy the window when the WM_DELETE_WINDOW message arrives */
            if( (Atom) event.xclient.data.l[ 0 ] == fgDisplay.DeleteWindow )
            {
                GETWINDOW( xclient );

                fgDestroyWindow ( window );

                if( fgState.ActionOnWindowClose == GLUT_ACTION_EXIT )
                {
                    fgDeinitialize( );
                    exit( 0 );
                }
                else if( fgState.ActionOnWindowClose == GLUT_ACTION_GLUTMAINLOOP_RETURNS )
                    fgState.ExecState = GLUT_EXEC_STATE_STOP;

                return;
            }
            break;

            /*
             * CreateNotify causes a configure-event so that sub-windows are
             * handled compatibly with GLUT.  Otherwise, your sub-windows
             * (in freeglut only) will not get an initial reshape event,
             * which can break things.
             *
             * GLUT presumably does this because it generally tries to treat
             * sub-windows the same as windows.
             *
             * XXX Technically, GETWINDOW( xconfigure ) and
             * XXX {event.xconfigure} may not be legit ways to get at
             * XXX data for CreateNotify events.  In practice, the data
             * XXX is in a union which is laid out much the same either
             * XXX way.  But if you want to split hairs, this isn't legit,
             * XXX and we should instead duplicate some code.
             */
        case CreateNotify:
        case ConfigureNotify:
            GETWINDOW( xconfigure );
            {
                int width = event.xconfigure.width;
                int height = event.xconfigure.height;

                if( ( width != window->State.OldWidth ) ||
                    ( height != window->State.OldHeight ) )
                {
                    SFG_Window *current_window = fgStructure.CurrentWindow;

                    window->State.OldWidth = width;
                    window->State.OldHeight = height;
                    if( FETCH_WCB( *window, Reshape ) )
                        INVOKE_WCB( *window, Reshape, ( width, height ) );
                    else
                    {
                        fgSetWindow( window );
                        glViewport( 0, 0, width, height );
                    }
                    glutPostRedisplay( );
                    if( window->IsMenu )
                        fgSetWindow( current_window );
                }
            }
            break;

        case DestroyNotify:
            /*
             * This is sent to confirm the XDestroyWindow call.
             *
             * XXX WHY is this commented out?  Should we re-enable it?
             */
            /* fgAddToWindowDestroyList ( window ); */
            break;

        case Expose:
            /*
             * We are too dumb to process partial exposes...
             *
             * XXX Well, we could do it.  However, it seems to only
             * XXX be potentially useful for single-buffered (since
             * XXX double-buffered does not respect viewport when we
             * XXX do a buffer-swap).
             *
             */
            if( event.xexpose.count == 0 )
            {
                GETWINDOW( xexpose );
                window->State.Redisplay = GL_TRUE;
            }
            break;

        case MapNotify:
        case UnmapNotify:
            /*
             * If we never do anything with this, can we just not ask to
             * get these messages?
             */
            break;

        case MappingNotify:
            /*
             * Have the client's keyboard knowledge updated (xlib.ps,
             * page 206, says that's a good thing to do)
             */
            XRefreshKeyboardMapping( (XMappingEvent *) &event );
            break;

        case VisibilityNotify:
        {
            GETWINDOW( xvisibility );
            /*
             * XXX INVOKE_WCB() does this check for us.
             */
            if( ! FETCH_WCB( *window, WindowStatus ) )
                break;
            fgSetWindow( window );

            /*
             * Sending this event, the X server can notify us that the window
             * has just acquired one of the three possible visibility states:
             * VisibilityUnobscured, VisibilityPartiallyObscured or
             * VisibilityFullyObscured
             */
            switch( event.xvisibility.state )
            {
            case VisibilityUnobscured:
                INVOKE_WCB( *window, WindowStatus, ( GLUT_FULLY_RETAINED ) );
                window->State.Visible = GL_TRUE;
                break;

            case VisibilityPartiallyObscured:
                INVOKE_WCB( *window, WindowStatus,
                            ( GLUT_PARTIALLY_RETAINED ) );
                window->State.Visible = GL_TRUE;
                break;

            case VisibilityFullyObscured:
                INVOKE_WCB( *window, WindowStatus, ( GLUT_FULLY_COVERED ) );
                window->State.Visible = GL_FALSE;
                break;

            default:
                fgWarning( "Unknown X visibility state: %d",
                           event.xvisibility.state );
                break;
            }
        }
        break;

        case EnterNotify:
        case LeaveNotify:
            GETWINDOW( xcrossing );
            GETMOUSE( xcrossing );
            if( ( event.type == LeaveNotify ) && window->IsMenu &&
                window->ActiveMenu && window->ActiveMenu->IsActive )
                fgUpdateMenuHighlight( window->ActiveMenu );

            INVOKE_WCB( *window, Entry, ( ( EnterNotify == event.type ) ?
                                          GLUT_ENTERED :
                                          GLUT_LEFT ) );
            break;

        case MotionNotify:
        {
            GETWINDOW( xmotion );
            GETMOUSE( xmotion );

            if( window->ActiveMenu )
            {
                if( window == window->ActiveMenu->ParentWindow )
                {
                    window->ActiveMenu->Window->State.MouseX =
                        event.xmotion.x_root - window->ActiveMenu->X;
                    window->ActiveMenu->Window->State.MouseY =
                        event.xmotion.y_root - window->ActiveMenu->Y;
                }

                fgUpdateMenuHighlight( window->ActiveMenu );

                break;
            }

            /*
             * XXX For more than 5 buttons, just check {event.xmotion.state},
             * XXX rather than a host of bit-masks?  Or maybe we need to
             * XXX track ButtonPress/ButtonRelease events in our own
             * XXX bit-mask?
             */
#define BUTTON_MASK \
  ( Button1Mask | Button2Mask | Button3Mask | Button4Mask | Button5Mask )
            if ( event.xmotion.state & BUTTON_MASK )
                INVOKE_WCB( *window, Motion, ( event.xmotion.x,
                                               event.xmotion.y ) );
            else
                INVOKE_WCB( *window, Passive, ( event.xmotion.x,
                                                event.xmotion.y ) );
        }
        break;

        case ButtonRelease:
        case ButtonPress:
        {
            GLboolean pressed = GL_TRUE;
            int button;

            if( event.type == ButtonRelease )
                pressed = GL_FALSE ;

            /*
             * A mouse button has been pressed or released. Traditionally,
             * break if the window was found within the freeglut structures.
             */
            GETWINDOW( xbutton );
            GETMOUSE( xbutton );

            /*
             * An X button (at least in XFree86) is numbered from 1.
             * A GLUT button is numbered from 0.
             * Old GLUT passed through buttons other than just the first
             * three, though it only gave symbolic names and official
             * support to the first three.
             */
            button = event.xbutton.button - 1;

            /*
             * Do not execute the application's mouse callback if a menu
             * is hooked to this button.  In that case an appropriate
             * private call should be generated.
             */
            if( fgCheckActiveMenu( window, button, pressed,
                                   event.xbutton.x_root, event.xbutton.y_root ) )
                break;

            /*
             * Check if there is a mouse or mouse wheel callback hooked to the
             * window
             */
            if( ! FETCH_WCB( *window, Mouse ) &&
                ! FETCH_WCB( *window, MouseWheel ) )
                break;

            fgState.Modifiers = fghGetXModifiers( &event );

            /* Finally execute the mouse or mouse wheel callback */
            if( ( button < glutDeviceGet ( GLUT_NUM_MOUSE_BUTTONS ) ) || ( ! FETCH_WCB( *window, MouseWheel ) ) )
                INVOKE_WCB( *window, Mouse, ( button,
                                              pressed ? GLUT_DOWN : GLUT_UP,
                                              event.xbutton.x,
                                              event.xbutton.y )
                );
            else
            {
                /*
                 * Map 4 and 5 to wheel zero; EVEN to +1, ODD to -1
                 *  "  6 and 7 "    "   one; ...
                 *
                 * XXX This *should* be behind some variables/macros,
                 * XXX since the order and numbering isn't certain
                 * XXX See XFree86 configuration docs (even back in the
                 * XXX 3.x days, and especially with 4.x).
                 *
                 * XXX Note that {button} has already been decremeted
                 * XXX in mapping from X button numbering to GLUT.
                 */
                int wheel_number = (button - glutDeviceGet ( GLUT_NUM_MOUSE_BUTTONS )) / 2;
                int direction = -1;
                if( button % 2 )
                    direction = 1;

                if( pressed )
                    INVOKE_WCB( *window, MouseWheel, ( wheel_number,
                                                       direction,
                                                       event.xbutton.x,
                                                       event.xbutton.y )
                    );
            }

            /* Trash the modifiers state */
            fgState.Modifiers = 0xffffffff;
        }
        break;

        case KeyRelease:
        case KeyPress:
        {
            FGCBKeyboard keyboard_cb;
            FGCBSpecial special_cb;

            GETWINDOW( xkey );
            GETMOUSE( xkey );

            /* Detect auto repeated keys, if configured globally or per-window */

            if ( fgState.KeyRepeat==GLUT_KEY_REPEAT_OFF || window->State.IgnoreKeyRepeat==GL_TRUE )
            {
                if (event.type==KeyRelease)
                {
                    /*
                     * Look at X11 keystate to detect repeat mode.
                     * While X11 says the key is actually held down, we'll ignore KeyRelease/KeyPress pairs.
                     */

                    char keys[32];
                    XQueryKeymap( fgDisplay.Display, keys ); /* Look at X11 keystate to detect repeat mode */

                    if ( event.xkey.keycode<256 )            /* XQueryKeymap is limited to 256 keycodes    */
                    {
                        if ( keys[event.xkey.keycode>>3] & (1<<(event.xkey.keycode%8)) )
                            window->State.KeyRepeating = GL_TRUE;
                        else
                            window->State.KeyRepeating = GL_FALSE;
                    }
                }
            }
            else
                window->State.KeyRepeating = GL_FALSE;

            /* Cease processing this event if it is auto repeated */

            if (window->State.KeyRepeating)
                break;

            if( event.type == KeyPress )
            {
                keyboard_cb = (FGCBKeyboard)( FETCH_WCB( *window, Keyboard ));
                special_cb  = (FGCBSpecial) ( FETCH_WCB( *window, Special  ));
            }
            else
            {
                keyboard_cb = (FGCBKeyboard)( FETCH_WCB( *window, KeyboardUp ));
                special_cb  = (FGCBSpecial) ( FETCH_WCB( *window, SpecialUp  ));
            }

            /* Is there a keyboard/special callback hooked for this window? */
            if( keyboard_cb || special_cb )
            {
                XComposeStatus composeStatus;
                char asciiCode[ 32 ];
                KeySym keySym;
                int len;

                /* Check for the ASCII/KeySym codes associated with the event: */
                len = XLookupString( &event.xkey, asciiCode, sizeof(asciiCode),
                                     &keySym, &composeStatus
                );

                /* GLUT API tells us to have two separate callbacks... */
                if( len > 0 )
                {
                    /* ...one for the ASCII translateable keypresses... */
                    if( keyboard_cb )
                    {
                        fgSetWindow( window );
                        fgState.Modifiers = fghGetXModifiers( &event );
                        keyboard_cb( asciiCode[ 0 ],
                                     event.xkey.x, event.xkey.y
                        );
                        fgState.Modifiers = 0xffffffff;
                    }
                }
                else
                {
                    int special = -1;

                    /*
                     * ...and one for all the others, which need to be
                     * translated to GLUT_KEY_Xs...
                     */
                    switch( keySym )
                    {
                    case XK_F1:     special = GLUT_KEY_F1;     break;
                    case XK_F2:     special = GLUT_KEY_F2;     break;
                    case XK_F3:     special = GLUT_KEY_F3;     break;
                    case XK_F4:     special = GLUT_KEY_F4;     break;
                    case XK_F5:     special = GLUT_KEY_F5;     break;
                    case XK_F6:     special = GLUT_KEY_F6;     break;
                    case XK_F7:     special = GLUT_KEY_F7;     break;
                    case XK_F8:     special = GLUT_KEY_F8;     break;
                    case XK_F9:     special = GLUT_KEY_F9;     break;
                    case XK_F10:    special = GLUT_KEY_F10;    break;
                    case XK_F11:    special = GLUT_KEY_F11;    break;
                    case XK_F12:    special = GLUT_KEY_F12;    break;

                    case XK_Left:   special = GLUT_KEY_LEFT;   break;
                    case XK_Right:  special = GLUT_KEY_RIGHT;  break;
                    case XK_Up:     special = GLUT_KEY_UP;     break;
                    case XK_Down:   special = GLUT_KEY_DOWN;   break;

                    case XK_KP_Prior:
                    case XK_Prior:  special = GLUT_KEY_PAGE_UP; break;
                    case XK_KP_Next:
                    case XK_Next:   special = GLUT_KEY_PAGE_DOWN; break;
                    case XK_KP_Home:
                    case XK_Home:   special = GLUT_KEY_HOME;   break;
                    case XK_KP_End:
                    case XK_End:    special = GLUT_KEY_END;    break;
                    case XK_KP_Insert:
                    case XK_Insert: special = GLUT_KEY_INSERT; break;
                    }

                    /*
                     * Execute the callback (if one has been specified),
                     * given that the special code seems to be valid...
                     */
                    if( special_cb && (special != -1) )
                    {
                        fgSetWindow( window );
                        fgState.Modifiers = fghGetXModifiers( &event );
                        special_cb( special, event.xkey.x, event.xkey.y );
                        fgState.Modifiers = 0xffffffff;
                    }
                }
            }
        }
        break;

        case ReparentNotify:
            break; /* XXX Should disable this event */

        default:
            fgWarning ("Unknown X event type: %d", event.type);
            break;
        }
    }

#elif TARGET_HOST_WIN32 || TARGET_HOST_WINCE

    MSG stMsg;

    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutMainLoopEvent" );

    while( PeekMessage( &stMsg, NULL, 0, 0, PM_NOREMOVE ) )
    {
        if( GetMessage( &stMsg, NULL, 0, 0 ) == 0 )
        {
            if( fgState.ActionOnWindowClose == GLUT_ACTION_EXIT )
            {
                fgDeinitialize( );
                exit( 0 );
            }
            else if( fgState.ActionOnWindowClose == GLUT_ACTION_GLUTMAINLOOP_RETURNS )
                fgState.ExecState = GLUT_EXEC_STATE_STOP;

            return;
        }

        TranslateMessage( &stMsg );
        DispatchMessage( &stMsg );
    }
#endif

    if( fgState.Timers.First )
        fghCheckTimers( );
    fghCheckJoystickPolls( );
    fghDisplayAll( );

    fgCloseWindows( );
}

/*
 * Enters the freeglut processing loop.
 * Stays until the "ExecState" changes to "GLUT_EXEC_STATE_STOP".
 */
void FGAPIENTRY glutMainLoop( void )
{
    int action;

#if TARGET_HOST_WIN32 || TARGET_HOST_WINCE
    SFG_Window *window = (SFG_Window *)fgStructure.Windows.First ;
#endif

    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutMainLoop" );

#if TARGET_HOST_WIN32 || TARGET_HOST_WINCE
    /*
     * Processing before the main loop:  If there is a window which is open and
     * which has a visibility callback, call it.  I know this is an ugly hack,
     * but I'm not sure what else to do about it.  Ideally we should leave
     * something uninitialized in the create window code and initialize it in
     * the main loop, and have that initialization create a "WM_ACTIVATE"
     * message.  Then we would put the visibility callback code in the
     * "case WM_ACTIVATE" block below.         - John Fay -- 10/24/02
     */
    while( window )
    {
        if ( FETCH_WCB( *window, Visibility ) )
        {
            SFG_Window *current_window = fgStructure.CurrentWindow ;

            INVOKE_WCB( *window, Visibility, ( window->State.Visible ) );
            fgSetWindow( current_window );
        }

        window = (SFG_Window *)window->Node.Next ;
    }
#endif

    fgState.ExecState = GLUT_EXEC_STATE_RUNNING ;
    while( fgState.ExecState == GLUT_EXEC_STATE_RUNNING )
    {
        SFG_Window *window;

        glutMainLoopEvent( );
        /*
         * Step through the list of windows, seeing if there are any
         * that are not menus
         */
        for( window = ( SFG_Window * )fgStructure.Windows.First;
             window;
             window = ( SFG_Window * )window->Node.Next )
            if ( ! ( window->IsMenu ) )
                break;

        if( ! window )
            fgState.ExecState = GLUT_EXEC_STATE_STOP;
        else
        {
            if( fgState.IdleCallback )
            {
                if( fgStructure.CurrentWindow &&
                    fgStructure.CurrentWindow->IsMenu )
                    /* fail safe */
                    fgSetWindow( window );
                fgState.IdleCallback( );
            }

            fghSleepForEvents( );
        }
    }

    /*
     * When this loop terminates, destroy the display, state and structure
     * of a freeglut session, so that another glutInit() call can happen
     *
     * Save the "ActionOnWindowClose" because "fgDeinitialize" resets it.
     */
    action = fgState.ActionOnWindowClose;
    fgDeinitialize( );
    if( action == GLUT_ACTION_EXIT )
        exit( 0 );
}

/*
 * Leaves the freeglut processing loop.
 */
void FGAPIENTRY glutLeaveMainLoop( void )
{
    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutLeaveMainLoop" );
    fgState.ExecState = GLUT_EXEC_STATE_STOP ;
}


#if TARGET_HOST_WIN32 || TARGET_HOST_WINCE
/*
 * Determine a GLUT modifer mask based on MS-WINDOWS system info.
 */
static int fghGetWin32Modifiers (void)
{
    return
        ( ( ( GetKeyState( VK_LSHIFT   ) < 0 ) ||
            ( GetKeyState( VK_RSHIFT   ) < 0 )) ? GLUT_ACTIVE_SHIFT : 0 ) |
        ( ( ( GetKeyState( VK_LCONTROL ) < 0 ) ||
            ( GetKeyState( VK_RCONTROL ) < 0 )) ? GLUT_ACTIVE_CTRL  : 0 ) |
        ( ( ( GetKeyState( VK_LMENU    ) < 0 ) ||
            ( GetKeyState( VK_RMENU    ) < 0 )) ? GLUT_ACTIVE_ALT   : 0 );
}

/*
 * The window procedure for handling Win32 events
 */
LRESULT CALLBACK fgWindowProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                               LPARAM lParam )
{
    SFG_Window* window;
    PAINTSTRUCT ps;
    LRESULT lRet = 1;

    FREEGLUT_INTERNAL_ERROR_EXIT_IF_NOT_INITIALISED ( "Event Handler" ) ;

    window = fgWindowByHandle( hWnd );

    if ( ( window == NULL ) && ( uMsg != WM_CREATE ) )
      return DefWindowProc( hWnd, uMsg, wParam, lParam );

    /* printf ( "Window %3d message <%04x> %12d %12d\n", window?window->ID:0,
             uMsg, wParam, lParam ); */
    switch( uMsg )
    {
    case WM_CREATE:
        /* The window structure is passed as the creation structure paramter... */
        window = (SFG_Window *) (((LPCREATESTRUCT) lParam)->lpCreateParams);
        FREEGLUT_INTERNAL_ERROR_EXIT ( ( window != NULL ), "Cannot create window",
                                       "fgWindowProc" );

        window->Window.Handle = hWnd;
        window->Window.Device = GetDC( hWnd );
        if( window->IsMenu )
        {
            unsigned int current_DisplayMode = fgState.DisplayMode;
            fgState.DisplayMode = GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH;
#if !TARGET_HOST_WINCE
            fgSetupPixelFormat( window, GL_FALSE, PFD_MAIN_PLANE );
#endif
            fgState.DisplayMode = current_DisplayMode;

            if( fgStructure.MenuContext )
                wglMakeCurrent( window->Window.Device,
                                fgStructure.MenuContext->Context
                );
            else
            {
                fgStructure.MenuContext =
                    (SFG_MenuContext *)malloc( sizeof(SFG_MenuContext) );
                fgStructure.MenuContext->Context =
                    wglCreateContext( window->Window.Device );
            }

            /* window->Window.Context = wglGetCurrentContext ();   */
            window->Window.Context = wglCreateContext( window->Window.Device );
        }
        else
        {
#if !TARGET_HOST_WINCE
            fgSetupPixelFormat( window, GL_FALSE, PFD_MAIN_PLANE );
#endif

            if( ! fgState.UseCurrentContext )
                window->Window.Context =
                    wglCreateContext( window->Window.Device );
            else
            {
                window->Window.Context = wglGetCurrentContext( );
                if( ! window->Window.Context )
                    window->Window.Context =
                        wglCreateContext( window->Window.Device );
            }
        }

        window->State.NeedToResize = GL_TRUE;
        window->State.Width  = fgState.Size.X;
        window->State.Height = fgState.Size.Y;

        ReleaseDC( window->Window.Handle, window->Window.Device );

#if TARGET_HOST_WINCE
        /* Take over button handling */
        {
            HINSTANCE dxDllLib=LoadLibrary(_T("gx.dll"));
            if (dxDllLib)
            {
                GXGetDefaultKeys_=(GXGETDEFAULTKEYS)GetProcAddress(dxDllLib, _T("?GXGetDefaultKeys@@YA?AUGXKeyList@@H@Z"));
                GXOpenInput_=(GXOPENINPUT)GetProcAddress(dxDllLib, _T("?GXOpenInput@@YAHXZ"));
            }

            if(GXOpenInput_)
                (*GXOpenInput_)();
            if(GXGetDefaultKeys_)
                gxKeyList = (*GXGetDefaultKeys_)(GX_LANDSCAPEKEYS);
        }

#endif /* TARGET_HOST_WINCE */
        break;

    case WM_SIZE:
        /*
         * If the window is visible, then it is the user manually resizing it.
         * If it is not, then it is the system sending us a dummy resize with
         * zero dimensions on a "glutIconifyWindow" call.
         */
        if( window->State.Visible )
        {
            window->State.NeedToResize = GL_TRUE;
#if TARGET_HOST_WINCE
            window->State.Width  = HIWORD(lParam);
            window->State.Height = LOWORD(lParam);
#else
            window->State.Width  = LOWORD(lParam);
            window->State.Height = HIWORD(lParam);
#endif /* TARGET_HOST_WINCE */
        }

        break;
#if 0
    case WM_SETFOCUS:
/*        printf("WM_SETFOCUS: %p\n", window ); */
        lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        break;

    case WM_ACTIVATE:
        if (LOWORD(wParam) != WA_INACTIVE)
        {
/*            printf("WM_ACTIVATE: fgSetCursor( %p, %d)\n", window,
                   window->State.Cursor ); */
            fgSetCursor( window, window->State.Cursor );
        }

        lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        break;
#endif

    case WM_SETCURSOR:
/*      printf ( "Cursor event %x %x %x %x\n", window, window->State.Cursor, lParam, wParam ) ; */
        if( LOWORD( lParam ) == HTCLIENT )
            fgSetCursor ( window, window->State.Cursor ) ;
        else
            lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        break;

    case WM_SHOWWINDOW:
        window->State.Visible = GL_TRUE;
        window->State.Redisplay = GL_TRUE;
        break;

    case WM_PAINT:
        /* Turn on the visibility in case it was turned off somehow */
        window->State.Visible = GL_TRUE;
        BeginPaint( hWnd, &ps );
        fghRedrawWindow( window );
        EndPaint( hWnd, &ps );
        break;

    case WM_CLOSE:
        fgDestroyWindow ( window );
        if ( fgState.ActionOnWindowClose != GLUT_ACTION_CONTINUE_EXECUTION )
            PostQuitMessage(0);
        break;

    case WM_DESTROY:
        /*
         * The window already got destroyed, so don't bother with it.
         */
        return 0;

    /* XXX For a future patch:  we need a mouse entry event.  Unfortunately Windows
     * XXX doesn't give us one, so we will probably need a "MouseInWindow" flag in
     * XXX the SFG_Window structure.  Set it to true to begin with and then have the
     * XXX WM_MOUSELEAVE code set it to false.  Then when we get a WM_MOUSEMOVE event,
     * XXX if the flag is false we invoke the Entry callback and set the flag to true.
     */
    case 0x02a2:  /* This is the message we get when the mouse is leaving the window */
        if( window->IsMenu &&
            window->ActiveMenu && window->ActiveMenu->IsActive )
            fgUpdateMenuHighlight( window->ActiveMenu );

        INVOKE_WCB( *window, Entry, ( GLUT_LEFT ) );
        break ;

    case WM_MOUSEMOVE:
    {
#if TARGET_HOST_WINCE
        window->State.MouseX = 320-HIWORD( lParam );
        window->State.MouseY = LOWORD( lParam );
#else
        window->State.MouseX = LOWORD( lParam );
        window->State.MouseY = HIWORD( lParam );
#endif /* TARGET_HOST_WINCE */
        /* Restrict to [-32768, 32767] to match X11 behaviour       */
        /* See comment in "freeglut_developer" mailing list 10/4/04 */
        if ( window->State.MouseX > 32767 ) window->State.MouseX -= 65536;
        if ( window->State.MouseY > 32767 ) window->State.MouseY -= 65536;

        if ( window->ActiveMenu )
        {
            fgUpdateMenuHighlight( window->ActiveMenu );
            break;
        }

        fgState.Modifiers = fghGetWin32Modifiers( );

        if( ( wParam & MK_LBUTTON ) ||
            ( wParam & MK_MBUTTON ) ||
            ( wParam & MK_RBUTTON ) )
            INVOKE_WCB( *window, Motion, ( window->State.MouseX,
                                           window->State.MouseY ) );
        else
            INVOKE_WCB( *window, Passive, ( window->State.MouseX,
                                            window->State.MouseY ) );

        fgState.Modifiers = 0xffffffff;
    }
    break;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
        GLboolean pressed = GL_TRUE;
        int button;

#if TARGET_HOST_WINCE
        window->State.MouseX = 320-HIWORD( lParam );
        window->State.MouseY = LOWORD( lParam );
#else
        window->State.MouseX = LOWORD( lParam );
        window->State.MouseY = HIWORD( lParam );
#endif /* TARGET_HOST_WINCE */

        /* Restrict to [-32768, 32767] to match X11 behaviour       */
        /* See comment in "freeglut_developer" mailing list 10/4/04 */
        if ( window->State.MouseX > 32767 ) window->State.MouseX -= 65536;
        if ( window->State.MouseY > 32767 ) window->State.MouseY -= 65536;

        switch( uMsg )
        {
        case WM_LBUTTONDOWN:
            pressed = GL_TRUE;
            button = GLUT_LEFT_BUTTON;
            break;
        case WM_MBUTTONDOWN:
            pressed = GL_TRUE;
            button = GLUT_MIDDLE_BUTTON;
            break;
        case WM_RBUTTONDOWN:
            pressed = GL_TRUE;
            button = GLUT_RIGHT_BUTTON;
            break;
        case WM_LBUTTONUP:
            pressed = GL_FALSE;
            button = GLUT_LEFT_BUTTON;
            break;
        case WM_MBUTTONUP:
            pressed = GL_FALSE;
            button = GLUT_MIDDLE_BUTTON;
            break;
        case WM_RBUTTONUP:
            pressed = GL_FALSE;
            button = GLUT_RIGHT_BUTTON;
            break;
        default:
            pressed = GL_FALSE;
            button = -1;
            break;
        }

#if !TARGET_HOST_WINCE
        if( GetSystemMetrics( SM_SWAPBUTTON ) )
        {
            if( button == GLUT_LEFT_BUTTON )
                button = GLUT_RIGHT_BUTTON;
            else
                if( button == GLUT_RIGHT_BUTTON )
                    button = GLUT_LEFT_BUTTON;
        }
#endif /* !TARGET_HOST_WINCE */

        if( button == -1 )
            return DefWindowProc( hWnd, uMsg, lParam, wParam );

        /*
         * Do not execute the application's mouse callback if a menu
         * is hooked to this button.  In that case an appropriate
         * private call should be generated.
         */
        if( fgCheckActiveMenu( window, button, pressed,
                               window->State.MouseX, window->State.MouseY ) )
            break;

        /* Set capture so that the window captures all the mouse messages */
        /*
         * XXX - Multiple button support:  Under X11, the mouse is not released
         * XXX - from the window until all buttons have been released, even if the
         * XXX - user presses a button in another window.  This will take more
         * XXX - code changes than I am up to at the moment (10/5/04).  The present
         * XXX - is a 90 percent solution.
         */
        if ( pressed == GL_TRUE )
          SetCapture ( window->Window.Handle ) ;
        else
          ReleaseCapture () ;

        if( ! FETCH_WCB( *window, Mouse ) )
            break;

        fgSetWindow( window );
        fgState.Modifiers = fghGetWin32Modifiers( );

        INVOKE_WCB(
            *window, Mouse,
            ( button,
              pressed ? GLUT_DOWN : GLUT_UP,
              window->State.MouseX,
              window->State.MouseY
            )
        );

        fgState.Modifiers = 0xffffffff;
    }
    break;

    case 0x020a:
        /* Should be WM_MOUSEWHEEL but my compiler doesn't recognize it */
    {
        /*
         * XXX THIS IS SPECULATIVE -- John Fay, 10/2/03
         * XXX Should use WHEEL_DELTA instead of 120
         */
        int wheel_number = LOWORD( wParam );
        short ticks = ( short )HIWORD( wParam ) / 120;
        int direction = 1;

        if( ticks < 0 )
        {
            direction = -1;
            ticks = -ticks;
        }

        /*
         * The mouse cursor has moved. Remember the new mouse cursor's position
         */
        /*        window->State.MouseX = LOWORD( lParam ); */
        /* Need to adjust by window position, */
        /*        window->State.MouseY = HIWORD( lParam ); */
        /* change "lParam" to other parameter */

        if( ! FETCH_WCB( *window, MouseWheel ) &&
            ! FETCH_WCB( *window, Mouse ) )
            break;

        fgSetWindow( window );
        fgState.Modifiers = fghGetWin32Modifiers( );

        while( ticks-- )
            if( FETCH_WCB( *window, MouseWheel ) )
                INVOKE_WCB( *window, MouseWheel,
                            ( wheel_number,
                              direction,
                              window->State.MouseX,
                              window->State.MouseY
                            )
                );
            else  /* No mouse wheel, call the mouse button callback twice */
            {
                /*
                 * Map wheel zero to button 3 and 4; +1 to 3, -1 to 4
                 *  "    "   one                     +1 to 5, -1 to 6, ...
                 *
                 * XXX The below assumes that you have no more than 3 mouse
                 * XXX buttons.  Sorry.
                 */
                int button = wheel_number * 2 + 3;
                if( direction < 0 )
                    ++button;
                INVOKE_WCB( *window, Mouse,
                            ( button, GLUT_DOWN,
                              window->State.MouseX, window->State.MouseY )
                );
                INVOKE_WCB( *window, Mouse,
                            ( button, GLUT_UP,
                              window->State.MouseX, window->State.MouseY )
                );
            }

        fgState.Modifiers = 0xffffffff;
    }
    break ;

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    {
        int keypress = -1;
        POINT mouse_pos ;

        if( ( fgState.KeyRepeat==GLUT_KEY_REPEAT_OFF || window->State.IgnoreKeyRepeat==GL_TRUE ) && (HIWORD(lParam) & KF_REPEAT) )
            break;

        /*
         * Remember the current modifiers state. This is done here in order
         * to make sure the VK_DELETE keyboard callback is executed properly.
         */
        fgState.Modifiers = fghGetWin32Modifiers( );

        GetCursorPos( &mouse_pos );
        ScreenToClient( window->Window.Handle, &mouse_pos );

        window->State.MouseX = mouse_pos.x;
        window->State.MouseY = mouse_pos.y;

        /* Convert the Win32 keystroke codes to GLUTtish way */
#       define KEY(a,b) case a: keypress = b; break;

        switch( wParam )
        {
            KEY( VK_F1,     GLUT_KEY_F1        );
            KEY( VK_F2,     GLUT_KEY_F2        );
            KEY( VK_F3,     GLUT_KEY_F3        );
            KEY( VK_F4,     GLUT_KEY_F4        );
            KEY( VK_F5,     GLUT_KEY_F5        );
            KEY( VK_F6,     GLUT_KEY_F6        );
            KEY( VK_F7,     GLUT_KEY_F7        );
            KEY( VK_F8,     GLUT_KEY_F8        );
            KEY( VK_F9,     GLUT_KEY_F9        );
            KEY( VK_F10,    GLUT_KEY_F10       );
            KEY( VK_F11,    GLUT_KEY_F11       );
            KEY( VK_F12,    GLUT_KEY_F12       );
            KEY( VK_PRIOR,  GLUT_KEY_PAGE_UP   );
            KEY( VK_NEXT,   GLUT_KEY_PAGE_DOWN );
            KEY( VK_HOME,   GLUT_KEY_HOME      );
            KEY( VK_END,    GLUT_KEY_END       );
            KEY( VK_LEFT,   GLUT_KEY_LEFT      );
            KEY( VK_UP,     GLUT_KEY_UP        );
            KEY( VK_RIGHT,  GLUT_KEY_RIGHT     );
            KEY( VK_DOWN,   GLUT_KEY_DOWN      );
            KEY( VK_INSERT, GLUT_KEY_INSERT    );

        case VK_DELETE:
            /* The delete key should be treated as an ASCII keypress: */
            INVOKE_WCB( *window, Keyboard,
                        ( 127, window->State.MouseX, window->State.MouseY )
            );
        }

#if TARGET_HOST_WINCE
        if(!(lParam & 0x40000000)) /* Prevent auto-repeat */
        {
            if(wParam==(unsigned)gxKeyList.vkRight)
                keypress = GLUT_KEY_RIGHT;
            else if(wParam==(unsigned)gxKeyList.vkLeft)
                keypress = GLUT_KEY_LEFT;
            else if(wParam==(unsigned)gxKeyList.vkUp)
                keypress = GLUT_KEY_UP;
            else if(wParam==(unsigned)gxKeyList.vkDown)
                keypress = GLUT_KEY_DOWN;
            else if(wParam==(unsigned)gxKeyList.vkA)
                keypress = GLUT_KEY_F1;
            else if(wParam==(unsigned)gxKeyList.vkB)
                keypress = GLUT_KEY_F2;
            else if(wParam==(unsigned)gxKeyList.vkC)
                keypress = GLUT_KEY_F3;
            else if(wParam==(unsigned)gxKeyList.vkStart)
                keypress = GLUT_KEY_F4;
        }
#endif

        if( keypress != -1 )
            INVOKE_WCB( *window, Special,
                        ( keypress,
                          window->State.MouseX, window->State.MouseY )
            );

        fgState.Modifiers = 0xffffffff;
    }
    break;

    case WM_SYSKEYUP:
    case WM_KEYUP:
    {
        int keypress = -1;
        POINT mouse_pos;

        /*
         * Remember the current modifiers state. This is done here in order
         * to make sure the VK_DELETE keyboard callback is executed properly.
         */
        fgState.Modifiers = fghGetWin32Modifiers( );

        GetCursorPos( &mouse_pos );
        ScreenToClient( window->Window.Handle, &mouse_pos );

        window->State.MouseX = mouse_pos.x;
        window->State.MouseY = mouse_pos.y;

        /*
         * Convert the Win32 keystroke codes to GLUTtish way.
         * "KEY(a,b)" was defined under "WM_KEYDOWN"
         */

        switch( wParam )
        {
            KEY( VK_F1,     GLUT_KEY_F1        );
            KEY( VK_F2,     GLUT_KEY_F2        );
            KEY( VK_F3,     GLUT_KEY_F3        );
            KEY( VK_F4,     GLUT_KEY_F4        );
            KEY( VK_F5,     GLUT_KEY_F5        );
            KEY( VK_F6,     GLUT_KEY_F6        );
            KEY( VK_F7,     GLUT_KEY_F7        );
            KEY( VK_F8,     GLUT_KEY_F8        );
            KEY( VK_F9,     GLUT_KEY_F9        );
            KEY( VK_F10,    GLUT_KEY_F10       );
            KEY( VK_F11,    GLUT_KEY_F11       );
            KEY( VK_F12,    GLUT_KEY_F12       );
            KEY( VK_PRIOR,  GLUT_KEY_PAGE_UP   );
            KEY( VK_NEXT,   GLUT_KEY_PAGE_DOWN );
            KEY( VK_HOME,   GLUT_KEY_HOME      );
            KEY( VK_END,    GLUT_KEY_END       );
            KEY( VK_LEFT,   GLUT_KEY_LEFT      );
            KEY( VK_UP,     GLUT_KEY_UP        );
            KEY( VK_RIGHT,  GLUT_KEY_RIGHT     );
            KEY( VK_DOWN,   GLUT_KEY_DOWN      );
            KEY( VK_INSERT, GLUT_KEY_INSERT    );

          case VK_DELETE:
              /* The delete key should be treated as an ASCII keypress: */
              INVOKE_WCB( *window, KeyboardUp,
                          ( 127, window->State.MouseX, window->State.MouseY )
              );
              break;

        default:
        {
#if !TARGET_HOST_WINCE
            BYTE state[ 256 ];
            WORD code[ 2 ];

            GetKeyboardState( state );

            if( ToAscii( (UINT)wParam, 0, state, code, 0 ) == 1 )
                wParam=code[ 0 ];

            INVOKE_WCB( *window, KeyboardUp,
                        ( (char)wParam,
                          window->State.MouseX, window->State.MouseY )
            );
#endif /* !TARGET_HOST_WINCE */
        }
        }

        if( keypress != -1 )
            INVOKE_WCB( *window, SpecialUp,
                        ( keypress,
                          window->State.MouseX, window->State.MouseY )
            );

        fgState.Modifiers = 0xffffffff;
    }
    break;

    case WM_SYSCHAR:
    case WM_CHAR:
    {
      if( (fgState.KeyRepeat==GLUT_KEY_REPEAT_OFF || window->State.IgnoreKeyRepeat==GL_TRUE) && (HIWORD(lParam) & KF_REPEAT) )
            break;

        fgState.Modifiers = fghGetWin32Modifiers( );
        INVOKE_WCB( *window, Keyboard,
                    ( (char)wParam,
                      window->State.MouseX, window->State.MouseY )
        );
        fgState.Modifiers = 0xffffffff;
    }
    break;

    case WM_CAPTURECHANGED:
        /* User has finished resizing the window, force a redraw */
        INVOKE_WCB( *window, Display, ( ) );

        /*lRet = DefWindowProc( hWnd, uMsg, wParam, lParam ); */
        break;

        /* Other messages that I have seen and which are not handled already */
    case WM_SETTEXT:  /* 0x000c */
        lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        /* Pass it on to "DefWindowProc" to set the window text */
        break;

    case WM_GETTEXT:  /* 0x000d */
        /* Ideally we would copy the title of the window into "lParam" */
        /* strncpy ( (char *)lParam, "Window Title", wParam );
           lRet = ( wParam > 12 ) ? 12 : wParam;  */
        /* the number of characters copied */
        lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        break;

    case WM_GETTEXTLENGTH:  /* 0x000e */
        /* Ideally we would get the length of the title of the window */
        lRet = 12;
        /* the number of characters in "Window Title\0" (see above) */
        break;

    case WM_ERASEBKGND:  /* 0x0014 */
        lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        break;

#if !TARGET_HOST_WINCE
    case WM_SYNCPAINT:  /* 0x0088 */
        /* Another window has moved, need to update this one */
        window->State.Redisplay = GL_TRUE;
        lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        /* Help screen says this message must be passed to "DefWindowProc" */
        break;

    case WM_NCPAINT:  /* 0x0085 */
      /* Need to update the border of this window */
        lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        /* Pass it on to "DefWindowProc" to repaint a standard border */
        break;

    case WM_SYSCOMMAND :  /* 0x0112 */
        {
          /*
           * We have received a system command message.  Try to act on it.
           * The commands are passed in through the "wParam" parameter:
           * The least significant digit seems to be which edge of the window
           * is being used for a resize event:
           *     4  3  5
           *     1     2
           *     7  6  8
           * Congratulations and thanks to Richard Rauch for figuring this out..
           */
            switch ( wParam & 0xfff0 )
            {
            case SC_SIZE       :
                break ;

            case SC_MOVE       :
                break ;

            case SC_MINIMIZE   :
                /* User has clicked on the "-" to minimize the window */
                /* Turn off the visibility */
                window->State.Visible = GL_FALSE ;

                break ;

            case SC_MAXIMIZE   :
                break ;

            case SC_NEXTWINDOW :
                break ;

            case SC_PREVWINDOW :
                break ;

            case SC_CLOSE      :
                /* Followed very closely by a WM_CLOSE message */
                break ;

            case SC_VSCROLL    :
                break ;

            case SC_HSCROLL    :
                break ;

            case SC_MOUSEMENU  :
                break ;

            case SC_KEYMENU    :
                break ;

            case SC_ARRANGE    :
                break ;

            case SC_RESTORE    :
                break ;

            case SC_TASKLIST   :
                break ;

            case SC_SCREENSAVE :
                break ;

            case SC_HOTKEY     :
                break ;

            default:
#if _DEBUG
                fgWarning( "Unknown wParam type 0x%x", wParam );
#endif
                break;
            }
        }
#endif /* !TARGET_HOST_WINCE */

        /* We need to pass the message on to the operating system as well */
        lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        break;

    default:
        /* Handle unhandled messages */
        lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        break;
    }

    return lRet;
}
#endif

/*** END OF FILE ***/
