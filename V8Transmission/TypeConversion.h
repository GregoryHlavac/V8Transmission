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