#pragma once

#include <SFELib/SofaDocument.h>
#include <sfe/Communication.h>

class Document : public SofaDocument
{
public:
	Document(const std::string& type);

	void initUI(simplegui::SimpleGUI& gui) override;

	void serverShutdown();
};
