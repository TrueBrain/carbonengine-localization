#include "stdafx.h"
#include "WrapPointList.h"

#include "localization.h"


BLUE_DEFINE( WrapPointList );

// -------------------------------------------------------------
// Description:
//   Return the list of points where text can be wrapped.
// Arguments:
//   self - Pointer to our pyobject, allows us to retrieve "this"
//   args - unused
// Return value:
//   A list containing all valid wrap points, e.g. the offsets in the string *before* which a wrap point may occur
// See also:
//   Microsoft's Uniscribe API, specifically: 
//     http://msdn.microsoft.com/en-us/library/dd368556.aspx
//     http://msdn.microsoft.com/en-us/library/dd319118.aspx
// -------------------------------------------------------------
PyObject* PyGetLinebreakPoints( PyObject* self, PyObject* args )
{
	CCP_STATS_ZONE( __FUNCTION__ );

	WrapPointList* wrapPointList = BluePythonCast<WrapPointList*>( self );

	PyObject* retVal = PyList_New( 0 );
	if ( ! retVal )
	{
		return NULL;
	}
#ifdef _WIN32
	BYTE isWhitespace = 0, prevWhitespace = 0; // Treat the character after any whitespace as a linebreak point.
	// There's no need to break at the very first character, we're in trouble by then anyway
	for ( size_t i = 0; i < wrapPointList->Size(); ++i )
	{
		isWhitespace = (*wrapPointList)[i].fWhiteSpace;
		if ( i > 0 && ( (*wrapPointList)[i].fSoftBreak || ( prevWhitespace && ! isWhitespace ) ) )
		{
			PyObject* val = PyInt_FromLong( (long) i );
			PyList_Append( retVal, val );
			Py_DECREF( val );
		}
		prevWhitespace = isWhitespace;
	}
#elif defined( __APPLE__ )
    AutoReleaseCF<CFStringTokenizerRef> tokenizer = CFStringTokenizerCreate(
        kCFAllocatorDefault,
        wrapPointList->GetCoreString(),
        CFRangeMake( 0, CFStringGetLength( wrapPointList->GetCoreString() ) ),
        kCFStringTokenizerUnitLineBreak,
        wrapPointList->GetCoreLocale());
    CFStringTokenizerTokenType tokenType = kCFStringTokenizerTokenNone;
    while( kCFStringTokenizerTokenNone != ( tokenType = CFStringTokenizerAdvanceToNextToken( tokenizer ) ) )
    {
        CFRange tokenRange = CFStringTokenizerGetCurrentTokenRange( tokenizer );
        if( tokenRange.location > 0 )
        {
            PyObject* val = PyInt_FromLong( (long) tokenRange.location );
            PyList_Append( retVal, val );
            Py_DECREF( val );
        }
    }
#endif
	return retVal;
}

// -------------------------------------------------------------
// Description:
//   Returns a list of wordstop points (where ctrl-leftArrow and ctrl-rightArrow should stop).
// Arguments:
//   self - Pointer to our pyobject, allows us to retrieve "this"
//   args - unused
// Return value:
//   A list containing all valid wrap points, e.g. the offsets in the string *before* which a wrap point may occur
// See also:
//   Microsoft's Uniscribe API, specifically: 
//     http://msdn.microsoft.com/en-us/library/dd368556.aspx
//     http://msdn.microsoft.com/en-us/library/dd319118.aspx
// -------------------------------------------------------------
PyObject* PyGetWordbreakPoints( PyObject* self, PyObject* args )
{
#ifdef _WIN32
	CCP_STATS_ZONE( __FUNCTION__ );

	WrapPointList* wrapPointList = BluePythonCast<WrapPointList*>( self );

	PyObject* retVal = PyList_New( 0 );
	if ( ! retVal )
	{
		return NULL;
	}
	BYTE isWhitespace = 0, prevWhitespace = 0; // Treat the character after any whitespace as a linebreak point.
	// There's no need to break at the very first character, we're in trouble by then anyway
	for ( size_t i = 0; i < wrapPointList->Size(); ++i )
	{
		isWhitespace = (*wrapPointList)[i].fWhiteSpace;
		if ( (*wrapPointList)[i].fWordStop || ( prevWhitespace && ! isWhitespace ) )
		{
			PyObject* val = PyInt_FromLong( (long) i );
			PyList_Append( retVal, val );
			Py_DECREF( val );
		}
		prevWhitespace = isWhitespace;
	}
    return retVal;
#else
    PyErr_SetString( PyExc_NotImplementedError, "This function is not implemented for this platform" );
    return nullptr;
#endif
}

