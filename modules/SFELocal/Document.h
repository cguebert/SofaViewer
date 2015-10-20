#pragma once

#include <SFELib/SofaDocument.h>
#include <sfe/Communication.h>

class Document : public SofaDocument
{
public:
	Document(simplegui::SimpleGUI& gui);
	std::string documentType() override;

	bool loadFile(const std::string& path) override;
	void initUI() override;

protected:
	void modifyDataRepository();
	void launchServer();

	sfecom::Communication m_communication;
	bool m_serverRunning = false;
};
