#include "stdafx.h"

#include <fstream>
#include <string>
#include <vector>

#include "localization.h"
#ifdef USE_FLATBUFFERS
#include "messages_generated.h"
#endif
#include "parser.h"

const Be::VarChooser TokenFlagChooser[] =
{
	{ "LINKIFY",        BeCast( TOKENFLAG_LINKIFY ),        "This token should be turned into a link" },
	{ "CAPITALIZE",     BeCast( TOKENFLAG_CAPITALIZE ),     "This token should be capitalized" },
	{ "UPPERCASE",      BeCast( TOKENFLAG_UPPERCASE ),      "This token should be uppercased" },
	{ "LOWERCASE",      BeCast( TOKENFLAG_LOWERCASE ),	    "This token should be lowercased" },
	{ "TITLECASE",      BeCast( TOKENFLAG_TITLECASE ),      "This token should be titlecased" },
	{ "USEGROUPING",    BeCast( TOKENFLAG_USEGROUPING ),    "This token should use numeric grouping" },
	{ "CONDITIONAL",    BeCast( TOKENFLAG_CONDITIONAL ),    "This token includes a conditional value" },
	{ "LINKINFO",       BeCast( TOKENFLAG_LINKINFO ),       "This token includes a linkinfo tag" },
	{ "QUANTITY",       BeCast( TOKENFLAG_QUANTITY ),       "This token includes a quantity tag" },
	{ "DECIMALPLACES",  BeCast( TOKENFLAG_DECIMALPLACES ),  "This token includes a decimalPlaces keyword for number formatting." },
	{ "LEADINGZEROES",  BeCast( TOKENFLAG_LEADINGZEROES ),  "This token includes a leadingZeroes keyword for number formatting." },
	{ 0	}
};
BLUE_REGISTER_ENUM_EX("TOKEN_FLAG", TokenFlag, TokenFlagChooser, ENUM_REG_ENUM_OBJECT_ON_MODULE);

const Be::VarChooser VariableTypeChooser[] = 
{
	{ "CHARACTER",           BeCast( VARIABLETYPE_CHARACTER ),           "This token should be handled by character handlers." },
	{ "NPCORGANIZATION",     BeCast( VARIABLETYPE_NPCORGANIZATION ),     "This token should be handled by npcorganization handlers." },
	{ "ITEM",                BeCast( VARIABLETYPE_ITEM ),                "This token should be handled by item handlers." },
	{ "LOCATION",            BeCast( VARIABLETYPE_LOCATION ),            "This token should be handled by location handlers." },
	{ "CHARACTERLIST",       BeCast( VARIABLETYPE_CHARACTERLIST ),       "This token should be handled by characterlist handlers." },
	{ "MESSAGE",             BeCast( VARIABLETYPE_MESSAGE ),             "This token should be handled by message handlers." },
	{ "DATETIME",            BeCast( VARIABLETYPE_DATETIME ),            "This token should be handled by datetime handlers." },
	{ "FORMATTEDTIME",       BeCast( VARIABLETYPE_FORMATTEDTIME ),       "This token should be handled by formattedtime handlers." },
	{ "TIMEINTERVAL",        BeCast( VARIABLETYPE_TIMEINTERVAL ),        "This token should be handled by timeinterval handlers." },
	{ "NUMERIC",             BeCast( VARIABLETYPE_NUMERIC ),             "This token should be handled by numeric handlers." },
	{ "GENERIC",             BeCast( VARIABLETYPE_GENERIC ),             "This token should be handled by generic handlers." },
	{ 0	}
};
BLUE_REGISTER_ENUM_EX("VARIABLE_TYPE", VariableType, VariableTypeChooser, ENUM_REG_ENUM_OBJECT_ON_MODULE);

