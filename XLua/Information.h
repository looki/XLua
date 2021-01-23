// --------------------
// Object information
// --------------------

// Basic object details
#define	ObjectName			_T("XLua")
#define	ObjectAuthor		_T("Justin Aquadro, Lukas Meller")
#define	ObjectCopyright		_T("Copyright © 2009-2021 Justin Aquadro")
#define	ObjectComment		_T("Powerful and extensible embedded Lua support for MMF2 & CTF 2.5(+).")
#define	ObjectURL			_T("http://www.taloncrossing.com/xlua")
#define	ObjectHelp			_T("Help\\XLua.chm")

// If you register your object with Clickteam, change this to the ID you were given
//#define ObjectRegID			REGID_PRIVATE
#define ObjectRegID			-1

// Change N,O,N,E to 4 unique characters (MMF currently still uses this to keep track)
//#define	IDENTIFIER			MAKEID(X,L,U,A)
#define	IDENTIFIER			'XLUA'

// --------------------
// Version information
// --------------------

// PRODUCT_VERSION_TGF or PRODUCT_VERSION_DEVELOPER
#define ForVersion			PRODUCT_VERSION_STANDARD

// Set this to the latest MMF build out when you build the object
#define	MinimumBuild		258
#define MINBUILD			MinimumBuild

// --------------------
// Beta information
// --------------------

// #define BETA
// #define POPUP_ON_DROP
// #define POPUP_ON_EXE
// #define POPUP_ON_BUILD
// #define POPUP_MESSAGE	_T("This is a beta extension- use with caution!")

// --------------------
// Handling priority
// --------------------
// If this extension will handle windows messages, specify the priority
// of the handling procedure, 0 = low and 255 = very high

#define	WINDOWPROC_PRIORITY 120
