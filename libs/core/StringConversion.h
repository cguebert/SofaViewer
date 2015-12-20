#pragma once

#include <core/VectorWrapper.h>
#include <core/MetaProperties.h>

#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace conversion
{

	namespace details
	{

	template <class T>
	class is_streamable
	{
		template<typename SS, typename TT> static auto testL(int) 
			-> decltype( std::declval<SS&>() << std::declval<TT>(), std::true_type() );
		template<typename, typename> static auto testL(...) -> std::false_type;

		template<typename SS, typename TT> static auto testR(int) 
			-> decltype( std::declval<SS&>() >> std::declval<TT&>(), std::true_type() );
		template<typename, typename> static auto testR(...) -> std::false_type;

	public:
		static const bool value = decltype(testL<std::ostringstream,T>(0))::value && decltype(testR<std::istringstream,T>(0))::value;
	};

	template <class T>
	class is_streamable<std::vector<T>> : public is_streamable<T> {};

	template <class T>
	class is_streamable<VectorWrapper<T>> : public is_streamable<T> {};

	//****************************************************************************//

	template <class T>
	struct StringConversion
	{
		static std::string toString(const T& val) 
		{ 
			std::ostringstream out; 
			out << val; 
			return out.str(); 
		}

		static void fromString(T& val, const std::string& text)
		{
			std::istringstream in(text); 
			in >> val;
		}
	};

	template <>
	struct StringConversion<std::string>
	{
		static std::string toString(const std::string& val)
		{ return val; }

		static void fromString(std::string& val, const std::string& text)
		{ val = text; }
	};

	template <class T>
	struct StringConversion<std::vector<T>>
	{
		static std::string toString(const std::vector<T>& val)
		{
			std::ostringstream out;

			if (!val.empty())
				out << val[0];

			for (typename std::vector<T>::size_type i = 1, nb = val.size(); i < nb; ++i)
				out << " " << val[i];

			return out.str();
		}

		static void fromString(std::vector<T>& val, const std::string& text)
		{ 
			val.clear();
			std::istringstream in(text);
			T t = T();
			while (in >> t)
				val.push_back(t);
		}
	};

	template <>
	struct StringConversion<std::vector<std::string>>
	{
		static std::string toString(const std::vector<std::string>& val)
		{
			std::ostringstream out;

			if (!val.empty())
				out << val[0];

			for (std::vector<std::string>::size_type i = 1, nb = val.size(); i < nb; ++i)
				out << "|" << val[i];

			return out.str();
		}

		static void fromString(std::vector<std::string>& val, const std::string& text)
		{ 
			val.clear();
			std::istringstream in(text);
			std::string t;
			while (std::getline(in, t, '|'))
				val.push_back(t);
		}
	};

	template <class T>
	struct StringConversion<VectorWrapper<T>>
	{
		static std::string toString(const VectorWrapper<T>& val) { return StringConversion<T>::toString(val.value()); }
		static void fromString(VectorWrapper<T>& val, const std::string& text) { StringConversion<T>::fromString(val.value(), text); }
	};

	//****************************************************************************//

	template <class T, bool use_streams> struct ConversionSelector { };

	template <class T>
	struct ConversionSelector<T, true>
	{
		static std::string toString(const T& val) { return details::StringConversion<T>::toString(val); }
		static void fromString(T& val, const std::string& text) { details::StringConversion<T>::fromString(val, text); }
		static std::string toString(const T& val, const meta::MetaContainer<T>& meta) { return details::StringConversion<T>::toString(val); }
		static void fromString(T& val, const std::string& text, const meta::MetaContainer<T>& meta) { details::StringConversion<T>::fromString(val, text); }
	};

	template <class T>
	struct ConversionSelector<T, false>
	{
		static std::string toString(const T& val) 
		{ static_assert(false, "Cannot save this type"); return ""; }

		static void fromString(T& val, const std::string& text) 
		{ static_assert(false, "Cannot load this type"); }

		static std::string toString(const T& val, const meta::MetaContainer<T>& meta) 
		{ return meta.serialize(val); }

		static void fromString(T& val, const std::string& text, const meta::MetaContainer<T>& meta) 
		{ meta.deserialize(val, text); }
	};

	} // namespace details

	//****************************************************************************//

	template <class T>
	std::string toString(const T& val)
	{ return details::ConversionSelector<T, details::is_streamable<std::decay_t<T>>::value>::toString(val); }

	template <class T>
	void fromString(T& val, const std::string& text)
	{ return details::ConversionSelector<T, details::is_streamable<std::decay_t<T>>::value>::fromString(val, text); }

	template <class T>
	std::string toString(const T& val, const meta::MetaContainer<T>& meta)
	{ return details::ConversionSelector<T, details::is_streamable<std::decay_t<T>>::value>::toString(val, meta); }

	template <class T>
	void fromString(T& val, const std::string& text, const meta::MetaContainer<T>& meta)
	{ return details::ConversionSelector<T, details::is_streamable<std::decay_t<T>>::value>::fromString(val, text, meta); }
}
