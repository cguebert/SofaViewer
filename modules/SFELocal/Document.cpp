#include "Document.h"

#include <core/DocumentFactory.h>
#include <core/ObjectProperties.h>

#include <sfe/sofaFrontEndLocal.h>
#include <sfe/Helpers.h>
#include <sfe/Server.h>

#include <iostream>
#include <fstream>
#include <future>

int SFELocalDoc = RegisterDocument<Document>("Sofa Local").setDescription("Run Sofa scenes using Sofa Front End Local")
	.addLoadFile("Sofa scenes (*.scn)");
ModuleHandle SFELocalModule = RegisterModule("SFELocal").addDocument(SFELocalDoc);

// Register the types used in the SimpleRender lib so that SFE can directly copy to them
namespace sfe
{
template<> struct DataTypeTrait<glm::vec2> : public ArrayTypeTrait<glm::vec2, 2>{};
template<> struct DataTypeTrait<glm::vec3> : public ArrayTypeTrait<glm::vec3, 3>{};
}

Document::Document(const std::string& type)
	: SofaDocument(type, sfe::getLocalSimulation())
{
}

Document::~Document()
{
	m_server.stopServer();
	m_simulation.clear(); // Free the simulation
}

bool Document::loadFile(const std::string& path)
{
	// Read settings
	std::vector<std::string> sofaPaths;
	if (m_gui->settings().contains("dataRepositoryPaths"))
	{
		m_gui->settings().get("dataRepositoryPaths", sofaPaths);
		m_simulation.getHelper()->setDataRepositoryPaths(sofaPaths);
	}
	else
	{
		namespace buttons = simplegui::buttons;
		if (buttons::Yes == m_gui->messageBox(simplegui::MessageBoxType::question, "Sofa ressources",
			"Do you want to set the SOFA share path now ?\n(Can be changed later in tools/Sofa paths)",
			buttons::Yes | buttons::No))
		{
			modifyDataRepository();
		}
	}

	m_simulation.setAnimate(false, true);
	if (!m_simulation.loadFile(path))
		return false;
	else
	{
		//	simulation.root().createObject("RequiredPlugin", { { "pluginName", "DtExtensions" } });
		//	mouseInteractor = simulation.root().createObject("DtMouseInteractor");

		// Initializes the scene
		m_simulation.init();

		// Create the meshes for rendering
		parseScene();

		setupCallbacks();

		createGraph();

		return true;
	}
}

void Document::initUI(simplegui::SimpleGUI& gui)
{
	SofaDocument::initUI(gui);

	// Menu actions
	auto& menu = m_gui->getMenu(simplegui::MenuType::Tools);
	m_serverButton = menu.addItem("Launch Server", "Launch a SFE server", [this](){ onServer(); } );
	menu.addSeparator();
	menu.addItem("Sofa paths", "Modify the directories where Sofa will look for meshes and textures", [this](){ modifyDataRepository(); } );
}

void Document::modifyDataRepository()
{
	auto dialog = m_gui->createDialog("Sofa Data Repository");
	auto& panel = dialog->content();

	auto helper = m_simulation.getHelper();
	std::vector<std::string> paths = helper->dataRepositoryPaths();
	panel.addProperty(property::createRefProperty("Paths", paths, meta::Directory()));
	dialog->setMinimumSize(400, 150);

	if(dialog->exec())
	{
		helper->setDataRepositoryPaths(paths);
		m_gui->settings().set("dataRepositoryPaths", paths);
	}
}

void Document::onServer()
{
	if (m_server.isRunning())
	{
		m_server.stopServer();
		m_serverButton->setTitle("Launch Server");
		m_serverButton->setHelp("Launch a SFE server");
		return;
	}

	auto dialog = m_gui->createDialog("SFE Server parameters");
	auto& panel = dialog->content();

	int port = 5074;
	panel.addProperty(property::createRefProperty("Server port", port));

	bool readOnly = false;
	panel.addProperty(property::createRefProperty("Read only", readOnly));

	if (dialog->exec())
	{
		if (m_server.launchServer(port, readOnly))
		{
			m_serverButton->setTitle("Stop Server");
			m_serverButton->setHelp("Stop the SFE server");
		}
		else
			m_gui->messageBox(simplegui::MessageBoxType::warning, "SFE Server", "Could not start the SFE server with theses parameters.");

	}
}