// -------------------------------------------------------------
// Description:
//   Creates the eveLocalization storage for the given language if it does not exist yet, otherwise
//   it updates an existing storage with the provided information.
// Arguments:
//   module - Ignored
//   args - The language for which we provide the messages and a dictionary describing message data.
//   The format of the message data dictionary must be this: {messageID:(text,metadata,tokens),...}
// Return value:
//   None.
// -------------------------------------------------------------
PyObject* PyLoadMessageData( PyObject* module, PyObject* args )
{
	CCP_STATS_ZONE( __FUNCTION__ );

	char* languageCode = 0;
	PyObject* dataDict = 0;

	if ( PyArg_ParseTuple( args, "sO", &languageCode, &dataDict ) )
	{
		if ( !PyDict_Check( dataDict ) )
		{
			PyErr_SetString( PyExc_TypeError, "Localization::LoadMessageData expects a dictionary." );
			return NULL;
		}

		LanguageID langID = CodeToLanguageID( languageCode );
		Language* lang = CCP_NEW( "language" ) Language( langID );
		MessageMap& mm = lang->data;
		PyObject* key;
		PyObject* value;
		Py_ssize_t pos = 0;
		while ( PyDict_Next( dataDict, &pos, &key, &value ) )
		{
			if( !key )
			{
				PyErr_SetString( PyExc_TypeError, "Localization::LoadMessageData - key is NULL" );
				return NULL;
			}

			if ( ! PyTuple_Check( value ) || PyTuple_Size( value ) != 3 )
			{
				PyErr_SetString( PyExc_TypeError, 
								 "Localization::LoadMessageData - dictionary value must be a 3 element tuple describing sourceText, metaData and tokens." );
				return NULL;
			}

			MessageID id;
			if ( PyLong_Check( key ) )
			{
				id = PyLong_AsUnsignedLongLong( key );
			}
			else if ( PyInt_Check( key ) )
			{
				id = PyInt_AsUnsignedLongLongMask( key );
			}
			else
			{
				PyErr_SetString( PyExc_TypeError, "Localization::LoadMessageData - dictionary must be keyed on int or long" );
				return NULL;
			}

			MessageData* md = CCP_NEW( "PyLoadMessageData/md" ) MessageData;

			// We use PyTuple_GET_ITEM for speed - we've already checked that we have a tuple
			// and that it is of the correct size. Also remember that PyTuple_GET_ITEM returns
			// a borrowed reference - we're not leaking the objects.

			PyObject* text  = PyTuple_GET_ITEM( value, 0 );
			if ( ! PyUnicode_Check( text ) )
			{
				PyErr_SetString( PyExc_TypeError, "Localization::LoadMessageData - first tuple entry must be unicode string" );
				return NULL;
			}

			PyObject* metaData = PyTuple_GET_ITEM( value, 1 );
			if ( metaData != Py_None )
			{
				if ( ! PyDict_Check( metaData ) )
				{
					PyErr_SetString( PyExc_TypeError, "Localization::LoadMessageData - second tuple entry must be a dictionary or None" );
					return NULL;
				}

				md->metaData = CCP_NEW( "eveLocalization/MetaData" ) MetaData();

				PyObject* key;
				PyObject* val;
				Py_ssize_t pos = 0;
				while ( PyDict_Next( metaData, &pos, &key, &val ) )
				{
					// temporary bug in pickle processing means that metadata keys are being exported as unicode.
					// Convert to string until this is fixed.
					PyObject *tmp = PyObject_Str(key);
					if (tmp)
					{
						std::string  k( PyString_AS_STRING( tmp ) );
						Py_DECREF(tmp);
                        std::wstring v( PyUnicodeToWString( val ) );
						md->metaData->insert( MetaData::value_type( k, v ) );
					}
				}
			}

			PyObject* token = PyTuple_GET_ITEM( value, 2 );
			if ( token != Py_None )
			{
				if ( ! PyDict_Check( token ) )
				{
					PyErr_SetString( PyExc_TypeError, "Localization::LoadMessageData - third tuple entry must be a dictionary or None" );
					return NULL;
				}

				if ( ! LoadTokens( token, *md ) )
				{
					// Error state was set from within ParseTokens
					return NULL;
				}
			}

			md->text = PyUnicodeToWString( text );

			mm[id] = md;
		}

		LanguageMap::iterator oldLang = g_settings.languages.find( langID );
		if ( oldLang != g_settings.languages.end() )
		{
			CCP_STATS_ZONE( "Updating language" );

			// update the already-existing language with new data
			for ( MessageMap::iterator i = lang->data.begin(); i != lang->data.end(); ++i )
			{
				oldLang->second->data[i->first] = i->second;
			}
		}
		else
		{
			CCP_STATS_ZONE( "Inserting language" );

			// insert the new language
			g_settings.languages.insert( LanguageMap::value_type( langID, lang ) );
		}
	}
	Py_RETURN_NONE;
}
MAP_FUNCTION( "LoadMessageData", PyLoadMessageData, "Load the message data we are operating on." );

#ifdef USE_FLATBUFFERS
// -------------------------------------------------------------
// Description:
//   Converts a flatbuffer VariableTypes enum to an eveLocalization
//   VariableType.
// Arguments:
//   fbType - The flatbuffer VariableTypes enum value to convert.
// Return value:
//   The corresponding eveLocalization VariableType enum value.
// -------------------------------------------------------------
VariableType ConvertVariableType(eve::localization::VariableTypes fbType) {
	switch (fbType) {
		case eve::localization::VariableTypes_character:
			return VARIABLETYPE_CHARACTER;
		case eve::localization::VariableTypes_npcOrganization:
			return VARIABLETYPE_NPCORGANIZATION;
		case eve::localization::VariableTypes_item:
			return VARIABLETYPE_ITEM;
		case eve::localization::VariableTypes_location:
			return VARIABLETYPE_LOCATION;
		case eve::localization::VariableTypes_characterlist:
			return VARIABLETYPE_CHARACTERLIST;
		case eve::localization::VariableTypes_messageid:
			return VARIABLETYPE_MESSAGE;
		case eve::localization::VariableTypes_datetime:
			return VARIABLETYPE_DATETIME;
		case eve::localization::VariableTypes_formattedtime:
			return VARIABLETYPE_FORMATTEDTIME;
		case eve::localization::VariableTypes_timeinterval:
			return VARIABLETYPE_TIMEINTERVAL;
		case eve::localization::VariableTypes_numeric:
			return VARIABLETYPE_NUMERIC;
		case eve::localization::VariableTypes_generic:
			return VARIABLETYPE_GENERIC;
		default:
			return VARIABLETYPE_GENERIC;
	}
}

