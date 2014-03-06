#pragma once

#include <v8.h>

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
		static void Getter(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
		{
			ThisClass* var = ConvertFromJS<ThisClass*>(info.GetIsolate(), info.Holder());
			info.GetReturnValue().Set(ConvertToJS<VariableType>(info.GetIsolate(), (var->*MemberVariable)));
		}
		static void Setter(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
		{
			ThisClass* var = ConvertFromJS<ThisClass*>(info.GetIsolate(), info.Holder());
			(var->*MemberVariable) = ConvertFromJS<VariableType>(info.GetIsolate(), value);
		}
	};
}