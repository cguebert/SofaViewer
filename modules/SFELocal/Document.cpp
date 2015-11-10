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
	if(m_gui->settings().get("dataRepositoryPaths", sofaPaths))
		m_simulation.getHelper()->setDataRepositoryPaths(sofaPaths);

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
	auto& menu = m_gui->getMenu(simplegui::SimpleGUI::MenuType::Tools);
	menu.addItem("Sofa paths", "Modify the directories where Sofa will look for meshes and textures", [this](){ modifyDataRepository(); } );
	menu.addItem("Launch Server", "Launch a SFE server", [this](){ launchServer(); } );
	menu.addItem("Stop Server", "Stop the SFE server", [this](){ m_server.stopServer(); });
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

void Document::launchServer()
{
	if (m_server.isRunning())
		return;

	auto dialog = m_gui->createDialog("Launch SFE Server");
	auto& panel = dialog->content();

	int port = 5074;
	panel.addProperty(property::createRefProperty("Server port", port));

	int readOnly = 0;
	auto readOnlyProp = property::createRefProperty("Read only", readOnly, meta::Checkbox());
	panel.addProperty(readOnlyProp);

	if (dialog->exec())
		m_server.launchServer(port, readOnly == 1);
}
