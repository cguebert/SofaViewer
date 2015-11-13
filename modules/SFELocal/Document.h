#pragma once

#include <SFELib/SofaDocument.h>
#include <sfe/SFEServer.h>

class Document : public SofaDocument
{
public:
	Document(const std::string& type);
	~Document();

	bool loadFile(const std::string& path) override;
	void initUI(simplegui::SimpleGUI& gui) override;

protected:
	void modifyDataRepository();
	void onServer();

	sfes::SFEServer m_server;
	simplegui::Button::SPtr m_serverButton;
};
