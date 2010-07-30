#include "kis_shade_selector_line_combo_box.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QMouseEvent>

#include "kis_shade_selector_line.h"

class KisShadeSelectorLineComboBoxPrivate : public QWidget {
public:
    int spacing;
    int selectorWidth;
    int selectorHeight;
    QRect highlightArea;

    KisShadeSelectorLineComboBoxPrivate(QWidget* parent) :
            QWidget(parent, Qt::Popup),
            spacing(20),
            selectorWidth(100),
            selectorHeight(20),
            highlightArea(-1,-1,0,0)
    {
        setMouseTracking(true);

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setSpacing(spacing);

        layout->addWidget(new KisShadeSelectorLine(0.2, 0.0, 0.0, this));
        layout->addWidget(new KisShadeSelectorLine(0.0, 0.5, 0.0, this));
        layout->addWidget(new KisShadeSelectorLine(0.0, 1.0, 0.0, this));
        layout->addWidget(new KisShadeSelectorLine(0.0, 0.0, 0.5, this));
        layout->addWidget(new KisShadeSelectorLine(0.0, 0.0, 1.0, this));


        for(int i=0; i<this->layout()->count(); i++) {
            KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(this->layout()->itemAt(i)->widget());
            Q_ASSERT(item);
            if(item!=0) {
                item->setMaximumSize(selectorWidth, selectorHeight);
                item->setMinimumSize(selectorWidth, selectorHeight);
                item->setMouseTracking(true);
                item->setEnabled(false);
                item->setColor(QColor(190,50,50));
            }
        }
    }

protected:
    void paintEvent(QPaintEvent *)
    {
        QPainter painter(this);
        painter.fillRect(0,0,width(), height(), QColor(128,128,128));
        painter.fillRect(highlightArea, palette().highlight());
    }

    void mouseMoveEvent(QMouseEvent * e)
    {
        if(rect().contains(e->pos())) {
            for(int i=0; i<layout()->count(); i++) {
                KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(layout()->itemAt(i)->widget());
                Q_ASSERT(item);

                if(item->geometry().adjusted(-spacing/2, -spacing/2, spacing/2, spacing/2).contains(e->pos())) {
                    QRect oldArea=highlightArea;
                    highlightArea=item->geometry().adjusted(-spacing/2, -spacing/2, spacing/2, spacing/2);
                    m_lastActiveConfiguration=item->toString();
                    update(highlightArea);
                    update(oldArea);
                }
            }
        }
        else {
            highlightArea.setRect(-1,-1,0,0);
        }
    }

    void mousePressEvent(QMouseEvent* e)
    {
        if(rect().contains(e->pos())) {
            KisShadeSelectorLineComboBox* parent = dynamic_cast<KisShadeSelectorLineComboBox*>(this->parent());
            Q_ASSERT(parent);
            parent->setConfiguration(m_lastActiveConfiguration);
        }
        hide();
        e->accept();
    }
    QString m_lastActiveConfiguration;
};

KisShadeSelectorLineComboBox::KisShadeSelectorLineComboBox(QWidget *parent) :
    QComboBox(parent),
    m_private(new KisShadeSelectorLineComboBoxPrivate(this))
{
}

void KisShadeSelectorLineComboBox::hidePopup()
{
    QComboBox::hidePopup();
    m_private->hide();
}

void KisShadeSelectorLineComboBox::showPopup()
{
    // only show if this is not the popup
    QComboBox::showPopup();
    m_private->move(mapToGlobal(QPoint(0,0)));
    m_private->show();
}

void KisShadeSelectorLineComboBox::setConfiguration(const QString &stri)
{

}
