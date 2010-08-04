#include "kis_shade_selector_line_combo_box.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QPainter>
#include <QMouseEvent>

#include "kis_shade_selector_line.h"

class KisShadeSelectorLineComboBoxPrivate : public QWidget {
public:
    int spacing;
    int selectorWidth;
    QRect highlightArea;

    KisShadeSelectorLineComboBoxPrivate(QWidget* parent) :
            QWidget(parent, Qt::Popup),
            spacing(20),
            selectorWidth(100),
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
//                item->setMaximumWidth(selectorWidth);
//                item->setMinimumWidth(selectorWidth);
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
            mouseMoveEvent(e);
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
    m_private(new KisShadeSelectorLineComboBoxPrivate(this)),
    m_currentLine(new KisShadeSelectorLine(0,0,0,this))
{
    QGridLayout* l = new QGridLayout(this);
    l->addWidget(m_currentLine);

    m_currentLine->setEnabled(false);
    m_currentLine->setColor(QColor(190,50,50));

    // 30 pixels for the arrow of the combobox
//    setMinimumWidth(m_private->selectorWidth+m_private->spacing+30);
//    setMaximumWidth(minimumWidth());
//    m_currentLine->setMaximumWidth(m_private->selectorWidth);

//    setMaximumHeight(QWIDGETSIZE_MAX);

    updateSettings();
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
    m_currentLine->fromString(stri);
}

QString KisShadeSelectorLineComboBox::configuration() const
{
    return m_currentLine->toString();
}

void KisShadeSelectorLineComboBox::setLineNumber(int n)
{
    m_currentLine->setLineNumber(n);
    for(int i=0; i<m_private->layout()->count(); i++) {
        KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(m_private->layout()->itemAt(i)->widget());
        Q_ASSERT(item);
        if(item!=0) {
            item->setLineNumber(n);;
        }
    }
}

//QSize KisShadeSelectorLineComboBox::sizeHint() const
//{
//    return minimumSize();
//}

void KisShadeSelectorLineComboBox::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
    m_currentLine->setMaximumWidth(width()-30-m_private->spacing);
    m_private->setMinimumWidth(width());
}

void KisShadeSelectorLineComboBox::updateSettings()
{
    m_currentLine->updateSettings();
    for(int i=0; i<m_private->layout()->count(); i++) {
        KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(m_private->layout()->itemAt(i)->widget());
        Q_ASSERT(item);
        if(item!=0) {
            item->updateSettings();
        }
    }

    setLineHeight(m_currentLine->m_lineHeight);
}


void KisShadeSelectorLineComboBox::setGradient(bool b)
{
    m_currentLine->m_gradient=b;
    for(int i=0; i<m_private->layout()->count(); i++) {
        KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(m_private->layout()->itemAt(i)->widget());
        Q_ASSERT(item);
        if(item!=0) {
            item->m_gradient=b;
        }
    }

    update();
}

void KisShadeSelectorLineComboBox::setPatches(bool b)
{
    m_currentLine->m_gradient=!b;
    for(int i=0; i<m_private->layout()->count(); i++) {
        KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(m_private->layout()->itemAt(i)->widget());
        Q_ASSERT(item);
        if(item!=0) {
            item->m_gradient=!b;
        }
    }

    update();
}

void KisShadeSelectorLineComboBox::setPatchCount(int count)
{
    m_currentLine->m_patchCount=count;
    for(int i=0; i<m_private->layout()->count(); i++) {
        KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(m_private->layout()->itemAt(i)->widget());
        Q_ASSERT(item);
        if(item!=0) {
            item->m_patchCount=count;
        }
    }

    update();
}

void KisShadeSelectorLineComboBox::setLineHeight(int height)
{
    m_currentLine->m_lineHeight=height;
    m_currentLine->setMinimumHeight(height);
    m_currentLine->setMaximumHeight(height);
    setMinimumHeight(height+m_private->spacing);
    setMaximumHeight(height+m_private->spacing);

    for(int i=0; i<m_private->layout()->count(); i++) {
        KisShadeSelectorLine* item = dynamic_cast<KisShadeSelectorLine*>(m_private->layout()->itemAt(i)->widget());
        Q_ASSERT(item);
        if(item!=0) {
            item->m_lineHeight=height;
            item->setMaximumHeight(height);
            item->setMinimumHeight(height);
        }
    }

    update();
}
