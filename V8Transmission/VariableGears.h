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

using v8::Local;
using v8::String;
using v8::ObjectTemplate;

#include "ClassGears.h"

namespace V8Transmission
{
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	A static variable gear.
	/// </summary>
	///
	/// <typeparam name="ValueType">				 	Type of the value type. </typeparam>
	/// <typeparam name="ValueType(*StaticVariable)">	Type of the value type(* static variable) </typeparam>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename ValueType, ValueType(*StaticVariable)>
	struct StaticVariableGear
	{
		static void Getter(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
		{
			info.GetReturnValue().Set(ConvertToJS<ValueType>(info.GetIsolate(), (*StaticVariable)));
		}
		static void Setter(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
		{
			(*StaticVariable) = ConvertFromJS<ValueType>(info.GetIsolate(), value);
		}
	};


	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	A member variable gear.
	/// </summary>
	///
	/// <typeparam name="ThisClass">				   	Type of this class. </typeparam>
	/// <typeparam name="VariableType">				   	Type of the variable type. </typeparam>
	/// <typeparam name="(ThisClass::*MemberVariable)">	Type of this class * member variable) </typeparam>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename ThisClass, typename VariableType, VariableType(ThisClass::*MemberVariable)>
	struct MemberVariableGear
	{
		typedef typename ClassGear<ThisClass>	CG;

		static void Getter(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
		{
			ThisClass* var = CG::Unwrap(info.GetIsolate(), info.Holder());
			info.GetReturnValue().Set(ConvertToJS<VariableType>(info.GetIsolate(), (var->*MemberVariable)));
		}
		static void Setter(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
		{
			ThisClass* var = CG::Unwrap(info.GetIsolate(), info.Holder());
			(var->*MemberVariable) = ConvertFromJS<VariableType>(info.GetIsolate(), value);
		}

		static void BindRW(Isolate* iso, const char* name)
		{
			Local<ObjectTemplate> protoTmpl = Local<ObjectTemplate>::New(iso, ClassGear<ThisClass>::PrototypeTemplate);
			protoTmpl->SetAccessor(String::NewFromUtf8(iso, name), Getter, Setter);
		}

		static void BindRO(Isolate* iso, const char* name)
		{
			Local<ObjectTemplate> protoTmpl = Local<ObjectTemplate>::New(iso, ClassGear<ThisClass>::PrototypeTemplate);
			protoTmpl->SetAccessor(String::NewFromUtf8(iso, name), Getter);
		}
	};
}