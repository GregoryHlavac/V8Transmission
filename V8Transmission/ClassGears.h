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

#include "ClassOptions.h"
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

		////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>
		/// 	Initializes the initial templates properly for this ClassGear, if you don't call this before
		/// 	trying to bind any other member function type things, you're going to have some serious issues.
		/// 	
		/// 	I repeat.
		/// 	
		/// 	THIS MUST BE CALLED BEFORE ANY OTHER FUNCTIONS OR TEMPLATES THAT ARE DEPENDENT UPON IT.
		/// </summary>
		///
		/// <param name="iso">	[in,out] If non-null, the ISO. </param>
		////////////////////////////////////////////////////////////////////////////////////////////////////
		static void Initialize(Isolate* iso)
		{
			// If we're enabling constructor we worry about the constructor's FunctionTemplate, otherwise
			// we don't care at all, and we'll just instantiate the prototype template and fill it accordingly.
			if (CO_EnableConstructor<Type>::Value)
			{
				Local<FunctionTemplate> ctorTemplate = FunctionTemplate::New(iso, ConstructorProxy);
				ctorTemplate->SetClassName(v8::String::NewFromUtf8(iso, CO_Identifier<NativeType>::Value()->c_str()));

				if (ConstructorTemplate.IsEmpty())
					ConstructorTemplate.Reset(iso, ctorTemplate);

				Local<ObjectTemplate> protoTmpl = ctorTemplate->PrototypeTemplate();

				// Check if we need to be able to set a second internal field that corresponds to
				// the bound types native 'type' that assists with casting, unsure as of yet how
				// to handle doing this with a derived type casting situation but it'll be investigated
				// at a later date when it is needed.
				if (CO_ExplicitTypeCheck<Type>::Value)
					protoTmpl->SetInternalFieldCount(2);
				else
					protoTmpl->SetInternalFieldCount(1);

				if (PrototypeTemplate.IsEmpty())
					PrototypeTemplate.Reset(iso, protoTmpl);
			}
			else
			{
				Local<ObjectTemplate> protoTmpl = ObjectTemplate::New(iso);

				// Check if we need to be able to set a second internal field that corresponds to
				// the bound types native 'type' that assists with casting, unsure as of yet how
				// to handle doing this with a derived type casting situation but it'll be investigated
				// at a later date when it is needed.
				if (CO_ExplicitTypeCheck<Type>::Value)
					protoTmpl->SetInternalFieldCount(2);
				else
					protoTmpl->SetInternalFieldCount(1);

				if (PrototypeTemplate.IsEmpty())
					PrototypeTemplate.Reset(iso, protoTmpl);
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>
		/// 	Binds the constructor template into the scope of the specified object template, this does
		/// 	nothing as of yet unless the CO_EnableConstructor<T>::Value is true, in the future it may
		/// 	allow statics (and enums) to be bound to the scope of this type's name.
		/// </summary>
		///
		/// <param name="iso"> 	[in,out] If non-null, the ISO. </param>
		/// <param name="tmpl">	The template. </param>
		////////////////////////////////////////////////////////////////////////////////////////////////////
		static void Bind(Isolate* iso, const Handle<ObjectTemplate>& tmpl)
		{
			HandleScope scope(iso);

			// Once again, we don't bind a type if it doesn't have a constructor.
			if (CO_EnableConstructor<Type>::Value)
			{
				Local<FunctionTemplate> ctorTemplate = Local<FunctionTemplate>::New(iso, ConstructorTemplate);
				tmpl->Set(iso, CO_Identifier<NativeType>::Value()->c_str(), ctorTemplate);
			}
		}

		static void ConstructorProxy(const FunctionCallbackInfo<Value>& arguments)
		{
			TypePtr new_object = Factory::Construct(arguments);
			arguments.GetReturnValue().Set(Wrap(arguments.GetIsolate(), new_object));
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>
		/// 	Wraps the native type pointer into a new v8::Object handle and returns it, useful for
		/// 	returning this type after it has been constructed natively back to the JS side.
		/// 	
		/// 	This also can be used even if the CO_EnableConstructor<T> is disabled for this type, as it
		/// 	does not disallow returning it as a reference of this kind of object.
		/// </summary>
		///
		/// <param name="iso">		 	[in,out] If non-null, the ISO. </param>
		/// <param name="native_ptr">	The native pointer. </param>
		///
		/// <returns>
		/// 	A Handle&lt;Object&gt;
		/// </returns>
		////////////////////////////////////////////////////////////////////////////////////////////////////
		static Handle<Object> Wrap(Isolate* iso, TypePtr native_ptr)
		{
			Local<ObjectTemplate> tmpl = Local<ObjectTemplate>::New(iso, PrototypeTemplate);
			Handle<Object> result = tmpl->NewInstance();

			Handle<External> internal_ptr = v8::External::New(iso, native_ptr);
			result->SetInternalField(0, internal_ptr);

			// If we need explicit type checking, set the external correctly.
			if (CO_ExplicitTypeCheck<Type>::Value)
			{
				Handle<External> type_ptr = v8::External::New(iso, CO_Identifier<NativeType>::Value);
				result->SetInternalField(1, type_ptr);
			}

			return result;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>
		/// 	Unwraps the type stored in the v8::Value to the appropriate native type pointer.
		/// 	
		/// 	If enabled this does explicit type checking on the second internal field of the object.
		/// 	If that type check fails, it returns nullptr.
		/// </summary>
		///
		/// <param name="iso">	[in,out] If non-null, the ISO. </param>
		/// <param name="obj">	The object. </param>
		///
		/// <returns>
		/// 	A TypePtr.
		/// </returns>
		////////////////////////////////////////////////////////////////////////////////////////////////////
		static TypePtr Unwrap(Isolate* iso, Handle<Value> obj)
		{
			Handle<External> field = Handle<External>::Cast(obj->ToObject()->GetInternalField(0));

			if (CO_ExplicitTypeCheck<Type>::Value)
			{
				Handle<External> typeField = Handle<External>::Cast(obj->ToObject()->GetInternalField(1));

				std::string* tptr = static_cast<std::string*>(typeField->Value());

				if (tptr != CO_Identifier<Type>::Value())
					return nullptr;
			}


			void* ptr = field->Value();
			return static_cast<TypePtr>(ptr);
		}
	};

	template <typename NativeType, typename TypeFactory>
	Persistent<FunctionTemplate> V8Transmission::ClassGear<NativeType, TypeFactory>::ConstructorTemplate;

	template <typename NativeType, typename TypeFactory>
	Persistent<ObjectTemplate> V8Transmission::ClassGear<NativeType, TypeFactory>::PrototypeTemplate;
}