#pragma once

#include <v8.h>

#include <tuple>
#include <functional>

#include <iostream>
#include <typeinfo>

#include "V8Transmission.h"

#include "Common.h"

#include "FunctionGears.h"
#include "VariableGears.h"

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
			ctorTemplate->SetClassName(v8::String::NewFromUtf8(iso, CO_Identifier<NativeType>::Value()->c_str()));

			Local<ObjectTemplate> protoTmpl = ctorTemplate->PrototypeTemplate();
			protoTmpl->SetInternalFieldCount(2);

			if (ConstructorTemplate.IsEmpty())
				ConstructorTemplate.Reset(iso, ctorTemplate);

			if (PrototypeTemplate.IsEmpty())
				PrototypeTemplate.Reset(iso, protoTmpl);
		}


		static void Bind(Isolate* iso, const Handle<ObjectTemplate>& tmpl)
		{
			HandleScope scope(iso);

			Local<FunctionTemplate> ctorTemplate = Local<FunctionTemplate>::New(iso, ConstructorTemplate);

			tmpl->Set(iso, CO_Identifier<NativeType>::Value()->c_str(), ctorTemplate);
		}

		template <typename ReturnType, typename... ArgumentTypes>
		static void BindMemberFunction(Isolate* iso, const char* fncName, ReturnType(NativeType::*MemberFunction) (ArgumentTypes...))
		{
			Local<ObjectTemplate> protoTmpl = Local<ObjectTemplate>::New(iso, PrototypeTemplate);
			Local<FunctionTemplate> lft = FunctionTemplate::New(iso, MemberFunctionGear<NativeType, ReturnType, ArgumentTypes...>::Invoke<MemberFunction>);

			protoTmpl->Set(String::NewFromUtf8(iso, fncName), lft);
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

			Handle<External> type_ptr = v8::External::New(iso, CO_Identifier<NativeType>::Value);
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
}