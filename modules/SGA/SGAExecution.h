#pragma once

#include <sga/ObjectFactory.h>

class SGANode;

class SGAExecution
{
public:
	SGAExecution(sga::ObjectFactory factory);

	bool convert(SGANode* root);

private:
	sga::ObjectFactory m_factory;
};