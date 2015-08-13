#include <ui/GraphModel.h>
#include <ui/MainWindow.h>
#include <ui/OpenGLView.h>
#include <ui/PropertiesDialog.h>
#include <ui/SimpleGUIImpl.h>

#include <core/DocumentFactory.h>
#include <core/Graph.h>

#include <QtWidgets>

class ChangeDir
{
public:
	ChangeDir(const std::string& path)
	{
		prevDir = QDir::current();
		QString qtPath = path.c_str();
		QDir::setCurrent(QFileInfo(qtPath).absolutePath());
	}

	~ChangeDir() { QDir::setCurrent(prevDir.absolutePath()); }

protected:
	QDir prevDir;
};

/******************************************************************************/

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	// Buttons
	auto buttonsDock = new QDockWidget(tr("Buttons"));
	m_buttonsDockWidget = new QWidget(this);
	auto buttonsLayout = new QGridLayout;
	m_buttonsDockWidget->setLayout(buttonsLayout);
	buttonsDock->setObjectName("ButtonsDock");
	buttonsDock->setWidget(m_buttonsDockWidget);
	buttonsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, buttonsDock);

	// Graph tree
	m_graph = new QTreeView(this);
	m_graph->setUniformRowHeights(true);
	m_graph->header()->hide();
	m_graph->setExpandsOnDoubleClick(false);
	auto graphDock = new QDockWidget(tr("Graph"), this);
	graphDock->setObjectName("GraphDock");
	graphDock->setWidget(m_graph);
	graphDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, graphDock);

	connect(m_graph, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(graphItemDoubleClicked(QModelIndex)));
	connect(m_graph, SIGNAL(expanded(QModelIndex)), this, SLOT(graphItemExpanded(QModelIndex)));
	connect(m_graph, SIGNAL(collapsed(QModelIndex)), this, SLOT(graphItemCollapsed(QModelIndex)));

	m_simpleGUI = std::make_shared<SimpleGUIImpl>(this);

	m_view = new OpenGLView(this);
	setCentralWidget(m_view);

	createActions();
	createMenus();

	statusBar();

	setWindowIcon(QIcon(":/share/icons/icon.png"));
	setCurrentFile("");

	readSettings();
}

void MainWindow::createActions()
{
	m_openAction = new QAction(tr("&Open..."), this);
	m_openAction->setIcon(QIcon(":/share/icons/open.png"));
	m_openAction->setShortcut(QKeySequence::Open);
	m_openAction->setStatusTip(tr("Open an existing Sofa scene"));
	connect(m_openAction, SIGNAL(triggered()), this, SLOT(open()));

	m_saveAction = new QAction(tr("&Save"), this);
	m_saveAction->setIcon(QIcon(":/share/icons/save.png"));
	m_saveAction->setShortcut(QKeySequence::Save);
	m_saveAction->setStatusTip(tr("Save the scene to disk"));
	m_saveAction->setEnabled(false);
	connect(m_saveAction, SIGNAL(triggered()), this, SLOT(save()));

	m_saveAsAction = new QAction(tr("Save &As..."), this);
	m_saveAsAction->setStatusTip(tr("Save the scene under a new name"));
	m_saveAsAction->setEnabled(false);
	connect(m_saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

	for (int i = 0; i < MaxRecentFiles; ++i) {
		m_recentFileActions[i] = new QAction(this);
		m_recentFileActions[i]->setVisible(false);
		connect(m_recentFileActions[i], SIGNAL(triggered()),
				this, SLOT(openRecentFile()));
	}

	m_exitAction = new QAction(tr("E&xit"), this);
	m_exitAction->setShortcut(tr("Ctrl+Q"));
	m_exitAction->setStatusTip(tr("Exit Panda"));
	connect(m_exitAction, SIGNAL(triggered()), this, SLOT(close()));

	m_aboutAction = new QAction(tr("&About"), this);
	m_aboutAction->setStatusTip(tr("Show the application's About box"));
	connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(about()));

	m_aboutQtAction = new QAction(tr("About &Qt"), this);
	m_aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
	connect(m_aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
	m_fileMenu = menuBar()->addMenu(tr("&File"));
	m_fileMenu->addAction(m_openAction);
	m_fileMenu->addAction(m_saveAction);
	m_fileMenu->addAction(m_saveAsAction);
	m_separatorAction = m_fileMenu->addSeparator();
	for (int i = 0; i < MaxRecentFiles; ++i)
		m_fileMenu->addAction(m_recentFileActions[i]);
	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_exitAction);

	m_toolsMenu = menuBar()->addMenu(tr("&Tools"));

	m_viewMenu = menuBar()->addMenu(tr("&View"));

	m_helpMenu = menuBar()->addMenu(tr("&Help"));
	m_helpMenu->addSeparator();
	m_helpMenu->addAction(m_aboutAction);
	m_helpMenu->addAction(m_aboutQtAction);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (okToContinue())
	{
		writeSettings();
		event->accept();
	}
	else
		event->ignore();
}

