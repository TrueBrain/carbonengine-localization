#include <cstdio>
#include <vector>

#include "flatbuffers/flatbuffers.h"
#include "messages_generated.h"

// -------------------------------------------------------------
// Description:
//   Outputs a flatbuffer as Python bytearray code to stdout.
// Arguments:
//   name - The name of the Python variable to create
//   buf - Pointer to the buffer data
//   size - Size of the buffer in bytes
// Return value:
//   None
// -------------------------------------------------------------
void PrintBufferAsPythonByteArray( const char* name, const uint8_t* buf, size_t size )
{
	printf( "%s = bytearray([\n    ", name );
	
	for ( size_t i = 0; i < size; i++ )
	{
		if ( i > 0 && i % 16 == 0 )
		{
			printf( "\n    " );
		}
		printf( "0x%02x", buf[i] );
		if ( i < size - 1 )
		{
			printf( ", " );
		}
	}
	
	printf( "\n])\n" );
}

// -------------------------------------------------------------
// Description:
//   Generates test flatbuffer data and outputs it as Python
//   bytearray code to stdout for use in Python tests.
// Arguments:
//   None
// Return value:
//   0
// -------------------------------------------------------------
int main()
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

	// Get the buffer
	uint8_t* buf = builder.GetBufferPointer();
	size_t size = builder.GetSize();

	// Output as Python byte array
	printf( "# This data was generated with the generate_flatbuffers_test_data tool\n" );
	PrintBufferAsPythonByteArray( "test_flatbuffer_data", buf, size );


	// Generate empty messages buffer for testing error handling

	flatbuffers::FlatBufferBuilder empty_builder( 256 );
	std::vector<flatbuffers::Offset<eve::localization::Message>> empty_messages;
	auto empty_messages_vector = empty_builder.CreateVector( empty_messages );
	auto empty_all_messages = eve::localization::CreateAllMessages( empty_builder, empty_messages_vector );
	empty_builder.Finish( empty_all_messages );
	
	uint8_t* empty_buf = empty_builder.GetBufferPointer();
	size_t empty_size = empty_builder.GetSize();
	
	PrintBufferAsPythonByteArray( "empty_messages_buffer", empty_buf, empty_size );

	return 0;
}
