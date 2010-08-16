#include "kis_curve_widget.h"

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include "kis_cubic_curve_widget.h"
#include "kis_linear_curve_widget.h"

class KisPaintedCurveWidget : public QWidget {
public:
    KisPaintedCurveWidget(QWidget *parent = 0)
        : QWidget(parent), m_lastPointX(-1)
    {}

protected:
    void paintEvent(QPaintEvent *)
    {
        QPainter painter(this);
//        painter.setMatrix(m_converterMatrix);
        painter.setRenderHint(QPainter::Antialiasing);

        QPainterPath path;
        path.moveTo(0,0);


        for (QMap<int, int>::iterator iter=m_points.begin(); iter!=(m_points.end()); iter++) {
            path.lineTo(iter.key(), iter.value());
        }

        painter.drawPath(path);
    }

    inline void deletePoints(int fromX, int toX)
    {
        int start = qMin(fromX, toX);
        int end = qMax(fromX, toX);

        QList<int> keys = m_points.keys();
        for(int i=0; i<keys.size(); i++) {
            if(keys.at(i)>start && keys.at(i)<end)
                m_points.remove(keys.at(i));
        }
    }

    void mousePressEvent(QMouseEvent *e)
    {
        if(e->button()==Qt::LeftButton) {
            m_lastPointX = e->x();
        }
    }

    void mouseMoveEvent(QMouseEvent *e)
    {
        if(e->buttons()&Qt::LeftButton) {
            deletePoints(m_lastPointX, e->x());
            m_points.insert(e->x(), e->y());
            m_lastPointX=e->x();
            update();
        }
    }

private:
    QMap<int, int> m_points;
    int m_lastPointX;
};

KisCurveWidget::KisCurveWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(new KisPaintedCurveWidget(this));
    layout->addWidget(new KisCubicCurveWidget(this));
    layout->addWidget(new KisLinearCurveWidget(this));

    resize(1500, 500);
}

KisCurveWidget::~KisCurveWidget()
{

}

