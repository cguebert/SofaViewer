#pragma once

#include <memory>

#include <QWidget>

class BaseDocument;
class SimpleGUIImpl;

class QTreeView;

class GraphView : public QWidget
{
	Q_OBJECT

public:
	GraphView(QWidget* parent, SimpleGUIImpl* gui);
	QWidget* view();

	void setDocument(std::shared_ptr<BaseDocument> doc);

private slots:
	void graphItemDoubleClicked(const QModelIndex&);
	void graphItemExpanded(const QModelIndex&);
	void graphItemCollapsed(const QModelIndex&);

private:
	void graphCallback(uint8_t reason);
	void setGraphItemExpandedState(size_t id, bool expanded);

	QTreeView* m_graph;
	SimpleGUIImpl* m_simpleGUI;
	std::shared_ptr<BaseDocument> m_document;

	std::vector<std::pair<size_t, bool>> m_graphItemsExpandedState; // Used when updating the graph
	bool m_updatingGraph = false;
};
