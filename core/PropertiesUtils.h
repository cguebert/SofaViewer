#pragma once

#include "Property.h"
#include "VectorWrapper.h"

#include <array>
#include <vector>

namespace property
{

	namespace details
	{

	template <class T>
	struct ArrayTraits
	{
		static const bool isArray = false;
		static const bool fixed = true;
		static const int size = 1;
		using value_type = T;
	};

	template <class T, int N>
	struct ArrayTraits<std::array<T, N>>
	{
		static const bool isArray = true;
		static const bool fixed = true;
		static const int size = N;
		using value_type = T;
	};

	template <class T>
	struct ArrayTraits<std::vector<T>>
	{
		static const bool isArray = true;
		static const bool fixed = false;
		static const int size = 1;
		using value_type = T;
	};

	template <class T>
	struct SingleValueCreator
	{
		template <class U>
		static Property::ValuePtr create(U&& val)
		{
			return std::make_shared<PropertyValueCopy<T>>(std::forward<U>(val));
		}
	};

	template <class T>
	struct SingleArrayCreator
	{
		template <class U>
		static Property::ValuePtr create(U&& val)
		{
			using base_type = ArrayTraits<T>::value_type;
			const int size = ArrayTraits<T>::size;
			std::vector<base_type> copyVal(std::begin(val), std::end(val));

			using WrapperType = VectorWrapper<std::vector<base_type>>;
			WrapperType wrapper(std::move(copyVal));
			wrapper.setColumnCount(size);
			wrapper.setFixedSize(true);

			return std::make_shared<PropertyValueCopy<WrapperType>>(std::move(wrapper));
		}
	};

	template <class T>
	struct DoubleArrayCreator
	{
		template <class U>
		static Property::ValuePtr create(U&& val)
		{
			using array_type = ArrayTraits<T>::value_type;
			const int size0 = ArrayTraits<T>::size;

			using base_type = ArrayTraits<array_type>::value_type;
			const int size1 = ArrayTraits<array_type>::size;

			std::vector<base_type> copyVal(size0 * size1);
			for (int i = 0; i < size0; ++i)
				for (int j = 0; j < size1; ++j)
					copyVal[i*size1 + j] = val[i][j];

			using WrapperType = VectorWrapper<std::vector<base_type>>;
			WrapperType wrapper(std::move(copyVal));
			wrapper.setColumnCount(size1);
			wrapper.setFixedSize(true);

			return std::make_shared<PropertyValueCopy<WrapperType>>(std::move(wrapper));
		}
	};

	template <class T>
	struct ArraysVectorCreator
	{
		template <class U>
		static Property::ValuePtr create(U&& val)
		{
			using array_type = ArrayTraits<T>::value_type;
			using base_type = ArrayTraits<array_type>::value_type;
			const int size = ArrayTraits<array_type>::size;
			const int nb = std::distance(std::begin(val), std::end(val));
			std::vector<base_type> copyVal(nb * size);
			for (int i = 0; i < nb; ++i)
			{
				for (int j = 0; j < size; ++j)
					copyVal[i*size + j] = val[i][j];
			}

			using WrapperType = VectorWrapper<std::vector<base_type>>;
			WrapperType wrapper(std::move(copyVal));
			wrapper.setColumnCount(size);

			return std::make_shared<PropertyValueCopy<WrapperType>>(std::move(wrapper));
		}
	};

	template <class T, bool extend0, bool extend1, bool fixed>
	struct ValueCopyCreator;

	template <class T, bool fixed>
	struct ValueCopyCreator<T, false, false, fixed> : SingleValueCreator<T>{};

	template <class T>
	struct ValueCopyCreator<T, true, false, false> : SingleValueCreator<T>{}; // Vector of single values

	template <class T>
	struct ValueCopyCreator<T, true, false, true> : SingleArrayCreator<T>{};

	template <class T>
	struct ValueCopyCreator<T, true, true, false> : ArraysVectorCreator<T>{};

	template <class T>
	struct ValueCopyCreator<T, true, true, true> : DoubleArrayCreator<T>{};

	template <class T>
	Property::ValuePtr createValueCopy(T&& val)
	{
		using value_type = std::remove_cv_t<std::remove_reference_t<T>>;
		const bool extend0 = details::ArrayTraits<value_type>::isArray;
		const bool fixed = details::ArrayTraits<value_type>::fixed;
		using base_type = details::ArrayTraits<value_type>::value_type;
		const bool extend1 = details::ArrayTraits<base_type>::isArray;
		return ValueCopyCreator<value_type, extend0, extend1, fixed>::create(std::forward<T>(val));
	}

	} // namespace details

	template <class T>
	Property::PropertyPtr createCopyProperty(const std::string& name, T&& val)
	{
		auto value = details::createValueCopy(std::forward<T>(val));
		return std::make_shared<Property>(name, value);
	}

	template <class T>
	Property::PropertyPtr createRefProperty(const std::string& name, T& val)
	{
		auto value = std::make_shared<PropertyValueRef<T>>(val);
		return std::make_shared<Property>(name, value);
	}

}
