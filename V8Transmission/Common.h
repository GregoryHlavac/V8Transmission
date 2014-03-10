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

namespace V8Transmission
{
	template <bool Condition>
	struct Assertion
	{
		enum { Value = 1 };
	};
	/** Unimplemented - causes a compile-time error if used. */
	template <> struct Assertion<false>;

	typedef v8::Handle<v8::Value> ValueHandle;

	template <typename Type, Type Val>
	struct Static_Option
	{
		static const Type Value = Val;
	};

	template <bool Val>
	struct Boolean_Option : Static_Option<bool, Val> {};

	template <int Val>
	struct Integer_Option : Static_Option<int, Val> {};

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	An object isolation context, in the future all ClassGear related things will require binding
	/// 	themselves to this to allow using multiple isolations and things of that likeness.
	/// 	
	/// 	At least that's the theory, I've yet to look into this stuff yet.
	/// </summary>
	///
	/// <typeparam name="T">	Generic type parameter. </typeparam>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename ForType>
	struct ObjectIsolationContext
	{
		typedef ForType Type;

		v8::Isolate* isolate;
		v8::Persistent<v8::FunctionTemplate>	ConstructorTemplate;
		v8::Persistent<v8::ObjectTemplate>		PrototypeTemplate;
	};

	template <typename T>
	v8::Isolate* V8Transmission::ObjectIsolationContext<T>::isolate;

	template <typename T>
	Persistent<FunctionTemplate> V8Transmission::ObjectIsolationContext<T>::ConstructorTemplate;

	template <typename T>
	Persistent<ObjectTemplate> V8Transmission::ObjectIsolationContext<T>::PrototypeTemplate;


}