// -------------------------------------------------------------
// Description:
//   Loads message data from a flatbuffer AllMessages object and creates or updates
//   the eveLocalization storage for the given language. This function parses the
//   flatbuffer data structure and converts it to the internal MessageData format,
//   including text, metadata, and tokens.
// Arguments:
//   languageCode - The language code for which we provide the messages.
//   allMessages - Pointer to the flatbuffer AllMessages object containing the message data.
// Return value:
//   None.
// -------------------------------------------------------------
void LoadMessageDataFromFlatbufferObject( const char* languageCode, const eve::localization::AllMessages* allMessages )
{
	const auto* fbMessages = allMessages->messages();

	// Create the language object
	LanguageID langID = CodeToLanguageID( languageCode );
	Language* lang = CCP_NEW( "PyLoadMessageFromFlatbuffer/Language" ) Language( langID );
	MessageMap& mm = lang->data;

	// Iterate through the messages and populate our message map
	for ( const auto fbMessagePtr : *fbMessages )
	{
		if ( ! fbMessagePtr )
		{
			continue;
		}
		const auto& fbMessage = *fbMessagePtr;

		MessageData* md = CCP_NEW( "PyLoadMessageDataFromFlatbuffer/MessageData" ) MessageData();

		// id
		MessageID msgID = fbMessage.id();

		// text
		std::wstring msgText;
		if ( fbMessage.text() )
		{
			if ( !UTF8ToWString( fbMessage.text()->str(), msgText ) )
			{
				msgText.clear();
			}
		}
		md->text = msgText;

		// metadata
		MetaDataPtr metaData = CCP_NEW( "PyLoadMessageDataFromFlatbuffer/MetaData" ) MetaData();
		if ( fbMessage.metadata() && fbMessage.metadata()->size() > 0 )
		{
			for ( const auto& fbMeta : *fbMessage.metadata() )
			{
				if ( fbMeta->property_name() && fbMeta->text() )
				{
					std::string propertyName = fbMeta->property_name()->str();
					std::wstring text;
					if ( UTF8ToWString( fbMeta->text()->str(), text ) )
					{
						metaData->insert( MetaData::value_type( propertyName, text ) );
					}
				}
			}
		}
		md->metaData = metaData;

		// tokens
		TokenContainerPtr tokens = CCP_NEW( "PyLoadMessageDataFromFlatbuffer/TokenContainer" ) TokenContainer();
		if ( fbMessage.tokens() && fbMessage.tokens()->size() > 0 )
		{
			for ( const auto& fbToken : *fbMessage.tokens() )
			{
				Token* token = CCP_NEW( "PyLoadMessageDataFromFlatbuffer/Token" ) Token();

				// markup
				std::wstring markup;
				if ( fbToken->markup() )
				{
					if ( !UTF8ToWString( fbToken->markup()->str(), markup ) )
					{
						CCP_DELETE token;
						continue;
					}
				}
				else
				{
					CCP_DELETE token;
					continue;
				}
				token->tagName = markup;

				// variableType
				token->variableType = ConvertVariableType( fbToken->variable_type() );

				// variableName
				if ( fbToken->variable_name() )
				{
					token->variableName = fbToken->variable_name()->str();
				}

				// propertyName
				if ( fbToken->property_name() )
				{
					token->propertyName = fbToken->property_name()->str();
				}

				// flags
				token->flags = static_cast<TokenFlags>( fbToken->args() );

				// kwargs
				if ( fbToken->kwargs() && fbToken->kwargs()->size() > 0 )
				{
					token->kwargs = CCP_NEW( "PyLoadMessageDataFromFlatbuffer/KeywordArgs" ) KeywordArgs;

					for ( const auto& fbKwarg : *fbToken->kwargs() )
					{
						if ( ! fbKwarg->key() || ! fbKwarg->value() )
						{
							continue;
						}

						std::string key = fbKwarg->key()->str();
						PyObject* value = nullptr;
						if ( fbKwarg->value_type() == eve::localization::KwargValue_KwargNumber && fbKwarg->value_as_KwargNumber() )
						{
							value = PyLong_FromLongLong( fbKwarg->value_as_KwargNumber()->value() );
						}
						else if ( fbKwarg->value_type() == eve::localization::KwargValue_KwargString )
						{
							if ( fbKwarg->value_as_KwargString() && fbKwarg->value_as_KwargString()->value() )
							{
								std::string strValue = fbKwarg->value_as_KwargString()->value()->str();
								value = PyUnicode_FromStringAndSize( strValue.c_str(), strValue.size() );
							}
							else
							{
								value = PyUnicode_FromStringAndSize( "", 0 );
							}
						}
						else
						{
							continue;
						}

						if ( value )
						{
							token->kwargs->insert( KeywordArgs::value_type( key, value ) );
						}
					}
				}

				// conditional_values
				if ( fbToken->conditional_values() && fbToken->conditional_values()->size() > 0 )
				{
					for ( size_t i = 0; i < fbToken->conditional_values()->size(); ++i )
					{
						const flatbuffers::String* cv = fbToken->conditional_values()->Get( i );
						if ( cv )
						{
							std::wstring value;
							if ( UTF8ToWString( cv->str(), value ) )
							{
								token->conditionalValues[i] = value;
							}
						}
					}
				}

				// This workaround section comes from the LoadTokens and LoadToken
				// functions (legacy)

				// -- BEGIN NO TOKENIZER UPDATES POSSIBLE WORKAROUND --
				// Temporary work around until we update the pickle generator again
				if ( token->variableType == VARIABLETYPE_DATETIME || token->variableType == VARIABLETYPE_FORMATTEDTIME )
				{
					// inject the formatting string
					if ( ! token->kwargs )
					{
						token->kwargs = CCP_NEW( "eveLocalization/KeywordArgs" ) KeywordArgs();
					}
					if ( token->kwargs->find( "format" ) == token->kwargs->cend() )
					{
						token->kwargs->insert( KeywordArgs::value_type( "format", PyUnicode_FromString( "%Y.%m.%d %H:%M" ) ) );
					}
				}

				// timeinterval needs special handling as it's the only property handler
				// with a default property that does not return the input value
				if ( token->variableType == VARIABLETYPE_TIMEINTERVAL )
				{
					// inject the default property name
					if ( token->propertyName.empty() )
					{
						token->propertyName = "shortForm";
					}
				}

				// see if we need leading zeroes
				if ( token->kwargs && token->kwargs->find( "leadingZeroes" ) != token->kwargs->cend() )
				{
					token->flags |= TOKENFLAG_LEADINGZEROES;
				}

				// Quantity kwarg superseeds a quantity flag because the quantity comes
				// from kwargs
				if ( token->kwargs && token->kwargs->find( "quantity" ) != token->kwargs->cend() )
				{
					if ( ( token->flags & TOKENFLAG_QUANTITY ) == TOKENFLAG_QUANTITY )
					{
						token->flags ^= TOKENFLAG_QUANTITY;
					}
				}

				// temporarily keep support for linkify
				if ( token->tagName.find( L"linkify" ) != std::wstring::npos )
				{
					token->flags |= TOKENFLAG_LINKIFY;
				}

				if ( token->tagName.find( L"linkinfo" ) != std::wstring::npos )
				{
					token->flags |= TOKENFLAG_LINKINFO;
				}

				// -- END WORKAROUND --

				// Done with token
				tokens->insert( TokenContainer::value_type( token->tagName, token ) );
			}
		}
		md->tokens = tokens;

		mm[msgID] = md;
	}

	// The language object is now populated, so we can insert it into g_settings,
	// or update the existing one.

	LanguageMap::iterator oldLang = g_settings.languages.find( langID );
	if ( oldLang != g_settings.languages.end() )
	{
		CCP_STATS_ZONE( "Updating language" );

		// update the already-existing language with new data
		for ( MessageMap::iterator i = lang->data.begin(); i != lang->data.end(); ++i )
		{
			oldLang->second->data[i->first] = i->second;
		}
	}
	else
	{
		CCP_STATS_ZONE( "Inserting language" );

		// insert the new language
		g_settings.languages.insert( LanguageMap::value_type( langID, lang ) );
	}

	return;
}

