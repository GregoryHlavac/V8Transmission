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

using v8::Value;
using v8::Local;
using v8::Handle;
using v8::Object;
using v8::Isolate;
using v8::External;
using v8::Persistent;
using v8::ObjectTemplate;
using v8::FunctionTemplate;
using v8::FunctionCallbackInfo;

namespace V8Transmission
{
	namespace Internal
	{
		namespace Convert_Expand_Execute_Raw_Function_Pointer
		{
#pragma region Argument Expansion
			template <int I, int N, typename ReturnType, typename... Args>
			struct Expander
			{
				template<class... Expanded>
				static void expand(ReturnType(*NativeFunction)(Args...), const FunctionCallbackInfo<Value>& args, const Expanded&... expanded)
				{
					Expander<I + 1, N, ReturnType, Args...>::expand(NativeFunction, args, expanded..., args[I]);
				}
			};

			template <int I, typename ReturnType, typename... Args>
			struct Expander<I, I, ReturnType, Args...>
			{
				template<class... Expanded>
				static void expand(ReturnType(*NativeFunction)(Args...), const FunctionCallbackInfo<Value>& args, const Expanded&... expanded)
				{
					NativeFunction(ConvertFromJS<Args>(args.GetIsolate(), expanded)...);
				}
			};
#pragma endregion
		}


		namespace Convert_Expand_Execute_Member_Function_Pointer
		{
#pragma region Argument Expansion
			template <int I, int N, class ThisClass, typename ReturnType, typename... Args>
			struct Expander
			{
				typedef ReturnType(ThisClass::*MemberFunctionPtr)(Args...);

				template<class... Expanded>
				static void expand(ThisClass* ptr, MemberFunctionPtr mfptr, const FunctionCallbackInfo<Value>& args, const Expanded&... expanded)
				{
					Expander<I + 1, N, ThisClass, ReturnType, Args...>::expand(ptr, mfptr, args, expanded..., args[I]);
				}
			};

			template <int I, class ThisClass, typename ReturnType, typename... Args>
			struct Expander<I, I, ThisClass, ReturnType, Args...>
			{
				typedef ReturnType(ThisClass::*MemberFunctionPtr)(Args...);

				template<class... Expanded>
				static void expand(ThisClass* ptr, MemberFunctionPtr mfptr, const FunctionCallbackInfo<Value>& args, const Expanded&... expanded)
				{
					(ptr->*mfptr)(ConvertFromJS<Args>(args.GetIsolate(), expanded)...);
				}
			};
#pragma endregion
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	A static function gear.
	/// 	
	/// 	Used to bind an arbitrary static or global function to javascript.
	/// 	
	/// 	Example
	/// 	
	/// 	int sum(int x, int y) { return x+y; }
	/// 	
	/// 	...
	/// 	global->Set(String::NewFromUtf8(isolate, "sum"), FunctionTemplate::New(isolate, StaticFunctionGear<int, int, int>::Invoke<sum>));
	///		...
	/// 
	/// </summary>
	///
	/// <typeparam name="ReturnType">   	Type of the return type. </typeparam>
	/// <typeparam name="ArgumentTypes">	Type of the argument types. </typeparam>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename ReturnType, typename... ArgumentTypes>
	struct StaticFunctionGear
	{
		typedef ReturnType(*StaticFunctionPtr) (ArgumentTypes...);

		template <StaticFunctionPtr sfptr>
		static void Invoke(const FunctionCallbackInfo<Value>& args)
		{
			Internal::Convert_Expand_Execute_Raw_Function_Pointer::Expander<0, sizeof...(ArgumentTypes), ReturnType, ArgumentTypes...>::expand(sfptr, args);
		}
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	A member function gear.
	/// 	
	/// 	Used to bind member functions on the specified class type.
	/// </summary>
	///
	/// <typeparam name="ThisClass">		Type of this class. </typeparam>
	/// <typeparam name="ReturnType">   	Type of the return type. </typeparam>
	/// <typeparam name="ArgumentTypes">	Type of the argument types. </typeparam>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class ThisClass, typename ReturnType, typename... ArgumentTypes>
	struct MemberFunctionGear
	{
		typedef ReturnType(ThisClass::*MemberFunctionPtr) (ArgumentTypes...);

		template <MemberFunctionPtr mfptr>
		static void Invoke(const FunctionCallbackInfo<Value>& args)
		{
			ThisClass* this_ptr = ConvertFromJS<ThisClass*>(args.GetIsolate(), args.Holder());

			if (this_ptr)
				Internal::Convert_Expand_Execute_Member_Function_Pointer::Expander<0, sizeof...(ArgumentTypes), ThisClass, ReturnType, ArgumentTypes...>::expand(this_ptr, mfptr, args);

			// TODO: Else some sort of error to avoid dereferencing a null pointer.
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>
		/// 	Binds the specified member function to the associated ClassGear. However that ClassGear must've
		/// 	invoked ClassGear<T>::Initialize(..) first or there will be an empty handle error thrown by V8.
		/// </summary>
		///
		/// <typeparam name="mfptr">	Type of the mfptr. </typeparam>
		/// <param name="iso"> 	[in,out] If non-null, the ISO. </param>
		/// <param name="name">	The name. </param>
		////////////////////////////////////////////////////////////////////////////////////////////////////
		template <MemberFunctionPtr mfptr>
		static void Bind(Isolate* iso, const char* name)
		{
			Local<ObjectTemplate> protoTmpl = Local<ObjectTemplate>::New(iso, ClassGear<ThisClass>::PrototypeTemplate);
			Local<FunctionTemplate> lft = FunctionTemplate::New(iso, Invoke<mfptr>);

			protoTmpl->Set(String::NewFromUtf8(iso, name), lft);
		}
	};
}