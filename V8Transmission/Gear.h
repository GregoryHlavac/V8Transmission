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
		namespace Convert_Expand_Execute
		{
#pragma region Argument Expansion
			template <int I, int N, typename ReturnType, typename... Args>
			struct Expander
			{
				template<class... Expanded>
				static void expand(std::function<ReturnType(Args...)>& NativeFunction, const v8::FunctionCallbackInfo<v8::Value>& args, const Expanded&... expanded)
				{
					Expander<I + 1, N, ReturnType, Args...>::expand(NativeFunction, args, expanded..., args[I]);
				}
			};

			template <int I, typename ReturnType, typename... Args>
			struct Expander<I, I, ReturnType, Args...>
			{
				template<class... Expanded>
				static void expand(const std::function<ReturnType(Args...)>& NativeFunction, const v8::FunctionCallbackInfo<v8::Value>& args, const Expanded&... expanded)
				{
					NativeFunction(ConvertFromJS<Args>(args.GetIsolate(), expanded)...);
				}
			};
#pragma endregion
		}


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
	}

	struct Invokable
	{
		static void Invoke(const v8::FunctionCallbackInfo<v8::Value>& args);
	};


	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	A function gear.
	/// 	
	/// 	This is used to bind a JS executable target to some code on the native side, as of yet it
	/// 	lacks any sort of error checking, so don't really go throwing this thing around willy-nilly
	/// 	without being sure that the stuff you're pulling from JS side are the types, that they're supposed
	/// 	to be (Or at the very least to be cast-able).
	/// 	
	/// 	
	/// 	Props to BD-Calvin and SlashLife in C++ channel on QuakeNet for helping me through this obnoxious bit.
	/// </summary>
	///
	/// <typeparam name="ReturnType">   	Type of the return type. </typeparam>
	/// <typeparam name="ArgumentTypes">	Type of the argument types. </typeparam>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename ReturnType, typename... ArgumentTypes>
	class FunctionGear 
	{
		typedef std::function<ReturnType(ArgumentTypes...)> NativeFunction;

	public:
		FunctionGear(NativeFunction func) : mNativeFunction(func) { }

 		ReturnType Invoke(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			return Internal::Convert_Expand_Execute::Expander<0, sizeof...(ArgumentTypes), ReturnType, ArgumentTypes...>::expand(mNativeFunction, args);
		}

	private:
		NativeFunction mNativeFunction;
	};


	template < typename ReturnType, typename... ArgumentTypes>
	struct StaticFunctionGear
	{
		template <ReturnType (*Func) (ArgumentTypes...)>
		//template <std::function<ReturnType(ArgumentTypes...)> Func>
		static void Invoke(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			Internal::Convert_Expand_Execute_Raw_Function_Pointer::Expander<0, sizeof...(ArgumentTypes), ReturnType, ArgumentTypes...>::expand(Func, args);
		}
	};





#pragma endregion

}