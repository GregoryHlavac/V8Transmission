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

	// Because who wants to type this over and over and over again?
	typedef v8::Handle<v8::Value> ValueHandle;
}