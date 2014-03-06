#pragma once

#include <v8.h>

namespace V8Transmission
{
	template <bool Condition>
	struct Assertion
	{
		enum { Value = 1 };
	};
	/** Unimplemented - causes a compile-time error if used. */
	template <> struct Assertion<false>;

	typedef v8::Handle<v8::Value> ValueHandle;

	template <typename Type, Type Val>
	struct Static_Option
	{
		static const Type Value = Val;
	};

	template <bool Val>
	struct Boolean_Option : Static_Option<bool, Val> {};

	template <int Val>
	struct Integer_Option : Static_Option<int, Val> {};
}