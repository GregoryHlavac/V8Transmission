#pragma once

#include <v8.h>

#include <tuple>
#include <functional>

#include <iostream>
#include <typeinfo>

#include "V8Transmission.h"

namespace V8Transmission
{
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
	class NativeTypeFactory
	{
	public:
		typedef T* ReturnType;

		static ReturnType Construct(const v8::FunctionCallbackInfo<v8::Value> arguments)
		{
			return new T;
		}

		static void Destruct(ReturnType obj)
		{
			delete obj;
		}
	};


	template <typename T>
	class ClassGear
	{
		typedef T Type;
	};



	namespace Internal
	{
		namespace Convert_Expand_Execute_Raw_Function_Pointer
		{
#pragma region Argument Expansion
			template <int I, int N, typename ReturnType, typename... Args>
			struct Expander
			{
				template<class... Expanded>
				static void expand(ReturnType (*NativeFunction)(Args...), const v8::FunctionCallbackInfo<v8::Value>& args, const Expanded&... expanded)
				{
					Expander<I + 1, N, ReturnType, Args...>::expand(NativeFunction, args, expanded..., args[I]);
				}
			};

			template <int I, typename ReturnType, typename... Args>
			struct Expander<I, I, ReturnType, Args...>
			{
				template<class... Expanded>
				static void expand(ReturnType(*NativeFunction)(Args...), const v8::FunctionCallbackInfo<v8::Value>& args, const Expanded&... expanded)
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
				template<class... Expanded>
				static void expand(ThisClass* ptr, ReturnType(ThisClass::*MemberFunction) (Args...), const v8::FunctionCallbackInfo<v8::Value>& args, const Expanded&... expanded)
				{
					Expander<I + 1, N, ThisClass, ReturnType, Args...>::expand(ptr, MemberFunction, args, expanded..., args[I]);
				}
			};

			template <int I, class ThisClass, typename ReturnType, typename... Args>
			struct Expander<I, I, ThisClass, ReturnType, Args...>
			{
				template<class... Expanded>
				static void expand(ThisClass* ptr, ReturnType(ThisClass::*MemberFunction) (Args...), const v8::FunctionCallbackInfo<v8::Value>& args, const Expanded&... expanded)
				{
					(ptr->*MemberFunction)(ConvertFromJS<Args>(args.getIsolate(), expanded)...);
				}
			};
#pragma endregion
		}
	}

	struct Invokable
	{
		static void Invoke(const v8::FunctionCallbackInfo<v8::Value>& args);
	};

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
		template <ReturnType (*Func) (ArgumentTypes...)>
		static void Invoke(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			Internal::Convert_Expand_Execute_Raw_Function_Pointer::Expander<0, sizeof...(ArgumentTypes), ReturnType, ArgumentTypes...>::expand(Func, args);
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
		template <ReturnType (ThisClass::*MemberFunction) (ArgumentTypes...)>
		static void Invoke(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			ThisClass* this_ptr = ConvertFromJS<ThisClass*>(args.GetIsolate(), args.Holder());

			if (this_ptr)
				Internal::Convert_Expand_Execute_Member_Function_Pointer::Expander<0, sizeof...(ArgumentTypes), ReturnType, ArgumentTypes...>::expand(this_ptr, MemberFunction, args);
			
			// TODO: Else some sort of error to avoid dereferencing a null pointer.
		}
	};





#pragma endregion

}