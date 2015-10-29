#pragma once

#include "VectorWrapper.h"

#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace conversion
{

	namespace details
	{

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

	template <class T>
	struct StringConversion<VectorWrapper<T>>
	{
		static std::string toString(const VectorWrapper<T>& val) { return conversion::toString(val.value()); }
		static void fromString(VectorWrapper<T>& val, const std::string& text) { conversion::fromString(val.value(), text); }
	};

	}

	//****************************************************************************//

	template <class T>
	std::string toString(const T& val)
	{
		return details::StringConversion<T>::toString(val);
	}

	template <class T>
	void fromString(T& val, const std::string& text)
	{
		details::StringConversion<T>::fromString(val, text);
	}
}
