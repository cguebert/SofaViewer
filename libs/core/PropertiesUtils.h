#pragma once

#include "Property.h"
#include "MetaProperties.h"
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
	using ArrayBaseType = typename ArrayTraits<T>::value_type;

	template <class T>
	using CopyVectorType = std::vector<ArrayBaseType<ArrayBaseType<T>>>;

	//****************************************************************************//

	template <class T>
	struct SimpleCopier
	{
		static void toVector(const T& val, T& vec) { vec = val; }
		static void fromVector(const T& vec, T& val) { val = vec; }
	};

	template <class T>
	struct ArrayCopier
	{
		static void toVector(const T& val, CopyVectorType<T>& vec) 
		{ std::copy(std::begin(val), std::end(val), std::begin(vec), std::end(vec)); }
		static void fromVector(const CopyVectorType<T>& vec, T& val) 
		{ std::copy(std::begin(vec), std::end(vec), std::begin(val), std::end(val)); }
	};

	template <class T>
	struct DoubleArrayCopier
	{
		static void toVector(const T& val, CopyVectorType<T>& vec)
		{
			using array_type = ArrayBaseType<T>;
			const int size0 = ArrayTraits<T>::size;
			const int size1 = ArrayTraits<array_type>::size;

			for (int i = 0; i < size0; ++i)
				for (int j = 0; j < size1; ++j)
					vec[i*size1 + j] = val[i][j];
		}
		static void fromVector(const CopyVectorType<T>& vec, T& val)
		{
			using array_type = ArrayBaseType<T>;
			const int size0 = ArrayTraits<T>::size;
			const int size1 = ArrayTraits<array_type>::size;

			for (int i = 0; i < size0; ++i)
				for (int j = 0; j < size1; ++j)
					val[i][j] = vec[i*size1 + j];
		}
	};

	template <class T>
	struct ArraysVectorCopier
	{
		static void toVector(const T& val, CopyVectorType<T>& vec)
		{
			using array_type = ArrayBaseType<T>;
			using base_type = ArrayBaseType<array_type>;
			const int size = ArrayTraits<array_type>::size;
			const int nb = std::distance(std::begin(val), std::end(val));

			for (int i = 0; i < nb; ++i)
			{
				for (int j = 0; j < size; ++j)
					vec[i*size + j] = val[i][j];
			}
		}
		static void fromVector(const CopyVectorType<T>& vec, T& val)
		{
			using array_type = ArrayBaseType<T>;
			using base_type = ArrayBaseType<array_type>;
			const int size = ArrayTraits<array_type>::size;
			const int nb = std::distance(std::begin(val), std::end(val));

			for (int i = 0; i < nb; ++i)
			{
				for (int j = 0; j < size; ++j)
					val[i][j] = vec[i*size + j];
			}
		}
	};

	template <class T, bool extend0, bool extend1, bool fixed>
	struct ValueCopier;

	template <class T, bool fixed>
	struct ValueCopier<T, false, false, fixed> : SimpleCopier<T>{};

	template <class T>
	struct ValueCopier<T, true, false, false> : SimpleCopier<T>{};

	template <class T>
	struct ValueCopier<T, true, false, true> : ArrayCopier<T>{};

	template <class T>
	struct ValueCopier<T, true, true, false> : ArraysVectorCopier<T>{};

	template <class T>
	struct ValueCopier<T, true, true, true> : DoubleArrayCopier<T>{};

	//****************************************************************************//

	template <class T>
	struct SingleValueCreator
	{
		using PropertyValueType = T;

		template <class U>
		static Property::ValuePtr create(U&& val)
		{
			return std::make_shared<PropertyCopyValue<T>>(std::forward<U>(val));
		}
	};

	template <class T>
	struct SingleArrayCreator
	{
		using PropertyValueType = VectorWrapper<CopyVectorType<T>>;

		template <class U>
		static Property::ValuePtr create(U&& val)
		{
			using base_type = ArrayBaseType<T>;
			const int size = ArrayTraits<T>::size;
			using copy_vector_type = CopyVectorType<T>;
			copy_vector_type copyVal(std::begin(val), std::end(val));

			using WrapperType = VectorWrapper<copy_vector_type>;
			WrapperType wrapper(std::move(copyVal));
			wrapper.setColumnCount(size);
			wrapper.setFixedSize(true);

			return std::make_shared<PropertyCopyValue<WrapperType>>(std::move(wrapper));
		}
	};

	template <class T>
	struct DoubleArrayCreator
	{
		using PropertyValueType = VectorWrapper<CopyVectorType<T>>;

		template <class U>
		static Property::ValuePtr create(U&& val)
		{
			using array_type = ArrayBaseType<T>;
			const int size0 = ArrayTraits<T>::size;

			using base_type = ArrayBaseType<array_type>;
			const int size1 = ArrayTraits<array_type>::size;

			using copy_vector_type = CopyVectorType<T>;
			copy_vector_type copyVal(size0 * size1);
			DoubleArrayCopier<T>::toVector(val, copyVal);

			using WrapperType = VectorWrapper<copy_vector_type>;
			WrapperType wrapper(std::move(copyVal));
			wrapper.setColumnCount(size1);
			wrapper.setFixedSize(true);

			return std::make_shared<PropertyCopyValue<WrapperType>>(std::move(wrapper));
		}
	};

	template <class T>
	struct ArraysVectorCreator
	{
		using PropertyValueType = VectorWrapper<CopyVectorType<T>>;

		template <class U>
		static Property::ValuePtr create(U&& val)
		{
			using array_type = ArrayBaseType<T>;
			using base_type = ArrayBaseType<array_type>;
			const int size = ArrayTraits<array_type>::size;
			const int nb = std::distance(std::begin(val), std::end(val));

			using copy_vector_type = CopyVectorType<T>;
			copy_vector_type copyVal(nb * size);
			ArraysVectorCopier<T>::toVector(val, copyVal);

			using WrapperType = VectorWrapper<copy_vector_type>;
			WrapperType wrapper(std::move(copyVal));
			wrapper.setColumnCount(size);

			return std::make_shared<PropertyCopyValue<WrapperType>>(std::move(wrapper));
		}
	};

	template <class T, bool extend0, bool extend1, bool fixed>
	struct CopyValueCreator;

	template <class T, bool fixed>
	struct CopyValueCreator<T, false, false, fixed> : SingleValueCreator<T>{}; // Single values and VectorWrapper

	template <class T>
	struct CopyValueCreator<T, true, false, false> : SingleValueCreator<T>{}; // Vector of single values

	template <class T>
	struct CopyValueCreator<T, true, false, true> : SingleArrayCreator<T>{};

	template <class T>
	struct CopyValueCreator<T, true, true, false> : ArraysVectorCreator<T>{};

	template <class T>
	struct CopyValueCreator<T, true, true, true> : DoubleArrayCreator<T>{};

	//****************************************************************************//

	template <class T>
	void copyToVector(const T& from, T& to) { to = from; }

	template <class T>
	void copyToVector(const T& val, details::CopyVectorType<T>& vec)
	{
		using value_type = std::remove_cv_t<std::remove_reference_t<T>>;
		const bool extend0 = ArrayTraits<value_type>::isArray;
		const bool fixed = ArrayTraits<value_type>::fixed;
		using base_type = ArrayTraits<value_type>::value_type;
		const bool extend1 = ArrayTraits<base_type>::isArray;
		ValueCopier<value_type, extend0, extend1, fixed>::toVector(val, vec);
	}

	template <class T> 
	T& value(T& val) { return val; }

	template <class T>
	T& value(VectorWrapper<T>& val) { return val.value(); }

	template <class T>
	void copyFromVector(const T& from, T& to) { to = from; }

	template <class T>
	void copyFromVector(const details::CopyVectorType<T>& vec, T& val)
	{
		using value_type = std::remove_cv_t<std::remove_reference_t<T>>;
		const bool extend0 = ArrayTraits<value_type>::isArray;
		const bool fixed = ArrayTraits<value_type>::fixed;
		using base_type = ArrayTraits<value_type>::value_type;
		const bool extend1 = ArrayTraits<base_type>::isArray;
		ValueCopier<value_type, extend0, extend1, fixed>::fromVector(vec, val);
	}

	//****************************************************************************//

	template <class T>
	class PropertyValueType
	{
	private:
		using value_type = std::remove_cv_t<std::remove_reference_t<T>>;
		static const bool extend0 = ArrayTraits<value_type>::isArray;
		static const bool fixed = ArrayTraits<value_type>::fixed;
		using base_type = ArrayBaseType<value_type>;
		static const bool extend1 = ArrayTraits<base_type>::isArray;
		
	public:
		using property_type = typename CopyValueCreator<value_type, extend0, extend1, fixed>::PropertyValueType;
	};

	template <class T>
	Property::ValuePtr createValueCopy(T&& val)
	{
		using value_type = std::remove_cv_t<std::remove_reference_t<T>>;
		const bool extend0 = ArrayTraits<value_type>::isArray;
		const bool fixed = ArrayTraits<value_type>::fixed;
		using base_type = ArrayBaseType<value_type>;
		const bool extend1 = ArrayTraits<base_type>::isArray;
		return CopyValueCreator<value_type, extend0, extend1, fixed>::create(std::forward<T>(val));
	}

	} // namespace details

	//****************************************************************************//

	template <class T>
	Property::ValuePtr createCopyValue(T&& val)
	{
		return details::createValueCopy(std::forward<T>(val));
	}

	template <class T, class... MetaArgs>
	Property::ValuePtr createCopyValue(T&& val, MetaArgs&&... meta)
	{
		auto valuePtr = details::createValueCopy(std::forward<T>(val));
		using value_type = details::PropertyValueType<T>::property_type;
		auto& metaContainer = dynamic_cast<meta::MetaContainer<value_type>&>(valuePtr->baseMetaContainer());
		metaContainer.add(std::forward<MetaArgs>(meta)...);
		return valuePtr;
	}

	template <class T>
	Property::ValuePtr createRefValue(T& val)
	{
		return std::make_shared<PropertyRefValue<T>>(val);
	}

	template <class T, class... MetaArgs>
	Property::ValuePtr createRefValue(T& val, MetaArgs&&... meta)
	{
		auto valuePtr = std::make_shared<PropertyRefValue<T>>(val);
		valuePtr->metaContainer().add(std::forward<MetaArgs>(meta)...);
		return valuePtr;
	}

	//****************************************************************************//

	template <class T>
	Property::SPtr createCopyProperty(const std::string& name, T&& val)
	{
		auto value = createCopyValue(std::forward<T>(val));
		return std::make_shared<Property>(name, value);
	}

	template <class T, class... MetaArgs>
	Property::SPtr createCopyProperty(const std::string& name, T&& val, MetaArgs&&... meta)
	{
		auto value = createCopyValue(std::forward<T>(val), std::forward<MetaArgs>(meta)...);
		return std::make_shared<Property>(name, value);
	}

	template <class T>
	Property::SPtr createRefProperty(const std::string& name, T& val)
	{
		auto value = createRefValue(val);
		return std::make_shared<Property>(name, value);
	}

	template <class T, class... MetaArgs>
	Property::SPtr createRefProperty(const std::string& name, T& val, MetaArgs&&... meta)
	{
		auto value = createRefValue(val, std::forward<MetaArgs>(meta)...);
		return std::make_shared<Property>(name, value);
	}

	//****************************************************************************//

	template <class valType, class propType = valType>
	class ValueRefWrapper : public BaseValueWrapper
	{
	public:
		ValueRefWrapper(valType& value, Property::SPtr property)
			: BaseValueWrapper(property), m_value(value)
		{ 
			m_propertyValue = property->value<propType>(); 
			assert(m_propertyValue != nullptr);
		}

		void writeToValue() override	{ details::copyFromVector(m_propertyValue->value(), m_value); }
		void readFromValue() override	{ details::copyToVector(m_value, details::value(m_propertyValue->value())); }
	protected:
		valType& m_value;
		std::shared_ptr<PropertyValue<propType>> m_propertyValue;
	};

	template <class T>
	BaseValueWrapper::SPtr createValueRefWrapper(T& value, Property::SPtr property)
	{
		using property_type = details::PropertyValueType<T>::property_type;
		return std::make_shared<ValueRefWrapper<T, property_type>>(value, property);
	}

}