void MainWindow::readSettings()
{
	QSettings settings("Christophe Guebert", "SofaFrontEndViewer");

	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("state").toByteArray());

	m_recentFiles = settings.value("recentFiles").toStringList();
	updateRecentFileActions();
}

void MainWindow::writeSettings()
{
	QSettings settings("Christophe Guebert", "SofaFrontEndViewer");

	settings.setValue("recentFiles", m_recentFiles);
	settings.setValue("geometry", saveGeometry());
	settings.setValue("state", saveState());
}

bool MainWindow::okToContinue()
{
	if (isWindowModified())
	{
		int r = QMessageBox::warning(this, tr("Panda"),
						tr("The document has been modified.\n"
						   "Do you want to save your changes?"),
						QMessageBox::Yes | QMessageBox::No
						| QMessageBox::Cancel);
		if (r == QMessageBox::Yes)
			return save();
		else if (r == QMessageBox::Cancel)
			return false;
	}
	return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
	m_curFile = fileName;
	setWindowModified(false);

	QString shownName = tr("Untitled");
	if (!m_curFile.isEmpty())
	{
		shownName = strippedName(m_curFile);
		m_recentFiles.removeAll(m_curFile);
		m_recentFiles.prepend(m_curFile);
		updateRecentFileActions();
	}

	setWindowTitle(tr("%1[*] - %2").arg(shownName)
								   .arg(tr("Sofa Front End Viewer")));
}

void MainWindow::updateRecentFileActions()
{
	QMutableStringListIterator i(m_recentFiles);
	while (i.hasNext()) {
		if (!QFile::exists(i.next()))
			i.remove();
	}

	for (int j = 0; j < MaxRecentFiles; ++j)
	{
		if (j < m_recentFiles.count())
		{
			QString text = tr("&%1 %2")
						   .arg(j + 1)
						   .arg(strippedName(m_recentFiles[j]));
			m_recentFileActions[j]->setText(text);
			m_recentFileActions[j]->setData(m_recentFiles[j]);
			m_recentFileActions[j]->setVisible(true);
		}
		else
			m_recentFileActions[j]->setVisible(false);
	}
	m_separatorAction->setVisible(!m_recentFiles.isEmpty());
}

bool MainWindow::save()
{
	if (m_curFile.isEmpty())
		return saveAs();
	else
		return saveFile(m_curFile);
}

bool MainWindow::saveAs()
{
	auto filters = DocumentFactory::instance().saveFilesFilter();
	QString fileName = QFileDialog::getSaveFileName(this,
							   tr("Save Document"), ".",
							   filters.c_str());
	if (fileName.isEmpty())
		return false;

	return saveFile(fileName);
}

QString MainWindow::strippedName(const QString& fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}

