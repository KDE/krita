#include "kis_color_selector_type_widget.h"
#include <QGridLayout>

#include "kis_color_selector.h"
#include "kis_canvas2.h"

KisColorSelectorTypeWidget::KisColorSelectorTypeWidget(QWidget* parent) :
        QComboBox(parent),
        m_popup(0)
{
    if(parent==0) {
        //this is the popup
        QGridLayout* layout = new QGridLayout(this);
        layout->setSpacing(15);

        layout->addWidget(new KisColorSelector(this, KisColorSelector::Ring, KisColorSelector::Triangle, KisColorSelector::H, KisColorSelector::SL), 0,0);
        layout->addWidget(new KisColorSelector(this, KisColorSelector::Ring, KisColorSelector::Square, KisColorSelector::H, KisColorSelector::SL), 0,1);
        layout->addWidget(new KisColorSelector(this, KisColorSelector::Ring, KisColorSelector::Square, KisColorSelector::H, KisColorSelector::SV), 0,2);

        layout->addWidget(new KisColorSelector(this, KisColorSelector::Square, KisColorSelector::Slider, KisColorSelector::SV, KisColorSelector::H), 1,0);
        layout->addWidget(new KisColorSelector(this, KisColorSelector::Square, KisColorSelector::Slider, KisColorSelector::VH, KisColorSelector::S), 1,1);
        layout->addWidget(new KisColorSelector(this, KisColorSelector::Square, KisColorSelector::Slider, KisColorSelector::SH, KisColorSelector::V), 1,2);

        layout->addWidget(new KisColorSelector(this, KisColorSelector::Square, KisColorSelector::Slider, KisColorSelector::SL, KisColorSelector::H), 2,0);
        layout->addWidget(new KisColorSelector(this, KisColorSelector::Square, KisColorSelector::Slider, KisColorSelector::LH, KisColorSelector::V), 2,1);
        layout->addWidget(new KisColorSelector(this, KisColorSelector::Square, KisColorSelector::Slider, KisColorSelector::SH, KisColorSelector::L), 2,2);

        layout->addWidget(new KisColorSelector(this, KisColorSelector::Wheel, KisColorSelector::Slider, KisColorSelector::SH, KisColorSelector::V), 3,0);
        layout->addWidget(new KisColorSelector(this, KisColorSelector::Wheel, KisColorSelector::Slider, KisColorSelector::VH, KisColorSelector::S), 3,1);
        layout->addWidget(new KisColorSelector(this, KisColorSelector::Wheel, KisColorSelector::Slider, KisColorSelector::LH, KisColorSelector::S), 3,2);
        layout->addWidget(new KisColorSelector(this, KisColorSelector::Wheel, KisColorSelector::Slider, KisColorSelector::VH, KisColorSelector::L), 3,3);

        setWindowFlags(Qt::Popup);
    }
    else {
        m_popup=new KisColorSelectorTypeWidget();
        setMinimumSize(80,80);
    }
}

KisColorSelectorTypeWidget::~KisColorSelectorTypeWidget()
{
    delete m_popup;
}

void KisColorSelectorTypeWidget::hidePopup()
{
    QComboBox::hidePopup();
    if(m_popup) {
        m_popup->hide();
    }
}

void KisColorSelectorTypeWidget::showPopup()
{
    if(parent()) {
        // only show if this is not the popup
        QComboBox::showPopup();
        m_popup->move(mapToGlobal(QPoint(0,0)));
        m_popup->show();
    }
}

void KisColorSelectorTypeWidget::setCanvas(KisCanvas2 *canvas)
{
    if(parent()) {
        //this is not the popup, but we should set the canvas for all popup selectors
        for(int i=0; i<m_popup->layout()->count(); i++) {
            KisColorSelector* item = dynamic_cast<KisColorSelector*>(m_popup->layout()->itemAt(i)->widget());
            Q_ASSERT(item);
            if(item!=0) {
                item->setCanvas(canvas);
                item->setMaximumSize(120,120);
                item->setMinimumSize(120,120);
            }
        }
    }
}

void KisColorSelectorTypeWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.fillRect(0,0,width(), height(), QColor(128,128,128));
}
