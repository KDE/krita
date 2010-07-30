#include "kis_shade_selector_line.h"

#include <QPainter>
#include <QColor>

#include <KConfig>
#include <KConfigGroup>
#include <KComponentData>
#include <KGlobal>

KisShadeSelectorLine::KisShadeSelectorLine(qreal hueDelta, qreal satDelta, qreal valDelta, QWidget *parent) :
    QWidget(parent)
{
    setDelta(hueDelta, satDelta, valDelta);

    updateSettings();
}

void KisShadeSelectorLine::setDelta(qreal hue, qreal sat, qreal val)
{
    m_hueDelta = hue;
    m_saturationDelta = sat;
    m_valueDelta = val;
}

void KisShadeSelectorLine::setColor(const QColor &color)
{
    m_color=color;
    update();
}

void KisShadeSelectorLine::updateSettings()
{
    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");

    m_gradient = cfg.readEntry("minimalShadeSelectorAsGradient", false);
    m_patchCount = cfg.readEntry("minimalShadeSelectorPatchCount", 10);
    m_lineHeight = cfg.readEntry("minimalShadeSelectorLineHeight", 20);
    m_backgroundColor = QColor(128, 128, 128);
}

void KisShadeSelectorLine::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(0,0, width(), height(), m_backgroundColor);
    if(m_gradient) {
        QColor c1;
        qreal hue1 = m_color.hueF()-m_hueDelta;
        if(hue1<0) hue1+=1.;
        if(hue1>1) hue1-=1.;
        c1.setHsvF(hue1,
                   qBound(0., m_color.saturationF()-m_saturationDelta, 1.),
                   qBound(0., m_color.valueF()-m_valueDelta, 1.));

        QColor c2;
        qreal hue2 = m_color.hueF()+m_hueDelta;
        if(hue2<0) hue2+=1.;
        if(hue2>1) hue2-=1.;
        c2.setHsvF(hue2,
                   qBound(0., m_color.saturationF()+m_saturationDelta, 1.),
                   qBound(0., m_color.valueF()+m_valueDelta, 1.));

        QLinearGradient gradient(0,0, width(), 0);
        gradient.setColorAt(0, c1);
        gradient.setColorAt(0.5, m_color);
        gradient.setColorAt(1, c2);

        painter.fillRect(0,0,width(), m_lineHeight, QBrush(gradient));
    }
    else {
        qreal patchWidth = (width()-2*m_patchCount)/qreal(m_patchCount);
        qreal hueStep=m_hueDelta/qreal(m_patchCount);
        qreal saturationStep=m_saturationDelta/qreal(m_patchCount);
        qreal valueStep=m_valueDelta/qreal(m_patchCount);

        int z=0;
        for(int i=-m_patchCount/2; i<=m_patchCount/2; i++) {
            if(i==0 && m_patchCount%2==0) continue;

            qreal hue=m_color.hueF()+(i*hueStep);
            if(hue<0) hue+=1.;
            if(hue>1) hue-=1.;

            qreal saturation = qBound(0., m_color.saturationF()+(i*saturationStep), 1.);

            qreal value = qBound(0., m_color.valueF()+(i*valueStep), 1.);


            painter.fillRect(z*(patchWidth+2)+1, 0, patchWidth, m_lineHeight, QColor::fromHsvF(hue, saturation, value));
            z++;
        }
    }
}