// -------------------------------------------------------------
// Description:
//   Reads flatbuffer message data from a file, then creates or updates the
//   eveLocalization storage for the given language.
// Arguments:
//   module - Ignored
//   args - The language for which we provide the messages and a filename to
//   load the flatbuffer data from.
// Return value:
//   None.
// -------------------------------------------------------------
PyObject* PyLoadMessageDataFromFlatbufferFile( PyObject* module, PyObject* args )
{
	CCP_STATS_ZONE( __FUNCTION__ );

	char* languageCode = 0;
	char* filePath = 0;

	if ( ! PyArg_ParseTuple( args, "ss", &languageCode, &filePath ) )
	{
		return NULL;
	}

	// Read the file
	std::ifstream file( filePath, std::ios::binary | std::ios::ate );
	if ( ! file.is_open() )
	{
		PyErr_SetString( PyExc_IOError, "Could not open flatbuffer file" );
		return NULL;
	}
	std::streamsize size = file.tellg();
	if ( size <= 0 )
	{
		PyErr_SetString( PyExc_IOError, "Flatbuffer file is empty or invalid" );
		return NULL;
	}
	file.seekg( 0, std::ios::beg );

	std::vector<char> buffer( size );
	if ( ! file.read( buffer.data(), size ) )
	{
		PyErr_SetString( PyExc_IOError, "Could not read flatbuffer file" );
		return NULL;
	}

	// Verify the flatbuffer data
	flatbuffers::Verifier verifier( reinterpret_cast<const uint8_t*>( buffer.data() ), buffer.size() );
	if ( ! eve::localization::VerifyAllMessagesBuffer( verifier ) )
	{
		PyErr_SetString( PyExc_ValueError, "Flatbuffer file failed verification" );
		return NULL;
	}

	// Get the root messages object
	const eve::localization::AllMessages* allMessages = eve::localization::GetAllMessages( buffer.data() );
	if ( ! allMessages )
	{
		PyErr_SetString( PyExc_ValueError, "Failed to get the root object from the flatbuffer" );
		return NULL;
	}

	if ( ! allMessages->messages() || allMessages->messages()->size() == 0 )
	{
		PyErr_SetString( PyExc_ValueError, "Flatbuffer does not contain any messages" );
		return NULL;
	}

	LoadMessageDataFromFlatbufferObject( languageCode, allMessages );

	Py_RETURN_NONE;
}
MAP_FUNCTION( "LoadMessageDataFromFlatbufferFile", PyLoadMessageDataFromFlatbufferFile, "Load the message data we are operating on from a flatbuffer file." );
#endif

