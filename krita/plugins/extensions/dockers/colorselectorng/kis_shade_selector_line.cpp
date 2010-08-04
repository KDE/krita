#include "kis_shade_selector_line.h"

#include <QPainter>
#include <QColor>
#include <QMouseEvent>

#include <KConfig>
#include <KConfigGroup>
#include <KComponentData>
#include <KGlobal>

#include "KoResourceManager.h"
#include "KoColorSpaceRegistry.h"

#include "kis_canvas2.h"

#include <KDebug>

KisShadeSelectorLine::KisShadeSelectorLine(QWidget *parent) :
    QWidget(parent), m_canvas(0)
{
    setDelta(0, 0, 0);
    updateSettings();
}

KisShadeSelectorLine::KisShadeSelectorLine(qreal hueDelta, qreal satDelta, qreal valDelta, QWidget *parent) :
    QWidget(parent), m_canvas(0)
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

    setMaximumHeight(m_lineHeight);
    setMinimumHeight(m_lineHeight);
}

void KisShadeSelectorLine::setCanvas(KisCanvas2 *canvas)
{
    m_canvas=canvas;

    connect(m_canvas->resourceManager(), SIGNAL(resourceChanged(int, const QVariant&)),
            this,                        SLOT(resourceChanged(int, const QVariant&)), Qt::UniqueConnection);
}

void KisShadeSelectorLine::setLineNumber(int n)
{
    m_lineNumber=n;
}

QString KisShadeSelectorLine::toString() const
{
    return QString("%1|%2|%3|%4").arg(m_lineNumber).arg(m_hueDelta).arg(m_saturationDelta).arg(m_valueDelta);
}

void KisShadeSelectorLine::fromString(const QString& string)
{
    QStringList strili = string.split('|');
    m_lineNumber = strili.at(0).toInt();
    m_hueDelta = strili.at(1).toDouble();
    m_saturationDelta = strili.at(2).toDouble();
    m_valueDelta = strili.at(3).toDouble();
}

void KisShadeSelectorLine::paintEvent(QPaintEvent *)
{
    m_pixelCache = QImage(width(), height(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&m_pixelCache);
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

    QPainter wpainter(this);
    wpainter.drawImage(0,0, m_pixelCache);
}

void KisShadeSelectorLine::mousePressEvent(QMouseEvent* e)
{
    Q_ASSERT(m_canvas);

    QColor color(m_pixelCache.pixel(e->pos()));
    if(color==m_backgroundColor)
        return;

    if(e->button()==Qt::LeftButton)
        m_canvas->resourceManager()->setForegroundColor(KoColor(color, KoColorSpaceRegistry::instance()->rgb8()));

    if(e->button()==Qt::RightButton)
        m_canvas->resourceManager()->setBackgroundColor(KoColor(color, KoColorSpaceRegistry::instance()->rgb8()));

    e->accept();
}

void KisShadeSelectorLine::resourceChanged(int key, const QVariant &v)
{
    if (key == KoCanvasResource::ForegroundColor || key == KoCanvasResource::BackgroundColor) {
        setColor((v.value<KoColor>()).toQColor());
    }
}
