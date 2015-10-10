#pragma once

#include <SFELib/SofaDocument.h>
#include <sfe/Communication.h>

class Document : public SofaDocument
{
public:
	Document(ui::SimpleGUI& gui);
	std::string documentType() override;

	void initUI() override;

protected:
};
