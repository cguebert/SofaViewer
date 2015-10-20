#include <ui/GraphModel.h>
#include <ui/GraphView.h>

#include <core/BaseDocument.h>
#include <core/Graph.h>

#include <QtWidgets>

GraphView::GraphView(QWidget* parent)
	: QWidget(parent)
{
	m_graph = new QTreeView(this);
	m_graph->setUniformRowHeights(true);
	m_graph->header()->hide();
	m_graph->setExpandsOnDoubleClick(false);

	connect(m_graph, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(graphItemDoubleClicked(QModelIndex)));
	connect(m_graph, SIGNAL(expanded(QModelIndex)), this, SLOT(graphItemExpanded(QModelIndex)));
	connect(m_graph, SIGNAL(collapsed(QModelIndex)), this, SLOT(graphItemCollapsed(QModelIndex)));
}

QWidget* GraphView::view()
{
	return m_graph;
}

void GraphView::setDocument(std::shared_ptr<BaseDocument> doc)
{
	m_document = doc;

	// Cleanly remove the previous model of the graph view
	auto oldModel = m_graph->model();
	m_graph->setModel(nullptr);
	if (oldModel)
		delete oldModel;

	auto& graph = m_document->graph();
	graph.setUpdateCallback([this](uint16_t val){ graphCallback(val); });
}

void GraphView::graphItemDoubleClicked(const QModelIndex& index)
{
	if (index.isValid())
	{
		GraphNode* item = static_cast<GraphNode*>(index.internalPointer());
		if (item)
		{
		/*	size_t uniqueId = item->uniqueId;
			auto it = std::find_if(m_propertiesDialogs.begin(), m_propertiesDialogs.end(), [uniqueId](const PropertiesDialogPair& p){
				return p.first == uniqueId;
			});
			if (it != m_propertiesDialogs.end()) // Show existing dialog
			{
				it->second->activateWindow();
				it->second->raise();
				return;
			}

			// Else create a new one
			auto prop = m_document->objectProperties(item);
			if (prop)
			{
				PropertiesDialog* dlg = new PropertiesDialog(prop, this);
				m_propertiesDialogs.emplace_back(uniqueId, dlg);
				dlg->show();
			}*/
		}
	}
}

void GraphView::graphItemExpanded(const QModelIndex& index)
{
	if (!m_updatingGraph && index.isValid())
	{
		GraphNode* item = static_cast<GraphNode*>(index.internalPointer());
		if (item)
			setGraphItemExpandedState(item->uniqueId, true);
	}
}

void GraphView::graphItemCollapsed(const QModelIndex& index)
{
	if (!m_updatingGraph && index.isValid())
	{
		GraphNode* item = static_cast<GraphNode*>(index.internalPointer());
		if (item)
			setGraphItemExpandedState(item->uniqueId, false);
	}
}

void GraphView::setGraphItemExpandedState(size_t id, bool expanded)
{
	auto it = std::find_if(m_graphItemsExpandedState.begin(), m_graphItemsExpandedState.end(), [id](const std::pair<size_t, bool>& p)
	{ return p.first == id; });

	if (it != m_graphItemsExpandedState.end())
		it->second = expanded;
	else
		m_graphItemsExpandedState.emplace_back(id, expanded);
}

void GraphView::graphCallback(uint8_t reasonVal)
{
	auto reason = static_cast<Graph::CallbackReason>(reasonVal);
	auto model = dynamic_cast<GraphModel*>(m_graph->model());

	switch (reason)
	{
	case Graph::CallbackReason::BeginSetNode:
	{
		if (model)
		{
			model->beginReset();
		}
		break;
	}

	case Graph::CallbackReason::EndSetNode:
	{
		if (!model)
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
		for (auto index : indices)
		{
			GraphNode* item = static_cast<GraphNode*>(index.internalPointer());
			if (item)
			{
				bool expanded = item->expanded; // First use the value set in the graph

				// Then restore the state if the node existed before the update
				auto uniqueId = item->uniqueId;
				auto it = std::find_if(itemsState.begin(), itemsState.end(), [uniqueId](const std::pair<size_t, bool>& p)
				{ return p.first == uniqueId; });

				if (it != itemsState.end())
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
