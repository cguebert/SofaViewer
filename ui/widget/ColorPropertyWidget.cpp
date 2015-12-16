#include <ui/widget/PropertyWidget.h>
#include <ui/widget/PropertyWidgetFactory.h>
#include <ui/widget/SimplePropertyWidget.h>

#include <QtWidgets>

#include <functional>

// Being able to draw the full size of the widget (hard with a QLabel and pixmaps)
class PreviewView : public QWidget
{
public:
	using RenderFunc = std::function<void(QPainter& painter, QSize size)>;
	PreviewView(QWidget* parent, RenderFunc func) 
		: QWidget(parent), m_renderFunc(func) 
	{
		setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
	}

	QSize sizeHint() const
	{
		return QSize(16, 16);
	}

	void paintEvent(QPaintEvent*)
	{
		QStylePainter painter(this);
		m_renderFunc(painter, size());
	}

protected:
	RenderFunc m_renderFunc;
};

//****************************************************************************//

namespace
{
template <class T> std::vector<T>& getVector(std::vector<T>& v) { return v; }
template <class T> const std::vector<T>& getVector(const std::vector<T>& v) { return v; }

template <class T> std::vector<T>& getVector(VectorWrapper<std::vector<T>>& v) { return v.value(); }
template <class T> const std::vector<T>& getVector(const VectorWrapper<std::vector<T>>& v) { return v.value(); }
}

//****************************************************************************//

template <class T>
class ColorChooserWidget
{
protected:
	using value_type = T;

	PreviewView* m_preview = nullptr;
	BasePropertyWidget* m_parent = nullptr;
	bool m_hasAlpha = false;
	QColor m_color;

public:
	QWidget* createWidgets(BasePropertyWidget* parent)
	{
		m_parent = parent;

		m_preview = new PreviewView(parent, [this](QPainter& painter, QSize size){
			draw(painter, size);
		});

		auto button = new QPushButton("...", parent);
		QObject::connect(button, &QPushButton::clicked, [this]() { chooseColor(); });

		auto container = new QWidget(parent);
		auto layout = new QHBoxLayout();
		layout->addWidget(m_preview);
		layout->addWidget(button);
		layout->setContentsMargins(0, 0, 0, 0);
		container->setLayout(layout);

		return container;
	}
	void readFromProperty(const value_type& val)
	{
		const auto& v = getVector(val);
		auto s = v.size();
		if (s == 3)
			m_color = QColor::fromRgbF(v[0], v[1], v[2]);
		else if (s == 4)
			m_color = QColor::fromRgbF(v[0], v[1], v[2], v[3]);

		m_hasAlpha = (s == 4);
	}
	void writeToProperty(value_type& val)
	{
		auto& v = getVector(val);
		int s = v.size();
		qreal c[4];
		m_color.getRgbF(&c[0], &c[1], &c[2], &c[3]);
		for (int i = 0, nb = std::min(4, s); i < nb; ++i)
			v[i] = c[i];
	}
	void chooseColor()
	{
		QColorDialog::ColorDialogOptions opt;
		if (m_hasAlpha)
			opt = QColorDialog::ShowAlphaChannel;
		QColor tmp = QColorDialog::getColor(m_color, m_parent, "", opt);
		if (tmp.isValid())
		{
			m_color = tmp;
			m_parent->setWidgetDirty();
		}
	}
	void draw(QPainter& painter, QSize size)
	{
		static QPixmap checker;
		if (checker.isNull()) // Creating checker board in use for transparent colors
		{
			int s = ceil(size.height() / 2.0);
			checker = QPixmap(2 * s, 2 * s);
			QColor c1 = m_preview->palette().color(QPalette::Midlight);
			QColor c2 = m_preview->palette().color(QPalette::Dark);
			QPainter pmp(&checker);
			pmp.fillRect(0, 0, s, s, c1);
			pmp.fillRect(s, s, s, s, c1);
			pmp.fillRect(0, s, s, s, c2);
			pmp.fillRect(s, 0, s, s, c2);
		}
		painter.fillRect(0, 0, size.width(), size.height(), checker);
		painter.fillRect(0, 0, size.width(), size.height(), m_color);
	}
};

//****************************************************************************//

template <class T>
class RegisterColorWidget
{
public:
	template <class U> using ColorPropertyWidget = SimplePropertyWidget<U, ColorChooserWidget<U>>;

	explicit RegisterColorWidget(const std::string& widgetName)
	{
		using vector_type = std::vector<T>;
		RegisterWidget<ColorPropertyWidget<vector_type>> PW_vector(widgetName);
		RegisterWidget<ColorPropertyWidget<VectorWrapper<vector_type>>> PW_vector_wrapper(widgetName);
	}
};

/*****************************************************************************/

RegisterColorWidget<float> PW_color_float("color");
RegisterColorWidget<double> PW_color_double("color");