// -------------------------------------------------------------
// Description:
//   Clear the whole eveLocalization storage.
// Arguments:
//   module - Ignored
//   args - Ignored
// Return value:
//   None
// -------------------------------------------------------------
PyObject* PyUnloadAllMessageData( PyObject* module, PyObject* args )
{
	for( auto it = g_settings.languages.begin(); it != g_settings.languages.end(); ++it )
	{
		CCP_DELETE it->second;
	}
	g_settings.languages.clear();

    // And make sure that we do have the default language, as it won't be reinitialized elsewhere
    Language* lang = CCP_NEW( "language" ) Language( LANGUAGEID_SYSTEM_DEFAULT );
    g_settings.languages.insert( LanguageMap::value_type( LANGUAGEID_SYSTEM_DEFAULT, lang ));

	Py_RETURN_NONE;
}
MAP_FUNCTION( "UnloadAllMessageData", PyUnloadAllMessageData, "Unloads all stored message data." );

// -------------------------------------------------------------
// Description:
//   Clear the eveLocalization storage for the specified language.
// Arguments:
//   module - Ignored
//   args - Argument tuple passed in from python. Must be the language we want to unload.
// Return value:
//   None
// -------------------------------------------------------------
PyObject* PyUnloadMessageData( PyObject* module, PyObject* args )
{
	char* languageCode;
	if ( ! PyArg_ParseTuple( args, "s", &languageCode ) )
	{
		return NULL;
	}

	LanguageID langID = CodeToLanguageID( languageCode );
	
	LanguageMapIt it = g_settings.languages.find( langID );
	if ( it != g_settings.languages.end() )
	{
		g_settings.languages.erase( it );
	}

	Py_RETURN_NONE;
}
MAP_FUNCTION( "UnloadMessageData", PyUnloadMessageData, "Unloads message data for the specified language, if it's loaded." );

// -------------------------------------------------------------
// Description:
//   Determine if a message for the given ID exists in the specified language.
// Arguments:
//   module - Ignored
//   args - Argument tuple passed in from python. Must be message ID and language.
// Return value:
//   True if the message exists, false otherwise.
// -------------------------------------------------------------
PyObject* PyIsValidMessageID( PyObject* module, PyObject* args )
{
	MessageID messageID;
	char* languageCode;
	if ( ! PyArg_ParseTuple( args, "Ks", &messageID, &languageCode ) )
	{
		return NULL;
	}

	LanguageID langID = CodeToLanguageID( languageCode );
	LanguageMapCit cit = g_settings.languages.find( langID );
	if ( cit == g_settings.languages.cend() )
	{
		Py_RETURN_FALSE;
	}

	MessageMapCit msg = cit->second->data.find( messageID );
	if ( msg == cit->second->data.cend() )
	{
		Py_RETURN_FALSE;
	}

	Py_RETURN_TRUE;
}
MAP_FUNCTION( "IsValidMessageID", PyIsValidMessageID, "Checks if messageID exists in the specified language." );

// -------------------------------------------------------------
// Description:
//   Return the raw text for a specified message id and language.
// Arguments:
//   module - Ignored
//   args - Argument tuple passed in from python. Must be message ID and language.
// Return value:
//   A unicode string with the raw message text.
// -------------------------------------------------------------
PyObject* PyGetRawByMessageID( PyObject* module, PyObject* args )
{
	MessageID messageID;
	char* languageCode;
	if ( ! PyArg_ParseTuple( args, "Ks", &messageID, &languageCode ) )
	{
		return NULL;
	}

	LanguageID langID = CodeToLanguageID( languageCode );
	LanguageMapCit cit = g_settings.languages.find( langID );
	if ( cit == g_settings.languages.cend() )
	{
		PyErr_SetString( PyExc_KeyError, "No such language." );
		return NULL;
	}

	MessageMapCit msg = cit->second->data.find( messageID );
	if ( msg == cit->second->data.cend() )
	{
		PyErr_SetString( PyExc_KeyError, "No such message." );
		return NULL;
	}

	return PyUnicode_FromWideChar( msg->second->text.c_str(), msg->second->text.size() );
}
MAP_FUNCTION( "GetRawByMessageID", PyGetRawByMessageID, "Returns the raw, unparsed message by ID" );

