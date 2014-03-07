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

#include <stdint.h>
#include <string>
#include <vector>

#include "Common.h"

namespace V8Transmission
{
	namespace TypeConversion
	{
		template <typename NativeType>
		struct ShiftJS
		{
			template <typename X>
			ValueHandle operator()(v8::Isolate*, X const &) const;
		};

		template <typename NativeType>
		struct ShiftJS<NativeType *> : ShiftJS<NativeType>{};



		template <typename NT>
		struct ShiftNative
		{
			NT operator()(v8::Isolate*, v8::Handle<v8::Value> const &) const;
		};




#if !defined(DOXYGEN)
		namespace Internal
		{
			template <typename IntegralType>
			struct ShiftJS_Integer_Small
			{
				ValueHandle operator()(v8::Isolate* iso, IntegralType v) const
				{
					return v8::Integer::New(iso, static_cast<int32_t>(v));
				}

			private:
				static_assert(std::is_integral<IntegralType>::value, "Type used was not an integral type.");
			};

			template <typename IntegralType>
			struct ShiftJS_Unsigned_Integer_Small
			{
				ValueHandle operator()(v8::Isolate* iso, IntegralType v) const
				{
					return v8::Integer::NewFromUnsigned(iso, static_cast<uint32_t>(v));
				}

			private:
				static_assert(std::is_integral<IntegralType>::value, "Type used was not an integral type.");
				static_assert(std::is_unsigned<IntegralType>::value, "Type used was not an unsigned type.");
			};

			template <typename IntegralType>
			struct ShiftJS_Integer_Large
			{
				/** Returns v as a double value. */
				v8::Handle<v8::Value> operator()(v8::Isolate* iso, IntegralType v) const
				{
					return v8::Number::New(iso, static_cast<double>(v));
				}
			};
		}
#endif

	}
}