#pragma once
#ifndef Localization_H
#define Localization_H

#include <locale>
#include <map>
#include <vector>

// language codes
// This is really a list of all the Windows LCID's we support, see http://msdn.microsoft.com/en-us/goglobal/bb964664
enum LanguageID
{
	LANGUAGEID_ENGLISH_US          = 1033,
	LANGUAGEID_JAPANESE            = 1041,
	LANGUAGEID_KOREAN              = 1042,
	LANGUAGEID_RUSSIAN             = 1049,
	LANGUAGEID_GERMAN              = 1031,
	LANGUAGEID_FRENCH              = 1036,
	LANGUAGEID_ITALIAN             = 1040,
	LANGUAGEID_SPANISH             = 1034,
	LANGUAGEID_CHINESE_SIMPLIFIED  = 2052, // Apparently simplified is the same as People's Republic of China in Windows
	LANGUAGEID_SYSTEM_DEFAULT      = 0x0400, // LOCALE_USER_DEFAULT,
	LANGUAGEID_DEFAULT             = LANGUAGEID_ENGLISH_US,
};

// Indicates special case handling for a token
enum TokenFlag
{
	TOKENFLAG_LINKIFY        = 1<<0, // OBSOLETE
	TOKENFLAG_CAPITALIZE     = 1<<1,
	TOKENFLAG_UPPERCASE      = 1<<2,
	TOKENFLAG_LOWERCASE      = 1<<3,
	TOKENFLAG_TITLECASE      = 1<<4,
	TOKENFLAG_USEGROUPING    = 1<<5,
	TOKENFLAG_CONDITIONAL    = 1<<6,
	TOKENFLAG_LINKINFO       = 1<<7,	// tells us that we need to insert dereferenced hyperlink data into kwargs
	TOKENFLAG_QUANTITY       = 1<<8,  // tells us that we need to insert dereferenced quantity data into kwargs
	TOKENFLAG_DECIMALPLACES  = 1<<9,
	TOKENFLAG_LEADINGZEROES  = 1<<10,
};

typedef unsigned int TokenFlags;

enum VariableType
{
	VARIABLETYPE_CHARACTER        = 0,
	VARIABLETYPE_NPCORGANIZATION,
	VARIABLETYPE_ITEM,
	VARIABLETYPE_LOCATION,
	VARIABLETYPE_CHARACTERLIST,
	VARIABLETYPE_MESSAGE,
	VARIABLETYPE_DATETIME,
	VARIABLETYPE_FORMATTEDTIME,
	VARIABLETYPE_TIMEINTERVAL,
	VARIABLETYPE_NUMERIC,
	VARIABLETYPE_GENERIC,
	VARIABLETYPE_MAX,
};

// IMPORTANT: KeywordArgs is INCREF'ing the PyObject since we only get a borrowed reference from parse.
typedef std::map<std::string, PyObject*> KeywordArgs;
typedef KeywordArgs::iterator            KeywordArgsIt;
typedef KeywordArgs::const_iterator      KeywordArgsCit;
typedef KeywordArgs*                     KeywordArgsPtr;

// Description of a single markup element in a string
struct Token
{
	Token() : kwargs( NULL ) {}

	~Token()
	{
		if ( kwargs )
		{
			for ( KeywordArgsIt it = kwargs->begin(); it != kwargs->end(); ++it )
			{
				Py_XDECREF( it->second );
			}
			kwargs->clear();
			CCP_DELETE kwargs;
		}
	}

	explicit Token( const Token& _other ) : kwargs( NULL )
	{
		*this = _other;
	}

