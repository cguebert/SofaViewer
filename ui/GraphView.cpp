#include <ui/GraphModel.h>
#include <ui/GraphView.h>
#include <ui/simplegui/MenuImpl.h>

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
	m_graph->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(m_graph, &QTreeView::doubleClicked, this, &GraphView::openItem);
	connect(m_graph, &QTreeView::customContextMenuRequested, this, &GraphView::showContextMenu);
}

QWidget* GraphView::view()
{
	return m_graph;
}

void GraphView::setDocument(std::shared_ptr<BaseDocument> doc)
{
	// Cleanly remove the previous model of the graph view
	auto oldModel = m_graph->model();
	m_graph->setModel(nullptr);
	if (oldModel)
		delete oldModel;

	m_document = doc;
	if (!m_document)
		return;

	auto& graph = m_document->graph();
	graph.setUpdateCallback([this](int val, GraphNode* parent, int first, int last){ 
		graphCallback(val, parent, first, last); 
	});
}

void GraphView::openItem(const QModelIndex& index)
{
	if (index.isValid())
	{
		GraphNode* item = static_cast<GraphNode*>(index.internalPointer());
		if (item)
			emit itemOpened(item);
	}
}

void GraphView::showContextMenu(const QPoint& pos)
{
	auto index = m_graph->indexAt(pos);
	if (index.isValid())
	{
		GraphNode* item = static_cast<GraphNode*>(index.internalPointer());
		if (item)
		{
			QMenu menu;
			MenuImpl menuImpl(&menu);
			m_document->graphContextMenu(item, menuImpl);
			if (!menu.actions().isEmpty())
				menu.exec(m_graph->mapToGlobal(pos));
		}
	}
}

void GraphView::graphCallback(int reasonVal, GraphNode* node, int first, int last)
{
	auto reason = static_cast<Graph::CallbackReason>(reasonVal);
	auto model = dynamic_cast<GraphModel*>(m_graph->model());

	if (model)
		model->operation(reasonVal, node, first, last);

	switch (reason)
	{
	case Graph::CallbackReason::EndSetRoot:
	{
		if (!model)
		{
			model = new GraphModel(this, m_document->graph());
			model->updatePixmaps();
			m_graph->setModel(model);
		}

		if (node)
		{
			m_graph->expandAll();

			// Restore the expanded state
			auto nodes = graph::getNodes(node, [](GraphNode*){ return true; });
			for (auto node : nodes)
			{
				if (!node->expanded)
					m_graph->setExpanded(model->index(node), false);
			}
		}

		break;
	}

	case Graph::CallbackReason::EndInsertNode:
		if (model)
			m_graph->setExpanded(model->index(node), node->expanded);
		break;
	} // switch
}
