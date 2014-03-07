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

#include <iostream>

#include "Common.h"
#include "TypeConversion.h"

namespace V8Transmission
{
	namespace TypeConversion
	{

#pragma region Numeric Specializations
		template <> struct ShiftJS<unsigned char> : Internal::ShiftJS_Unsigned_Integer_Small<unsigned char>{};

		template <> struct ShiftJS<int16_t> : Internal::ShiftJS_Integer_Small<int16_t>{};

		template <> struct ShiftJS<uint16_t> : Internal::ShiftJS_Unsigned_Integer_Small<uint16_t>{};

		template <> struct ShiftJS<int32_t> : Internal::ShiftJS_Integer_Small<int32_t>{};

		template <> struct ShiftJS<uint32_t> : Internal::ShiftJS_Unsigned_Integer_Small<uint32_t>{};

		template <> struct ShiftJS<int64_t> : Internal::ShiftJS_Integer_Large<int64_t> {};

		template <> struct ShiftJS<uint64_t> : Internal::ShiftJS_Integer_Large<uint64_t> {};

		template <>
		struct ShiftJS<float>
		{
			ValueHandle operator()(v8::Isolate* iso, float v) const
			{
				return v8::Number::New(iso, static_cast<double>(v));
			}
		};

		template <>
		struct ShiftJS<double>
		{
			ValueHandle operator()(v8::Isolate* iso, double v) const
			{
				return v8::Number::New(iso, v);
			}
		};

		template <>
		struct ShiftJS<bool>
		{
			ValueHandle operator()(v8::Isolate* iso, bool v) const
			{
				return v8::Boolean::New(iso, v);
			}
		};

		template <>
		struct ShiftJS<std::string>
		{
			ValueHandle operator()(v8::Isolate* iso, std::string v) const
			{
				return v8::String::NewFromUtf8(iso, v.c_str(), v8::String::kNormalString, v.size());
			}
		};
#pragma endregion

#pragma region For Lazy People
		static const ShiftJS<int16_t> Int16ToJS = ShiftJS<int16_t>();
		static const ShiftJS<uint16_t> UInt16ToJS = ShiftJS<uint16_t>();
		static const ShiftJS<int32_t> Int32ToJS = ShiftJS<int32_t>();
		static const ShiftJS<uint32_t> UInt32ToJS = ShiftJS<uint32_t>();
		static const ShiftJS<int64_t> Int64ToJS = ShiftJS<int64_t>();
		static const ShiftJS<uint64_t> UInt64ToJS = ShiftJS<uint64_t>();
		static const ShiftJS<float>		FloatToJS = ShiftJS<float>();
		static const ShiftJS<double> DoubleToJS = ShiftJS<double>();
		static const ShiftJS<bool> BoolToJS = ShiftJS<bool>();
		static const ShiftJS<std::string> StdStringToJS = ShiftJS<std::string>();
#pragma endregion



#pragma region Shift to Native Type
		template<>
		struct ShiftNative<std::string>
		{
			std::string operator()(v8::Isolate* iso, const v8::Handle<v8::Value>& val) const
			{
				return std::string(*v8::String::Utf8Value(val));
			}
		};
#pragma endregion Shift to Native Type
	}
}