	Token& operator=( const Token& _other )
	{
		if ( this != &_other )
		{
			tagName = _other.tagName;
			variableType = _other.variableType;
			variableName = _other.variableName;
			propertyName = _other.propertyName;
			flags = _other.flags;
			if ( kwargs )
			{
				for ( KeywordArgsIt it = kwargs->begin(); it != kwargs->end(); ++it )
				{
					Py_XDECREF( it->second );
				}
				CCP_DELETE kwargs;
				kwargs = NULL;
			}
			if ( _other.kwargs )
			{
				kwargs = CCP_NEW( "eveLocalization/KeywordArgs" ) KeywordArgs( _other.kwargs->begin(), _other.kwargs->end() );
				// _other is going to dec ref during destruction, alas we need to incref here!
				for ( KeywordArgsIt it = kwargs->begin(); it != kwargs->end(); ++it )
				{
					Py_XINCREF( it->second );
				}
			}
			for ( size_t i = 0; i < 3; ++i )
			{
				conditionalValues[i] = _other.conditionalValues[i];
			}
		}

		return *this;
	}

	std::wstring   tagName;              // the markup this token is for
	VariableType   variableType;         // name of the variable type
	std::string    variableName;         // name of the variable within this token
	std::string    propertyName;         // name of the property this token describes
	TokenFlags     flags;			    // a bunch of flags determining the actual formatting and other stuff
	KeywordArgsPtr kwargs;               // simple key->value mapping; IIRC only 2 items max so two pointers might be better
	std::wstring   conditionalValues[3]; // conditional values -> 0 to 3 unicode strings
};

typedef std::map<std::wstring, Token*> TokenContainer;
typedef TokenContainer::iterator TokenContainerIt;
typedef TokenContainer::const_iterator TokenContainerCit;
typedef TokenContainer* TokenContainerPtr;

typedef std::map<std::string, std::wstring> MetaData;
typedef MetaData::iterator MetaDataIt;
typedef MetaData::const_iterator MetaDataCit;
typedef MetaData* MetaDataPtr;

struct MessageData
{
	MessageData() : tokens( NULL ),  metaData( NULL ) {}

	~MessageData() 
	{
		if ( tokens )
		{
			for( auto it = tokens->begin(); it != tokens->end(); ++it )
			{
				CCP_DELETE it->second;
			}
			CCP_DELETE tokens;
		}

		if ( metaData )
		{
			CCP_DELETE metaData;
		}
	}

	explicit MessageData( const MessageData& _other ) : tokens( NULL ),  metaData( NULL )
	{
		*this = _other;
	}

	MessageData& operator=( const MessageData& _other )
	{
		if ( this != &_other )
		{
			text = _other.text;

			if ( tokens )
			{
				CCP_DELETE tokens;
				tokens = NULL;
			}

			if ( _other.tokens )
			{
				tokens = CCP_NEW( "eveLocalizaton/Tokens" ) TokenContainer( _other.tokens->begin(), _other.tokens->end() );
			}

			if ( metaData )
			{
				CCP_DELETE metaData;
				metaData = NULL;
			}

			if ( _other.metaData )
			{
				metaData = CCP_NEW( "eveLocalization/MetaData" ) MetaData( _other.metaData->begin(), _other.metaData->end() );
			}
		}

		return *this;
	}

	std::wstring       text;
	TokenContainerPtr  tokens;
	MetaDataPtr        metaData;
};

typedef uint64_t MessageID;
class MessageMap : public std::map<MessageID, MessageData*>
{
public:
	~MessageMap()
	{
		for( auto it = begin(); it != end(); ++it )
		{
			CCP_DELETE it->second;
		}
	}
};
typedef MessageMap::iterator MessageMapIt;
typedef MessageMap::const_iterator MessageMapCit;
typedef std::map<LanguageID, MessageMap> LanguageData;
typedef LanguageData::iterator LanguageDataIt;
typedef LanguageData::const_iterator LanguageDataCit;

typedef std::map<VariableType, PyObject*> PropertyHandlerMap;
typedef PropertyHandlerMap::iterator PropertyHandlerMapIt;
typedef PropertyHandlerMap::const_iterator PropertyHandlerMapCit;

// pointer to the quantity category function we should use to determine the conditional value index
typedef size_t (*QuantityCategoryFuncPtr)( int64_t );

// small helper to get array size
#define	DIM(a)	( sizeof( a ) / sizeof( a[0] ) )

