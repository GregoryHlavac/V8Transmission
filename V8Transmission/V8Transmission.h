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

#include "Common.h"
#include "TypeConversion.h"
#include "NativeShifts.h"

#include "ClassGears.h"
#include "ClassOptions.h"
#include "FunctionGears.h"
#include "VariableGears.h"

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