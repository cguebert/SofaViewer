#pragma once

#ifndef WIN32
#	define EXPORT_DYNAMIC_LIBRARY
#	define IMPORT_DYNAMIC_LIBRARY
#else
#	define EXPORT_DYNAMIC_LIBRARY __declspec( dllexport )
#	define IMPORT_DYNAMIC_LIBRARY __declspec( dllimport )
#	pragma warning(disable : 4251)
#endif

#ifdef BUILD_SERIALIZATION
#	define SERIALIZATION_API EXPORT_DYNAMIC_LIBRARY
#else
#	define SERIALIZATION_API IMPORT_DYNAMIC_LIBRARY
#endif