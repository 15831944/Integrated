/*****************************************************************************
**
** Integrated.cpp
**
** Description:
**     Contains Unigraphics entry points for the application.
**
*****************************************************************************/

/* Include files */
#if ! defined ( __hp9000s800 ) && ! defined ( __sgi ) && ! defined ( __sun )
#   include <strstream>
#   include <iostream>
    using std::ostrstream;
    using std::endl;    
    using std::ends;
    using std::cerr;
#else
#   include <strstream.h>
#   include <iostream.h>
#endif


#include "ManiDlg.hpp"


#include <uf.h>
#include <ug_session.hxx>
#include <ug_exception.hxx>
#include <uf_ui.h>
#include <uf_exit.h>
#include <ug_info_window.hxx>


static void processException( const UgException &exception );

class ManiDlg;
ManiDlg *maindlg;

extern DllExport void ufusr( char *parm, int *returnCode, int rlen )
{
    /* Initialize the API environment */
    UgSession session( true );
	
    //try
	if(UF_initialize()==0)
    {    
		

		/* TODO: Add your application code here */
		maindlg= new ManiDlg();
		maindlg->Show();

		
		
		int if_ok=maindlg->if_ok;
		if(if_ok==0) 
		{ 
			delete maindlg;
			return;
		}
		else
		{		
			delete maindlg;
			UF_terminate();
		}
    }
}

extern int ufusr_ask_unload( void )
{
    	//return( UF_UNLOAD_UG_TERMINATE );//NX关闭后才卸载 发布
	return(UF_UNLOAD_IMMEDIATELY);       //程序退出就卸载 开发调试
}

static void processException( const UgException &exception )
{
    /* Construct a buffer to hold the text. */
    ostrstream error_message;

    /* Initialize the buffer with the required text. */
    error_message << endl
                  << "Error:" << endl
                  << ( exception.askErrorText() ).c_str()
                  << endl << endl << ends;

    /* Open the UgInfoWindow */
    UgInfoWindow::open ( );

    /* Write the message to the UgInfoWindow.  The str method */
    /* freezes the buffer, so it must be unfrozen afterwards. */
    UgInfoWindow::write( error_message.str() );

    /* Write the message to standard error */
    cerr << error_message.str();
    error_message.rdbuf()->freeze( 0 );
}