bool MainWindow::loadFile(const QString& fileName)
{
	std::string cpath = fileName.toStdString();
	auto document = DocumentFactory::instance().createForFile(cpath, *m_simpleGUI.get());
	if(!document) // TODO: show a dialog so that the user can choose the correct document to use
	{
		statusBar()->showMessage(tr("Cannot create a document to load this file"), 2000);
		return false;
	}

	ChangeDir cd(cpath);

	if (!document->loadFile(cpath))
	{
		statusBar()->showMessage(tr("Loading failed"), 2000);
		return false;
	}

	setDocument(document);

	setCurrentFile(fileName);
	statusBar()->showMessage(tr("File loaded"), 2000);
	return true;
}

bool MainWindow::saveFile(const QString& fileName)
{
	if (!m_document->saveFile(fileName.toStdString()))
	{
		statusBar()->showMessage(tr("Saving failed"), 2000);
		return false;
	}

	setCurrentFile(fileName);
	statusBar()->showMessage(tr("File saved"), 2000);
	return true;
}

void MainWindow::open()
{
	if (okToContinue()) {
		auto filters = DocumentFactory::instance().loadFilesFilter();
		QString fileName = QFileDialog::getOpenFileName(this,
								   tr("Open Document"), ".",
								   filters.c_str());
		if (!fileName.isEmpty())
			loadFile(fileName);
	}
}

void MainWindow::openRecentFile()
{
	if (okToContinue())
	{
		QAction *action = qobject_cast<QAction *>(sender());
		if(action)
		{
			loadFile(action->data().toString());
		}
	}
}

void MainWindow::showStatusBarMessage(QString text)
{
	statusBar()->showMessage(text, 2000);
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About SFE Viewer"),
			tr("<h2>Sofa Front End Viewer</h2>"
			   "<p>Copyright &copy; 2015 Christophe Guébert"
			   "<p>Using Sofa and Sofa Front End"));
}

void MainWindow::graphItemDoubleClicked(const QModelIndex& index)
{
	if(index.isValid())
	{
		GraphNode* item = static_cast<GraphNode*>(index.internalPointer());
		if(item)
		{
			size_t uniqueId = item->uniqueId;
			auto it = std::find_if(m_propertiesDialogs.begin(), m_propertiesDialogs.end(), [uniqueId](const PropertiesDialogPair& p){
				return p.first == uniqueId;
			});
			if(it != m_propertiesDialogs.end()) // Show existing dialog
			{
				it->second->activateWindow();
				it->second->raise();
				return;
			}

			// Else create a new one
			auto prop = m_document->objectProperties(item);
			if(prop)
			{
				PropertiesDialog* dlg = new PropertiesDialog(prop, this);
				m_propertiesDialogs.emplace_back(uniqueId, dlg);
				dlg->show();
			}
		}
	}
}

void MainWindow::graphItemExpanded(const QModelIndex& index)
{
	if(!m_updatingGraph && index.isValid())
	{
		GraphNode* item = static_cast<GraphNode*>(index.internalPointer());
		if(item)
			setGraphItemExpandedState(item->uniqueId, true);
	}
}

void MainWindow::graphItemCollapsed(const QModelIndex& index)
{
	if(!m_updatingGraph && index.isValid())
	{
		GraphNode* item = static_cast<GraphNode*>(index.internalPointer());
		if(item)
			setGraphItemExpandedState(item->uniqueId, false);
	}
}

void MainWindow::setGraphItemExpandedState(size_t id, bool expanded)
{
	auto it = std::find_if(m_graphItemsExpandedState.begin(), m_graphItemsExpandedState.end(), [id](const std::pair<size_t, bool>& p)
	{ return p.first == id; });

	if(it != m_graphItemsExpandedState.end())
		it->second = expanded;
	else
		m_graphItemsExpandedState.emplace_back(id, expanded);
}

int MainWindow::addCallback(CallbackFunc func)
{
	auto id = m_callbacks.size();
	m_callbacks.push_back(func);
	return id;
}

void MainWindow::executeCallback()
{
	QAction* action = qobject_cast<QAction*>(sender());
	if(action)
	{
		bool ok = false;
		int id = action->data().toInt(&ok);
		if(ok)
			m_callbacks[id]();
	}
}

