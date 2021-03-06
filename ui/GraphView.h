#pragma once

#include <memory>

#include <QWidget>

class BaseDocument;
class GraphNode;

class QTreeView;

class GraphView : public QWidget
{
	Q_OBJECT

public:
	GraphView(QWidget* parent);
	QWidget* view();

	void setDocument(std::shared_ptr<BaseDocument> doc);

signals:
	void itemOpened(void* item);

private:
	void openItem(const QModelIndex&);
	void showContextMenu(const QPoint& pos);

	void graphCallback(int reason, GraphNode* node, int first, int last);

	QTreeView* m_graph;
	std::shared_ptr<BaseDocument> m_document;
};