// -------------------------------------------------------------
// Description:
//   Fetch the actual data for a given messageID and language.
// Arguments:
//   module - Ignored
//   args - Argument tuple passed in from python. Must include messageID and language.
// Return value:
//   A 3-tuple describing (text, metadata, tokens)
// -------------------------------------------------------------
PyObject* PyGetMessageDataByID( PyObject* module, PyObject* args )
{
	MessageID messageID;
	char* languageCode;
	PyObject* ret = NULL;

	if ( PyArg_ParseTuple( args, "Ks", &messageID, &languageCode ) )
	{
		LanguageID lid = CodeToLanguageID( languageCode );
		Language& lang = *g_settings.languages[lid];
		MessageMapCit mdit = lang.data.find(messageID);

		if ( mdit == lang.data.cend() )
		{
			PyErr_SetString( PyExc_KeyError, "No such element" );
			return NULL;
		}

		const MessageData& msg = *mdit->second;

		PyObject* text = PyUnicode_FromWideChar( msg.text.c_str(), msg.text.size() );
		PyObject* tokens = NULL;
		PyObject* metaData = NULL;

		if ( msg.metaData )
		{
			metaData = PyDict_New();
			for ( MetaDataCit cit = msg.metaData->cbegin(); cit != msg.metaData->cend(); ++cit )
			{
				PyObject* tmp = PyUnicode_FromWideChar( cit->second.c_str(), cit->second.size() );
				PyDict_SetItemString( metaData, cit->first.c_str(), tmp );
				Py_XDECREF( tmp );
			}
		}

		if ( msg.tokens )
		{
			tokens = PyDict_New();
			for ( TokenContainerCit cit = msg.tokens->cbegin(); cit != msg.tokens->cend(); ++cit )
			{
				const Token& tok = *cit->second;

				PyObject* temp = NULL;
				PyObject* token = PyDict_New();

				temp = PyString_FromStringAndSize( tok.variableName.c_str(), tok.variableName.size() );
				PyDict_SetItemString( token, "variableName", temp);
				Py_XDECREF( temp );

				temp = PyInt_FromLong( (long) tok.variableType );
				PyDict_SetItemString( token, "variableType", temp);
				Py_XDECREF( temp );

				if ( tok.propertyName.size() > 0 )
				{
					temp = PyString_FromStringAndSize( tok.propertyName.c_str(), tok.propertyName.size() );
				}
				else
				{
					temp = Py_None;
                    Py_INCREF( temp );
				}
				PyDict_SetItemString( token, "propertyName", temp );
				Py_XDECREF( temp );

				temp = PyInt_FromLong( tok.flags );
				PyDict_SetItemString( token, "args", temp );
				Py_XDECREF( temp );

				temp = PyDict_New();
				if ( tok.kwargs )
				{
					for ( KeywordArgsCit kw = tok.kwargs->cbegin(); kw != tok.kwargs->cend(); ++kw )
					{
						PyDict_SetItemString( temp, kw->first.c_str(), kw->second );
					}
				}
				PyDict_SetItemString( token, "kwargs", temp );
				Py_XDECREF( temp );

				temp = PyList_New(0);
				for ( size_t i = 0; i < 3; ++i )
				{
					if ( 0 == tok.conditionalValues[i].size() )
					{
						break;
					}

					PyObject* tmp = PyUnicode_FromWideChar( tok.conditionalValues[i].c_str(), tok.conditionalValues[i].size() );
					PyList_Append( temp, tmp );
					Py_XDECREF( tmp );
				}
				PyDict_SetItemString( token, "conditionalValues", temp );
				Py_XDECREF( temp );

				temp = PyUnicode_FromWideChar( tok.tagName.c_str(), tok.tagName.size() );
				PyDict_SetItem( tokens, temp, token );
				Py_XDECREF( temp );
				Py_XDECREF( token );
			}
		}

		ret = Py_BuildValue("(OOO)", text, metaData ? metaData : Py_None, tokens ? tokens : Py_None );

		Py_XDECREF( text );
		Py_XDECREF( metaData );
		Py_XDECREF( tokens );
	}

	return ret;
}
MAP_FUNCTION( "GetMessageDataByID", PyGetMessageDataByID, "Returns the message data tuple for a specific message ID." );

// -------------------------------------------------------------
// Description:
//   Checks if this Language exists in the eveLocalization storage.
// Arguments:
//   module - Ignored
//   args - Argument tuple passed in from python. Must include the language.
// Return value:
//   True if there is MessageData for this language, False other.
// -------------------------------------------------------------
PyObject* PyHasLanguage( PyObject* module, PyObject* args )
{
	char* languageCode;
	if ( PyArg_ParseTuple( args, "s", &languageCode ) )
	{
		LanguageID langID = CodeToLanguageID( languageCode );
		LanguageMapCit lang = g_settings.languages.find( langID );

		if ( lang == g_settings.languages.cend() )
        {
            Py_RETURN_FALSE;
        }
        else
        {
            Py_RETURN_TRUE;
        }
	}

	return NULL;
}
MAP_FUNCTION( "HasLanguage", PyHasLanguage, "Returns true if we have data for the specified language, false otherwise." );

// -------------------------------------------------------------
// Description:
//   Set the default (a.k.a. primary) language that should be used as fallback.
// Arguments:
//   module - Ignored
//   args - Argument tuple passed in from python. Must include the language.
// Return value:
//   None
// -------------------------------------------------------------
PyObject* PySetPrimaryLanguage( PyObject* module, PyObject* args )
{
	char* languageCode;
	if ( PyArg_ParseTuple( args, "s", &languageCode ) )
	{
		g_settings.defaultLanguage = CodeToLanguageID( languageCode );
		Py_RETURN_NONE;
	}

	return NULL;
}
MAP_FUNCTION( "SetPrimaryLanguage", PySetPrimaryLanguage, "Set the default fallback language." );

// -------------------------------------------------------------
// Description:
//   Get the currently set default language
// Arguments:
//   module - Ignored
//   args - Ignored
// Return value:
//   Python representation of the default language currently set
// -------------------------------------------------------------
PyObject* PyGetPrimaryLanguage( PyObject* module, PyObject* args )
{
	return PyString_FromString( LanguageIDToCode( g_settings.defaultLanguage ) ) ;
}
MAP_FUNCTION( "GetPrimaryLanguage", PyGetPrimaryLanguage, "Returns the default fallback language." );

