#include <cstdio>
#include <vector>

#include "BlueExposure.h"

#include "flatbuffers/flatbuffers.h"
#include "messages_generated.h"

PyObject* PyGenerateValidFlatbuffersTestData( PyObject* module, PyObject* args )
{
	flatbuffers::FlatBufferBuilder builder( 1024 );

	// Create some test messages
	std::vector<flatbuffers::Offset<eve::localization::Message>> messages;

	// Message 0: Simple message with no tags
	{
		auto text = builder.CreateString( "This is a simple test message." );
		auto msg = eve::localization::CreateMessage( builder, 0, text );
		messages.push_back( msg );
	}

	// Message 1: Message with a generic token
	{
		auto text = builder.CreateString( "Hello {name}!" );
		
		// Create token for {name}
		auto markup = builder.CreateString( "{name}" );
		auto variable_name = builder.CreateString( "name" );
		auto token = eve::localization::CreateToken(
			builder,
			markup,
			eve::localization::VariableTypes_generic,
			variable_name,
			0, // property_name
			0  // args
		);
		
		std::vector<flatbuffers::Offset<eve::localization::Token>> tokens;
		tokens.push_back( token );
		auto tokens_vector = builder.CreateVector( tokens );
		
		auto msg = eve::localization::CreateMessage( builder, 1, text, tokens_vector );
		messages.push_back( msg );
	}

	// Message 2: Message with metadata
	{
		auto text = builder.CreateString( "Test message with metadata" );
		
		// Create metadata
		auto prop_name = builder.CreateString( "context" );
		auto prop_text = builder.CreateString( "test_context" );
		auto metadata = eve::localization::CreateMetadata( builder, prop_name, prop_text );
		
		std::vector<flatbuffers::Offset<eve::localization::Metadata>> metadata_vector;
		metadata_vector.push_back( metadata );
		auto metadata_vec = builder.CreateVector( metadata_vector );
		
		auto msg = eve::localization::CreateMessage( builder, 2, text, 0, metadata_vec );
		messages.push_back( msg );
	}

	// Message 3: Message with numeric token and kwargs
	{
		auto text = builder.CreateString( "You have {count} items." );
		
		// Create token with kwargs
		auto markup = builder.CreateString( "{count}" );
		auto variable_name = builder.CreateString( "count" );
		
		// Create kwargs - decimalPlaces
		auto kwarg_key = builder.CreateString( "decimalPlaces" );
		auto kwarg_number = eve::localization::CreateKwargNumber( builder, 0 );
		auto kwarg = eve::localization::CreateKwarg(
			builder,
			kwarg_key,
			eve::localization::KwargValue_KwargNumber,
			kwarg_number.Union()
		);
		
		std::vector<flatbuffers::Offset<eve::localization::Kwarg>> kwargs;
		kwargs.push_back( kwarg );
		auto kwargs_vector = builder.CreateVector( kwargs );
		
		auto token = eve::localization::CreateToken(
			builder,
			markup,
			eve::localization::VariableTypes_numeric,
			variable_name,
			0, // property_name
			eve::localization::ArgFlag_decimalPlaces,
			kwargs_vector
		);
		
		std::vector<flatbuffers::Offset<eve::localization::Token>> tokens;
		tokens.push_back( token );
		auto tokens_vector = builder.CreateVector( tokens );
		
		auto msg = eve::localization::CreateMessage( builder, 3, text, tokens_vector );
		messages.push_back( msg );
	}

	// Create the AllMessages table
	auto messages_vector = builder.CreateVector( messages );
	auto all_messages = eve::localization::CreateAllMessages( builder, messages_vector );
	builder.Finish( all_messages );

	// Return the byte buffer
	auto msg_bytes = builder.GetBufferPointer();
	auto msg_size = builder.GetSize();
	return PyByteArray_FromStringAndSize( reinterpret_cast<const char*>( msg_bytes ), static_cast<Py_ssize_t>( msg_size ) );
}
MAP_FUNCTION( "GenerateValidFlatbuffersTestData", PyGenerateValidFlatbuffersTestData, "Creates a byte array containing serialized flatbuffers test data." );

PyObject* PyGenerateInvalidFlatbuffersTestData( PyObject* module, PyObject* args )
{
	// Return an invalid flatbuffer (just some random bytes)
	const char invalid_data[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };
	return PyByteArray_FromStringAndSize( invalid_data, sizeof( invalid_data ) );
}
MAP_FUNCTION( "GenerateInvalidFlatbuffersTestData", PyGenerateInvalidFlatbuffersTestData, "Creates a byte array containing invalid flatbuffers data." );

PyObject* PyGenerateEmptyFlatbuffersTestData( PyObject* module, PyObject* args )
{
	flatbuffers::FlatBufferBuilder builder( 1024 );

	// Create an empty AllMessages table
	auto messages_vector = builder.CreateVector( std::vector<flatbuffers::Offset<eve::localization::Message>>() );
	auto all_messages = eve::localization::CreateAllMessages( builder, messages_vector );
	builder.Finish( all_messages );

	// Return the byte buffer
	auto msg_bytes = builder.GetBufferPointer();
	auto msg_size = builder.GetSize();
	return PyByteArray_FromStringAndSize( reinterpret_cast<const char*>( msg_bytes ), static_cast<Py_ssize_t>( msg_size ) );
}
MAP_FUNCTION( "GenerateEmptyFlatbuffersTestData", PyGenerateEmptyFlatbuffersTestData, "Creates a byte array containing flatbuffers data with no messages." );


// DLL entry point and module registration

const char* g_moduleName = "EveLocalizationTest";

static void StartDLL()
{
	BeClasses->RegisterClasses( BlueRegistration::GetClassRegs() );
	
	PyObject* module = Py_InitModule( CCP_STRINGIZE( CCP_CONCATENATE( EveLocalizationTest, CCP_BUILD_FLAVOR ) ), NULL );
	
	BlueRegisterToModule( module, 
						  BlueRegistration::GetClassRegs(), 
						  BlueRegistration::GetFuncRegs(), 
						  BlueRegistration::GetEnumRegs() );
}

#ifdef _WIN32
BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(instance);
	}
	else if (reason == DLL_PROCESS_DETACH)
	{
		;
	}
    return TRUE;
}
#endif

//-----------------------------------------------------------------------------
// init - python dll module entry function
//-----------------------------------------------------------------------------
extern "C" void
#ifdef _WIN32
__declspec(dllexport)
#else
__attribute__((visibility ("default")))
#endif
CCP_CONCATENATE( initEveLocalizationTest, CCP_BUILD_FLAVOR )()
{
	StartDLL();
}
