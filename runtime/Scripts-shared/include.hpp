//
// Created by Omen on 26-10-2022.
//

#ifndef GAMEENGINE_INCLUDE_HPP
#define GAMEENGINE_INCLUDE_HPP

#include <string>;
#include <vector>;

#if defined DLL_EXPORTS
	#if defined WIN32
		#define LIB_API(RetType) extern "C" __declspec (dllexport) RetType
	#else
		#define LIB_API(RetType) extern "C" RetType __attribute__ ((visibility ("default")))
	#endif
#else
	#if defined WIN32
		#define LIB_API(RetType) extern "C" __declspec (dllimport) RetType
	#else
		#define LIB_API(RetType) extern "C" RetType
	#endif
#endif

LIB_API(const char *) get_hello_text ();
#endif // GAMEENGINE_INCLUDE_HPP