// -------------------------------------------------------------
// Description:
//   Returns the unicode symbol representing a decimal point.
// Arguments:
//   module - Ignored
//   args - Argument tuple passed in from python. Must include the language.
// Return value:
//   A unicode string containing the symbol to use as decimal point for the given language.
// -------------------------------------------------------------
PyObject* PyGetDecimalSeparator( PyObject* module, PyObject* args )
{
	PyObject *retVal = NULL;
	char* languageCode;

	if ( PyArg_ParseTuple( args, "s", &languageCode ) )
	{
		LanguageID langID = CodeToLanguageID( languageCode );
		LanguageMapCit lang = g_settings.languages.find( langID );
		if ( lang != g_settings.languages.cend() )
		{
			const Language& l = *lang->second;
			wchar_t w = l.decimalSep;
			retVal = PyUnicode_FromWideChar( &w, 1 );
		}
		else
		{
			PyErr_SetString( PyExc_KeyError, "Could not find language." );
		}
	}
	else
	{
		PyErr_SetString( PyExc_TypeError, "Method expects the language code to be passed in." );
	}
	
	return retVal;
}
MAP_FUNCTION( "GetDecimalSeparator", PyGetDecimalSeparator, "Returns the symbol to be used as decimal point in this locale." );

// -------------------------------------------------------------
// Description:
//   Returns the unicode symbol to use when grouping numbers.
//   Note: In general you would not want to use this directly because different locales have different rules for number grouping.
// Arguments:
//   module - Ignored
//   args - Argument tuple passed in from python. Must include the language we want to know the thousand separator for.
// Return value:
//   Unicode string describing the symbol to use for number grouping.
// -------------------------------------------------------------
PyObject* PyGetThousandSeparator( PyObject* module, PyObject* args )
{
	PyObject *retVal = NULL;
	char* languageCode;

	if ( PyArg_ParseTuple( args, "s", &languageCode ) )
	{
		LanguageID langID = CodeToLanguageID( languageCode );
		LanguageMapCit lang = g_settings.languages.find( langID );
		if ( lang != g_settings.languages.cend() )
		{
			const Language& l = *lang->second;
			wchar_t w = l.thousandSep;
			retVal = PyUnicode_FromWideChar( &w, 1 );
		}
		else
		{
			PyErr_SetString( PyExc_KeyError, "Could not find language." );
		}
	}
	else
	{
		PyErr_SetString( PyExc_TypeError, "Method expects the language code to be passed in." );
	}
	
	return retVal;
}
MAP_FUNCTION( "GetThousandSeparator", PyGetThousandSeparator, "Returns the symbol to be used as thousand separator in this locale." );

// -------------------------------------------------------------
// Description:
//   Python-exposed function to retrieve metadata associated with a message.
// Arguments:
//   module - Ignored
//   args - Argument tuple passed in from python. This must include the messageID, property name 
//   and language we want to fetch metadata for.
// Return value:
//   Metadata for the specified message, or a KeyError if not found
// -------------------------------------------------------------
PyObject* PyGetMetaDataByID( PyObject* module, PyObject* args )
{
	MessageID messageID;
	char* languageCode;
	char* propertyName;
	if ( PyArg_ParseTuple( args, "Kss", &messageID, &propertyName, &languageCode ) )
	{
		LanguageID langID = CodeToLanguageID( languageCode );

		LanguageMapCit lang = g_settings.languages.find( langID );
		if ( lang == g_settings.languages.cend() )
		{
			char tmp[128];
			sprintf_s(tmp, "Language %s does not exist.", languageCode );
			PyErr_SetString( PyExc_KeyError, tmp );
			return NULL;
		}

		const MessageMap& mm = lang->second->data;
		MessageMapCit cit = mm.find(messageID);

		// Not in the language we requested? Ah well, let's try with the fallback language then
		if ( cit == mm.cend() )
		{
			const MessageMap& mm = g_settings.languages[g_settings.defaultLanguage]->data;
			cit = mm.find(messageID);
			if ( cit == mm.cend() )
			{
				char tmp[128];
				sprintf_s(tmp, "Message ID %llu does not exists neither in requested nor in fallback language.", messageID );
				PyErr_SetString( PyExc_KeyError, tmp );
				return NULL;
			}
		}

		const MessageData& msg = *cit->second;

		if ( msg.metaData )
		{
			MetaDataCit cit = msg.metaData->find( propertyName );
			if ( cit != msg.metaData->cend() )
			{
				return PyUnicode_FromWideChar( cit->second.c_str(), cit->second.size() );
			}
		}

		char tmp[128];
		sprintf_s(tmp, "Message ID %llu does not have metadata for property %s.", messageID, propertyName );
		PyErr_SetString( PyExc_KeyError, tmp );
		return NULL;
	}

	return NULL;
}
MAP_FUNCTION( "GetMetaDataByID", PyGetMetaDataByID, "Returns the meta data associated with the given ID and property name."\
													"Raises KeyError if no metaData for the supplied parameters exists." );

// -------------------------------------------------------------
// Description:
//   Set the time delta for date/time formatting. Since we are always running in GMT but need to display GMT+8 for China
//   then we are adding 
// Arguments:
//   module - Ignored
//   args - Argument tuple passed in from python. This should contain the amount of seconds we add to the current time.
// Return value:
//   None if successful, raises an exception in error case
// -------------------------------------------------------------
PyObject* PySetTimeDelta( PyObject* module, PyObject* args )
{
	if ( ! PyArg_ParseTuple( args, "L", &g_settings.timeDelta ) )
	{
		return NULL;
	}
	Py_RETURN_NONE;
}
MAP_FUNCTION( "SetTimeDelta", PySetTimeDelta, "Adjust the actual time by timeDelta seconds when formatting date/time.");

