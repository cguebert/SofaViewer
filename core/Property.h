#pragma once

#include <string>

class Property
{
public:
	const std::string& name() const;
	const std::string& help() const;
	const std::string& group() const;

//protected:
	std::string m_value;
	std::string m_name, m_help, m_group;
};

inline const std::string& Property::name() const
{ return m_name; }

inline const std::string& Property::help() const
{ return m_help; }

inline const std::string& Property::group() const
{ return m_group; }
