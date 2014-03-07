////////////////////////////////////////////////////////////////////////////////////////////////////
///	The MIT License (MIT)
///
///	Copyright (c) 2014 Gregory Hlavac
///
///	Permission is hereby granted, free of charge, to any person obtaining a copy
///	of this software and associated documentation files (the "Software"), to deal
///	in the Software without restriction, including without limitation the rights
///	to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
///	copies of the Software, and to permit persons to whom the Software is
///	furnished to do so, subject to the following conditions:
///
///	The above copyright notice and this permission notice shall be included in
///	all copies or substantial portions of the Software.
///
///	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
///	THE SOFTWARE.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <v8.h>
#include <string>

#include "Common.h"

namespace V8Transmission
{
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	A ClassOption that specifies whether or not a class can be constructed without the 'new'
	/// 	keyword on the javascript side, in reality this just enables a sort of behind the scenes
	/// 	auto-redirect to the new operator, if anything its just a shortcut and makes your JS less
	/// 	concise so it isn't recommended to enable this unless you've got a good reason.
	/// </summary>
	///
	/// <typeparam name="T">	Generic type parameter. </typeparam>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename T>
	struct CO_AllowConstructorWithoutNew : Boolean_Option<false> {};

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	A ClassOption to enable a primitive form of garbage collection for a class to track javascript
	/// 	side classes that were instantiated and linked to a native class.
	/// 	
	/// 	This isn't implemented yet, but will likely utilize std::shared_ptr and weak_ptr.
	/// </summary>
	///
	/// <typeparam name="T">	Generic type parameter. </typeparam>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename T>
	struct CO_EnableSmartPointerGC : Boolean_Option<false> {};

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	A ClassOption to force checking of the type-identifier stored in the second internal pointer
	/// 	field of an object, this relies upon the CO_Identifier<T> of that class to check if the object
	/// 	being marshalled to is the same as the object that it actually is.
	/// </summary>
	///
	/// <typeparam name="T">	Generic type parameter. </typeparam>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename T>
	struct CO_ExplicitTypeCheck : Boolean_Option<false> {};

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	A ClassOption to enable constructing this object, keep in mind this does not disqualify you
	/// 	from using this value in V8 if you disable this, but it will mean it will have to be returned
	/// 	from some other function call or as a global of some sort.
	/// </summary>
	///
	/// <typeparam name="T">	Generic type parameter. </typeparam>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename T>
	struct CO_EnableConstructor : Boolean_Option<true> {};

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	A ClassOption to specify a type's 'Identifier' explicitly.
	/// 	
	/// 	If left to default however this uses typeid(T).name which isn't always accurate depending
	/// 	on the compiler you're using so it is best recommended to always specialize this for types
	/// 	that you are wrapping, unless you don't need to construct it.
	/// 	
	/// 	
	/// 	This identifier is used for..
	/// 	- Constructor/Object Tag (new ...();)  
	/// 	- Internal Type Pointer (if enabled with CO_ExplicitTypeCheck<T>)  
	/// 	- V8 Class-Name on Prototype.
	/// </summary>
	///
	/// <typeparam name="T">	Generic type parameter. </typeparam>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename T>
	struct CO_Identifier
	{
		static std::string* Value()
		{
			static std::string* id = new std::string(typeid(T).name);

			return id;
		}
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	///		Base policy template used by the wrapper to create objects from JS arguments.
	///		
	///		By default this only calls the no argument constructor of the type.
	/// </summary>
	///
	/// <typeparam name="T">	Generic type parameter. </typeparam>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename T>
	struct CO_NativeTypeFactory
	{
		typedef T* ReturnType;

		static ReturnType Construct(const v8::FunctionCallbackInfo<v8::Value>& arguments)
		{
			return new T;
		}

		static void Destruct(ReturnType obj)
		{
			delete obj;
		}
	};
}