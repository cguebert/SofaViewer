#pragma once

#include <SFELib/SofaDocument.h>
#include <sfe/Communication.h>

class Document : public SofaDocument
{
public:
	Document(const std::string& type);
	~Document();

	bool loadFile(const std::string& path) override;
	void initUI(simplegui::SimpleGUI& gui) override;

protected:
	void modifyDataRepository();
	void launchServer();

	sfecom::Communication m_communication;
	bool m_serverRunning = false;
};
