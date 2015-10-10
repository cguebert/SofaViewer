#include "Document.h"

#include <core/DocumentFactory.h>
#include <core/ObjectProperties.h>
#include <core/SimpleGUI.h>

#include <sfe/sofaFrontEndClient.h>

#include <iostream>
#include <future>

int SFEClientDoc = RegisterDocument<Document>("SFEClientDoc").setDescription("Run Sofa scenes using Sofa Front End Client").canCreateNew(true);
ModuleHandle SFEClientModule = RegisterModule("SFEClient").addDocument(SFEClientDoc);

// Register the types used in the SimpleRender lib so that SFE can directly copy to them
namespace sfe
{
template<> struct DataTypeTrait<glm::vec2> : public ArrayTypeTrait<glm::vec2, 2>{};
template<> struct DataTypeTrait<glm::vec3> : public ArrayTypeTrait<glm::vec3, 3>{};
}

Document::Document(ui::SimpleGUI& gui)
	: SofaDocument(gui, sfe::Simulation())
{
}

std::string Document::documentType()
{
	return "SFEClientDoc";
}

void Document::initUI()
{
	SofaDocument::initUI();

	auto dialog = m_gui.createDialog("Connect to SFE Server");
	auto& panel = dialog->content();

	std::string serverAddress = "localhost";
	panel.addProperty(property::createRefProperty("Server address", serverAddress));

	int serverPort = 5074;
	panel.addProperty(property::createRefProperty("Server port", serverPort));

	if (dialog->exec())
	{
		m_simulation = sfe::getRemoteSimulation(serverAddress, std::to_string(serverPort));
		if (m_simulation)
			parseScene();
	}
}
