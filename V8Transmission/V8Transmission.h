#pragma once

#include <v8.h>

#include "Common.h"
#include "TypeConversion.h"
#include "NativeShifts.h"

#include "Gear.h"

namespace V8Transmission
{
	template <typename T>
	v8::Handle<v8::Value> ConvertToJS(v8::Isolate* iso,T v)
	{
		return TypeConversion::ShiftJS<T>()(iso, v);
	}

	template <typename NT>
	NT ConvertFromJS(v8::Isolate* iso, v8::Handle<v8::Value> v)
	{
		return TypeConversion::ShiftNative<NT>()(iso, v);
	}

	template <typename NT>
	bool IsOfType(v8::Isolate* iso, v8::Handle<v8::Value> v)
	{
		return ConvertFromJS<NT>(iso, v) != nullptr;
	}
}