QMenu* MainWindow::menu(unsigned char idVal)
{
	using Type = ui::SimpleGUI::Menu;
	auto menuId = static_cast<Type>(idVal);
	switch(menuId)
	{
	case Type::File:
		return m_fileMenu;
	case Type::Tools:
		return m_toolsMenu;
	case Type::View:
		return m_viewMenu;
	case Type::Help:
		return m_helpMenu;
	default:
		return nullptr;
	}
}

QGridLayout* MainWindow::buttonsLayout()
{
	QGridLayout* layout = dynamic_cast<QGridLayout*>(m_buttonsDockWidget->layout());
	if(!layout)
		return new QGridLayout(m_buttonsDockWidget);
	return layout;
}

void MainWindow::closeDialog(ObjectProperties* objProp)
{
	auto it = std::find_if(m_propertiesDialogs.begin(), m_propertiesDialogs.end(), [objProp](const PropertiesDialogPair& p){
		return p.second->objectProperties().get() == objProp;
	});

	if(it != m_propertiesDialogs.end())
		it->second->reject();
}

void MainWindow::removeDialog(PropertiesDialog* dlg)
{
	m_document->closeObjectProperties(dlg->objectProperties());
	auto it = std::find_if(m_propertiesDialogs.begin(), m_propertiesDialogs.end(), [dlg](const PropertiesDialogPair& p){
		return p.second == dlg;
	});
	if(it != m_propertiesDialogs.end())
		m_propertiesDialogs.erase(it);
	dlg->deleteLater();
}

void MainWindow::setDocument(std::shared_ptr<BaseDocument> document)
{
	m_simpleGUI->clear();

	// Cleanly remove the previous model of the graph view
	auto oldModel = m_graph->model();
	m_graph->setModel(nullptr);
	if(oldModel)
		delete oldModel;

	m_document = document;

	m_saveFilter = DocumentFactory::instance().saveFilesFilter().c_str();
	bool canSave = !m_saveFilter.isEmpty();
	m_saveAction->setEnabled(canSave);
	m_saveAsAction->setEnabled(canSave);

	auto& graph = m_document->graph();
	graph.setUpdateCallback([this](uint16_t val){ graphCallback(val); });

	m_view->setDocument(m_document.get());
	m_document->initUI();
}

void MainWindow::graphCallback(uint16_t reasonVal)
{
	auto reason = static_cast<Graph::CallbackReason>(reasonVal);
	auto model = dynamic_cast<GraphModel*>(m_graph->model());

	switch(reason)
	{
		case Graph::CallbackReason::BeginSetNode:
		{
			if(model)
			{
				model->beginReset();
			}
			break;
		}

		case Graph::CallbackReason::EndSetNode:
		{
			if(!model)
			{
				model = new GraphModel(this, m_document->graph());
				m_graph->setModel(model);
			}
			else
				model->updatePixmaps();
			model->endReset();

			m_updatingGraph = true;
			m_graph->expandAll();

			// Restore the expanded state
			auto itemsState = m_graphItemsExpandedState;
			m_graphItemsExpandedState.clear();
			auto indices = model->getPersistentIndexList();
			for(auto index : indices)
			{
				GraphNode* item = static_cast<GraphNode*>(index.internalPointer());
				if(item)
				{
					bool expanded = item->expanded; // First use the value set in the graph

					// Then restore the state if the node existed before the update
					auto uniqueId = item->uniqueId;
					auto it = std::find_if(itemsState.begin(), itemsState.end(), [uniqueId](const std::pair<size_t, bool>& p)
					{ return p.first == uniqueId; });

					if(it != itemsState.end())
						expanded = it->second;

					m_graphItemsExpandedState.emplace_back(uniqueId, expanded);
					m_graph->setExpanded(index, expanded);
				}
			}

			m_updatingGraph = false;
			break;
		}
	} // switch
}
