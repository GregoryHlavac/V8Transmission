#pragma once

#include <v8.h>

#include "FunctionGears.h"

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
	template <typename NativeType, typename TypeFactory = CO_NativeTypeFactory<NativeType> >
	struct ClassGear
	{
		static Persistent<FunctionTemplate>	ConstructorTemplate;
		static Persistent<ObjectTemplate>	PrototypeTemplate;

		typedef typename TypeFactory	Factory;

		typedef NativeType Type;
		typedef NativeType* TypePtr;

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
			TypePtr new_object = Factory::Construct(arguments);

			arguments.GetReturnValue().Set(Wrap(arguments.GetIsolate(), new_object));
		}

		static Handle<Object> Wrap(Isolate* iso, TypePtr native_ptr)
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
}