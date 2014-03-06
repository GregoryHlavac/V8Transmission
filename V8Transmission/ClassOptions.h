#pragma once

#include "Common.h"

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
	struct CO_NativeTypeFactory
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
}