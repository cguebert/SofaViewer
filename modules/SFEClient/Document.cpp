#include "Document.h"

#include <core/DocumentFactory.h>
#include <core/ObjectProperties.h>
#include <core/SimpleGUI.h>

#include <sfe/sofaFrontEndClient.h>

#include <iostream>
#include <future>

int SFEClientDoc = RegisterDocument<Document>("Sofa Client").setDescription("Run Sofa scenes using Sofa Front End Client").canCreateNew(true);
ModuleHandle SFEClientModule = RegisterModule("SFEClient").addDocument(SFEClientDoc);

Document::Document(const std::string& type)
	: SofaDocument(type, sfe::Simulation())
{
}

void Document::initUI(simplegui::SimpleGUI& gui)
{
	auto dialog = gui.createDialog("Connect to SFE Server");
	auto& panel = dialog->content();

	std::string serverAddress = "localhost";
	panel.addProperty(property::createRefProperty("Server address", serverAddress));

	int serverPort = 5074;
	panel.addProperty(property::createRefProperty("Server port", serverPort));

	if (dialog->exec())
	{
		m_simulation = sfe::getRemoteSimulation(serverAddress, std::to_string(serverPort));
		if (!m_simulation)
			return;

		SofaDocument::initUI(gui);
		parseScene();
		setupCallbacks();
	}
}