// -------------------------------------------------------------
// Description:
//   Return the currently specified timeDelta, relative to GMT, in seconds.
// Arguments:
//   module - Ignored
//   args - Ignored
// Return value:
//   The currently specified timeDelta in seconds.
// -------------------------------------------------------------
PyObject* PyGetTimeDelta( PyObject* module, PyObject* args )
{
	PyObject* ret = PyLong_FromLongLong( (long long) g_settings.timeDelta );
	return ret;
}
MAP_FUNCTION( "GetTimeDelta", PyGetTimeDelta, "Returns the current timeDelta, in seconds.");

// -------------------------------------------------------------
// Description:
//   Allow python to fetch a localized message by ID and language.
//   Note: This is exposed from localization.cpp because the blue macros don't deal with kwargs
// Arguments:
//   module - Ignored
//   args - Argument tuple passed in from python. This should contain the ID of the message we are requesting as well as the
//   language the message should be in.
//   kwargs - Keyword arguments passed in from Python. These are the parameters for message replacement.
// Return value:
//   A unicode string containing the processed message, or a KeyError if not found.
// -------------------------------------------------------------
PyObject* PyGetMessageByID( PyObject* module, PyObject* args, PyObject* kwargs )
{
	MessageID messageID;
	char* languageCode;
	if ( PyArg_ParseTuple( args, "Ks", &messageID, &languageCode ) )
	{
		LanguageID langID = CodeToLanguageID( languageCode );

		LanguageMapCit lang = g_settings.languages.find( langID );
		if ( lang == g_settings.languages.cend() )
		{
			char tmp[128];
			sprintf_s(tmp, "Language %s does not exist.", languageCode );
			PyErr_SetString( PyExc_KeyError, tmp );
			return NULL;
		}

		const MessageMap& mm = lang->second->data;
		MessageMapCit cit = mm.find(messageID);

		// Not in the language we requested? Ah well, let's try with the fallback language then
		if ( cit == mm.cend() )
		{
			const MessageMap& mm = g_settings.languages[g_settings.defaultLanguage]->data;
			cit = mm.find(messageID);
			if ( cit == mm.cend() )
			{
				char tmp[128];
				sprintf_s(tmp, "Message ID %llu does not exists neither in requested nor in fallback language.", messageID );
				PyErr_SetString( PyExc_KeyError, tmp );
				return NULL;
			}
		}

		const MessageData& msg = *cit->second;

		if ( msg.tokens && kwargs )
		{
			std::wstringstream text;
			bool ret = Parse( msg.text, *lang->second, *( msg.tokens ), kwargs, text );
			if ( ! ret )
			{
				// Actual exception object should be set in Parse.
				return NULL;
			}
			std::wstring tmp = text.str();
			return PyUnicode_FromWideChar( tmp.c_str(), tmp.size() );			
		}
		else
		{
			return PyUnicode_FromWideChar( msg.text.c_str(), msg.text.size() );
		}
	}

	return NULL;
}

// -------------------------------------------------------------
// Description:
//   Python-exposed utility function for locale-aware number formatting.
//   Note: This is exposed from localization.cpp because the blue macros don't deal with kwargs
// Arguments:
//   module - Ignored
//   args - Argument tuple passed in from python
//   kwargs - Keyword arguments passed in from Python.
// Return value:
//   A unicode string containing the formatted number.
// See also:
//   NumericFormatter
// -------------------------------------------------------------
PyObject* PyFormatNumeric( PyObject* module, PyObject* args, PyObject* kwargs )
{
	PyObject* value = NULL;
	char* languageCode;
	size_t result = 0;
	wchar_t number[STACK_BUFFER_SIZE_LARGE];
	PyObject* tmp = NULL;

	if ( ! PyArg_ParseTuple( args, "Os", &value, &languageCode ) )
	{
		return NULL;
	}

	// Initialize our local variables...
	bool useGrouping = false;
	int decimalPlaces = 2;
	int leadingZeroes = 0;

	if ( kwargs )
	{
		tmp = PyDict_GetItemString( kwargs, "useGrouping" );
		if ( tmp )
		{
			useGrouping = tmp == Py_True;
		}
	
		tmp = PyDict_GetItemString( kwargs, "decimalPlaces" );
		if ( tmp && tmp != Py_None && PyInt_Check( tmp ) )
		{
			decimalPlaces = std::max( 0, std::min( 9, (int) PyInt_AS_LONG( tmp ) ) );
		}

		tmp = PyDict_GetItemString( kwargs, "leadingZeroes" );
		if ( tmp && tmp != Py_None && PyInt_Check( tmp ) )
		{
			leadingZeroes = std::max( 0, std::min( 9, (int) PyInt_AS_LONG( tmp ) ) );
		}
	}

	result = FormatNumber( number, value, decimalPlaces, leadingZeroes, useGrouping );

	if ( 0 == result )
	{
		return NULL;
	}

	return PyUnicode_FromWideChar( number, result );
}
