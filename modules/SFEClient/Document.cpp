#include "Document.h"

#include <core/DocumentFactory.h>
#include <core/ObjectProperties.h>

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
		{
			gui.messageBox(simplegui::MessageBoxType::warning, "Connection error", "Could not connect to the server using these parameters.");
			gui.closeDocument();
			return;
		}

		auto helper = sfe::getClientSpecificHelper(m_simulation);
		auto callback = helper->addCallback(sfec::ClientSpecificHelper::CallbackType::Shutdown, [this]() { serverShutdown(); });
		m_sfeCallbacks.push_back(callback);

		SofaDocument::initUI(gui);
		parseScene();
		setupCallbacks();
		createGraph();
	}
}

void Document::serverShutdown()
{
	m_graph.setRoot(nullptr);

	m_animateButton->setEnabled(false);
	m_stepButton->setEnabled(false);
	m_resetButton->setEnabled(false);
	m_updateGraphButton->setEnabled(false);

//	m_gui->messageBox(simplegui::MessageBoxType::information, "Server shutdown", "The server has closed the connection");
//	m_gui->closeDocument();
}