PyObject* Py__init__(PyObject *self, PyObject *args)
{
	WrapPointList* pThis = BluePythonCast<WrapPointList*>( self );
	Py_UNICODE *textStr = 0;
    int textLength = 0;
	const char* langStr = 0;
	if (!PyArg_ParseTuple(args, "u#s", &textStr, &textLength, &langStr))
	{
		PyErr_SetString( PyExc_ValueError, 
			"Error in constructor arguments:\n"
			"text: Unicode string, the text you wish to analyze.\n" 
			"languageCode: The language to use for analysis, such as en-us or ja." );
		PyOS->PyFlushError( "WrapPointList::Py__init__" );
		return NULL;
	}
#ifdef _WIN32
    pThis->m_wrapPointListCount = textLength;

	if ( pThis->m_wrapPointListCount > (size_t) INT_MAX )
	{
		PyErr_SetString( PyExc_ValueError, "The passed in string is too long." );
		PyOS->PyFlushError( "WrapPointList::Initialize" );
		return NULL;
	};

	SCRIPT_CONTROL control;
	memset( &control, 0, sizeof( SCRIPT_CONTROL ) );
	control.uDefaultLanguage = CodeToLanguageID( langStr );

	if ( ! pThis->TextAnalyze( textStr, (int)pThis->m_wrapPointListCount, &control, NULL ) )
	{
		PyErr_SetString( PyExc_SystemError, "Text analysis failed, cannot determine wrap points.");
		PyOS->PyFlushError( "WrapPointList::Initialize" );
		return NULL;
	}

	pThis->m_wrapPointList = CCP_NEW( "EveLocalization/WrapPoints/WrapPointList" ) SCRIPT_LOGATTR[pThis->m_wrapPointListCount];

	// Generate the word-break information for each item-run
	for ( int i = 0; i < pThis->m_itemRunCount; i++ )
	{
		ItemRun *itemRun = &pThis->m_itemRunList[i];
		ScriptBreak( textStr + itemRun->charPos, itemRun->len, &itemRun->analysis, pThis->m_wrapPointList + itemRun->charPos );
	}
#elif defined( __APPLE__ )
    pThis->m_coreString = ToStringRef( reinterpret_cast<const wchar_t*>( textStr ), size_t( textLength ) );
    if ( ! pThis->m_coreString )
    {
        PyErr_SetString( PyExc_SystemError, "Text analysis failed, cannot convert to string." );
        return nullptr;
    }
    AutoReleaseCF<CFStringRef> name = ToStringRef( LanguageIDToCode( CodeToLanguageID( langStr ) ) );
    pThis->m_coreLocale = CFLocaleCreate( nullptr, name );
    if ( ! pThis->m_coreLocale )
    {
        PyErr_SetString( PyExc_SystemError, "Text analysis failed, cannot create locale." );
        return nullptr;
    }
#endif
	Py_RETURN_NONE;
};

// DOM-IGNORE-BEGIN
const Be::ClassInfo* WrapPointList::ExposeToBlue()
{
	EXPOSURE_BEGIN( WrapPointList, "A list of potential line break (wrap) points" )
		MAP_INTERFACE( WrapPointList )
		MAP_METHOD
		(
			"GetLinebreakPoints",
			PyGetLinebreakPoints,
			"Returns a list of points at which we can wrap text."
		)
		MAP_METHOD
		(
			"GetWordbreakPoints",
			PyGetWordbreakPoints,
			"Returns a list of wordstop points (where ctrl-leftArrow and ctrl-rightArrow should stop)."
		)
		MAP_METHOD
		( 
			"__init__",
			Py__init__,
			"WrapPointList constructor\nArguments:"
			"\ttext: Unicode string, the text you wish to analyze.\n" 
			"\tlanguageCode: The language to use for analysis, such as en-us or ja." 
		)

	EXPOSURE_END()
}
// DOM-IGNORE-END
