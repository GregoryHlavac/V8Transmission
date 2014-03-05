#pragma once

#include <v8.h>

#include <tuple>
#include <functional>

#include <iostream>
#include <typeinfo>

#include "V8Transmission.h"

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
	struct NativeTypeFactory
	{
		typedef T* ReturnType;

		static ReturnType Construct(const FunctionCallbackInfo<Value>& arguments)
		{
			return new T;
		}

		static void Destruct(ReturnType obj)
		{
			delete obj;
		}

		static std::string* Identifier()
		{
			static std::string* chrName = new std::string(typeid(T).name());

			return chrName;
		}
	};

	template <typename NativeType, typename TypeFactory = NativeTypeFactory<NativeType> >
	struct ClassGear
	{
		static Persistent<FunctionTemplate>	ConstructorTemplate;
		static Persistent<ObjectTemplate>	PrototypeTemplate;

		//typedef typename BaseType		DerivedFrom;
		typedef typename TypeFactory	Factory;

		typedef NativeType* NTPtr;

		static void Initialize(Isolate* iso)
		{
			Local<FunctionTemplate> ctorTemplate = FunctionTemplate::New(iso, ConstructHandle);
			ctorTemplate->SetClassName(v8::String::NewFromUtf8(iso, Factory::Identifier()->c_str()));

			Local<ObjectTemplate> protoTmpl = ctorTemplate->PrototypeTemplate();
			protoTmpl->SetInternalFieldCount(2);

			if (ConstructorTemplate.IsEmpty())
				ConstructorTemplate.Reset(iso, ctorTemplate);
		}


		static void Bind(Isolate* iso, const Handle<ObjectTemplate>& tmpl)
		{
			Local<FunctionTemplate> ctorTemplate = Local<FunctionTemplate>::New(iso, ConstructorTemplate);

			std::cout << "Type bound with identifier: " << std::string(Factory::Identifier()->c_str()) << std::endl;

			tmpl->Set(iso, Factory::Identifier()->c_str(), ctorTemplate);
		}


		static void ConstructHandle(const FunctionCallbackInfo<Value>& arguments)
		{
			NTPtr new_object = Factory::Construct(arguments);

			arguments.GetReturnValue().Set(Wrap(arguments.GetIsolate(), new_object));
		}

		static Handle<Object> Wrap(Isolate* iso, NTPtr native_ptr)
		{
			if (PrototypeTemplate.IsEmpty())
				PrototypeTemplate.Reset(iso, Local<FunctionTemplate>::New(iso, ConstructorTemplate)->PrototypeTemplate());

			Local<ObjectTemplate> tmpl = Local<ObjectTemplate>::New(iso, PrototypeTemplate);
			Handle<Object> result = tmpl->NewInstance();

			Handle<External> internal_ptr = v8::External::New(iso, native_ptr);
			result->SetInternalField(0, internal_ptr);

			Handle<External> type_ptr = v8::External::New(iso, Factory::Identifier());
			result->SetInternalField(1, type_ptr);

			return result;
		}

	};

	template <typename NativeType, typename TypeFactory>
	Persistent<FunctionTemplate> V8Transmission::ClassGear<NativeType, TypeFactory>::ConstructorTemplate;

	template <typename NativeType, typename TypeFactory>
	Persistent<ObjectTemplate> V8Transmission::ClassGear<NativeType, TypeFactory>::PrototypeTemplate;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	///		Void class gear to cut off infinite recursion, and doesn't do anything, acts as an abstract
	///		base class that does nothing.
	/// </summary>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template<>
	struct ClassGear<void> {};


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
				template<class... Expanded>
				static void expand(ThisClass* ptr, ReturnType(ThisClass::*MemberFunction) (Args...), const FunctionCallbackInfo<Value>& args, const Expanded&... expanded)
				{
					Expander<I + 1, N, ThisClass, ReturnType, Args...>::expand(ptr, MemberFunction, args, expanded..., args[I]);
				}
			};

			template <int I, class ThisClass, typename ReturnType, typename... Args>
			struct Expander<I, I, ThisClass, ReturnType, Args...>
			{
				template<class... Expanded>
				static void expand(ThisClass* ptr, ReturnType(ThisClass::*MemberFunction) (Args...), const FunctionCallbackInfo<Value>& args, const Expanded&... expanded)
				{
					(ptr->*MemberFunction)(ConvertFromJS<Args>(args.getIsolate(), expanded)...);
				}
			};
#pragma endregion
		}
	}

	struct Invokable
	{
		static void Invoke(const FunctionCallbackInfo<Value>& args);
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
		static void Invoke(const FunctionCallbackInfo<Value>& args)
		{
			Internal::Convert_Expand_Execute_Raw_Function_Pointer::Expander<0, sizeof...(ArgumentTypes), ReturnType, ArgumentTypes...>::expand(Func, args);
		}
	};



#pragma region Class/Member Related
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
		static void Invoke(const FunctionCallbackInfo<Value>& args)
		{
			ThisClass* this_ptr = ConvertFromJS<ThisClass*>(args.GetIsolate(), args.Holder());

			if (this_ptr)
				Internal::Convert_Expand_Execute_Member_Function_Pointer::Expander<0, sizeof...(ArgumentTypes), ReturnType, ArgumentTypes...>::expand(this_ptr, MemberFunction, args);

			// TODO: Else some sort of error to avoid dereferencing a null pointer.
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
	template <typename ThisClass, typename VariableType, VariableType (ThisClass::*MemberVariable)>
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
			(var->*MemberVariable) = ConvertFromJS<VariableType>(iso, value);
		}
	};
#pragma endregion

}