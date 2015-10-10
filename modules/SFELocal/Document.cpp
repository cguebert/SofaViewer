#include "Document.h"

#include <core/DocumentFactory.h>
#include <core/ObjectProperties.h>
#include <core/SimpleGUI.h>

#include <sfe/sofaFrontEndLocal.h>
#include <sfe/Helpers.h>
#include <sfe/Server.h>

#include <iostream>
#include <future>

int SFELocalDoc = RegisterDocument<Document>("SFELocalDoc").setDescription("Run Sofa scenes using Sofa Front End Local")
	.addLoadFile("Sofa scenes (*.scn)");
ModuleHandle SFELocalModule = RegisterModule("SFELocal").addDocument(SFELocalDoc);

// Register the types used in the SimpleRender lib so that SFE can directly copy to them
namespace sfe
{
template<> struct DataTypeTrait<glm::vec2> : public ArrayTypeTrait<glm::vec2, 2>{};
template<> struct DataTypeTrait<glm::vec3> : public ArrayTypeTrait<glm::vec3, 3>{};
}

Document::Document(ui::SimpleGUI& gui)
	: SofaDocument(gui, sfe::getLocalSimulation())
{
}

std::string Document::documentType()
{
	return "SFELocalDoc";
}

bool Document::loadFile(const std::string& path)
{
	// Read settings
	std::vector<std::string> sofaPaths;
	if(m_gui.settings().get("dataRepositoryPaths", sofaPaths))
		m_simulation.getHelper()->setDataRepositoryPaths(sofaPaths);

	return SofaDocument::loadFile(path);
}

void Document::initUI()
{
	SofaDocument::initUI();

	// Menu actions
	m_gui.addMenuItem(ui::SimpleGUI::Menu::Tools, "Sofa paths", "", [this](){ modifyDataRepository(); } );
	m_gui.addMenuItem(ui::SimpleGUI::Menu::Tools, "Launch Server", "", [this](){ launchServer(); } );
	m_gui.addMenuItem(ui::SimpleGUI::Menu::Tools, "Stop Server", "", [this](){ m_communication.closeCommunication(); m_serverRunning = false; });
}

void Document::modifyDataRepository()
{
	auto dialog = m_gui.createDialog("Sofa Data Repository");
	auto& panel = dialog->content();

	auto helper = m_simulation.getHelper();
	std::vector<std::string> paths = helper->dataRepositoryPaths();
	panel.addProperty(property::createRefProperty("Paths", paths));

	if(dialog->exec())
	{
		helper->setDataRepositoryPaths(paths);
		m_gui.settings().set("dataRepositoryPaths", paths);
	}
}

void connectionFunc(int socket, bool readOnly)
{
	auto com = std::make_shared<sfecom::Communication>(socket);

	{
		sfes::Server server(com);
		server.setReadOnly(readOnly);
		server.defaultMessageLoop();
	}
}

void acceptFunc(sfecom::Communication* com, bool readOnly)
{
	while (true)
	{
		int socket = com->acceptConnection();
		if (socket < 0)
			return;
		std::thread(connectionFunc, socket, readOnly).detach();
	}
}

void Document::launchServer()
{
	if (m_serverRunning)
		return;

	auto dialog = m_gui.createDialog("Launch SFE Server");
	auto& panel = dialog->content();

	int port = 5074;
	panel.addProperty(property::createRefProperty("Server port", port));

	int readOnly = 0;
	auto readOnlyProp = property::createRefProperty("Read only", readOnly);
	readOnlyProp->setWidget("checkbox");
	panel.addProperty(readOnlyProp);

	if (dialog->exec())
	{
		m_serverRunning = m_communication.createServer(port);

		std::thread(acceptFunc, &m_communication, readOnly != 0).detach();
	}
}