#define STACK_BUFFER_SIZE_SMALL 32
#define STACK_BUFFER_SIZE_LARGE 1024

// Information required for a specific language
// This includes it's language code, as well as the quantity function it should use
struct Language
{
	Language( LanguageID langID = LANGUAGEID_DEFAULT );

	explicit Language( const Language& _other )
	{
		*this = _other;
	}

	Language& operator=( const Language& _other )
	{
		if ( this != &_other )
		{
			id = _other.id;
			quantityCategoryFunc = _other.quantityCategoryFunc;
			decimalSep = _other.decimalSep;
			thousandSep = _other.thousandSep;
			data.clear();
			data.insert( _other.data.begin(), _other.data.end() );
			locale = _other.locale;
		}
		return *this;
	}

	bool SetNumberSeparators(LanguageID langID);

	LanguageID               id; // same as the primary language identifier
	QuantityCategoryFuncPtr  quantityCategoryFunc;
	wchar_t                  thousandSep;
	wchar_t                  decimalSep;
	MessageMap               data;
	std::locale				 locale;
};

typedef std::map<LanguageID, Language*> LanguageMap;
typedef LanguageMap::iterator LanguageMapIt;
typedef LanguageMap::const_iterator LanguageMapCit;

// Helper struct to manage our globals
struct LocalizationSettings
{
	LocalizationSettings() : defaultLanguage( LANGUAGEID_DEFAULT ), timeDelta( 0 ) 
	{
		Language* lang = CCP_NEW( "default language" ) Language( LANGUAGEID_SYSTEM_DEFAULT );
		languages.insert( LanguageMap::value_type( LANGUAGEID_SYSTEM_DEFAULT, lang ) );
	}

	LanguageID              defaultLanguage;    // fallback for messages not found in requested language
	LanguageMap             languages;	        // map of all loaded languages and their data
	PropertyHandlerMap      propertyHandlerMap; // map of all available property handlers
	time_t					timeDelta;			// allow shifting the displayed time by a certain amount of time
};
extern LocalizationSettings g_settings;

// enum converters
extern LanguageID CodeToLanguageID( const char* );
extern const char* LanguageIDToCode( LanguageID );
extern const char* LanguageIDToLocaleName( LanguageID languageID );

// Python exposed
extern PyObject* PyGetMessageByID( PyObject* module, PyObject* args, PyObject* kwargs );
extern PyObject* PyFormatNumeric( PyObject* module, PyObject* args, PyObject* kwargs );

// quantities
extern size_t GetType1QuantityCategory( int64_t quantity );
extern size_t GetType2QuantityCategory( int64_t quantity );
extern size_t GetType3QuantityCategory( int64_t quantity );

// Stuff
extern bool LoadToken( PyObject* dict, Token& token );
extern bool LoadTokens( PyObject* dict, MessageData& messageData );
extern size_t FormatNumber( wchar_t (&out)[STACK_BUFFER_SIZE_LARGE], PyObject* value, int numDigits = 2, int leadingZeroes = 0, bool useGrouping = false );


std::wstring PyUnicodeToWString( PyObject* unicode );

#ifdef __APPLE__
CFStringRef ToStringRef( const char* string );
CFStringRef ToStringRef( const std::string& string );
CFStringRef ToStringRef( const wchar_t* string, size_t length );
CFStringRef ToStringRef( const std::wstring& string );

template <typename T>
class AutoReleaseCF
{
public:
    AutoReleaseCF()
    :   m_ref( nullptr )
    {
    }

    AutoReleaseCF( T ref )
    :   m_ref( ref )
    {
    }
    
    ~AutoReleaseCF()
    {
        if( m_ref )
        {
            CFRelease( m_ref );
        }
    }
    
    AutoReleaseCF& operator=( T ref )
    {
        if( m_ref == ref )
        {
            return *this;
        }
        if( m_ref )
        {
            CFRelease( m_ref );
        }
        m_ref = ref;
        return* this;
    }
    
    operator T() const
    {
        return m_ref;
    }
private:
    T m_ref;
};
#endif

#endif // Localization_H
