#pragma once

#include <core/StructMeta.h>

namespace meta
{

BaseStructItem::~BaseStructItem() 
{
}

namespace utils
{
	std::string replaceAll(const std::string& val, const std::string& from, const std::string& to)
{
	if (from.empty())
		return val;

	std::string ret = val;
	auto fromSize = from.size(), toSize = to.size();
	size_t pos = 0;
	while ((pos = val.find(from, pos)) != std::string::npos)
	{
		ret.replace(pos, fromSize, to);
		pos += toSize;
	}

	return ret;
}

std::string encode(const std::string& val)
{
	auto v = replaceAll(val, ",", "\\,");
	v = replaceAll(v, "}", "\\}");
	return v;
}

std::string decode(const std::string& val)
{
	auto v = replaceAll(val, "\\,", ",");
	v = replaceAll(v, "\\}", "}");
	return v;
}

std::string getValueToken(const std::string& text, size_t& pos)
{
	size_t start = pos, len = text.size();
	while (pos < len)
	{
		if ((text[pos] == ',' || text[pos] == '}') && text[pos - 1] != '\\')
			break;
		++pos;
	}

	size_t tokLen = pos - start;
	++pos;
	return text.substr(start, tokLen);
}

std::string getObjectToken(const std::string& text, size_t& pos)
{
	size_t start = pos, len = text.size();
	while (pos < len)
	{
		if (text[pos] == '}' && text[pos - 1] != '\\')
			break;
		++pos;
	}

	++pos;
	size_t tokLen = pos - start;
	return text.substr(start, tokLen);
}

} // namespace utils

} // namespace meta
