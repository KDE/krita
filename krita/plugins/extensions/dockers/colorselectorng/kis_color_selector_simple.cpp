#include "kis_color_selector_simple.h"

#include <QImage>
#include <QPainter>
#include <QColor>
#include "KoColor.h"
#include <cmath>

#include <KDebug>

KisColorSelectorSimple::KisColorSelectorSimple(KisColorSelectorBase *parent) :
    KisColorSelectorComponent(parent),
    m_parameter(KisColorSelector::SL)
{
}

void KisColorSelectorSimple::setConfiguration(Parameter param, Type type)
{
    m_parameter = param;
    m_type = type;
}

void KisColorSelectorSimple::selectColor(int x, int y)
{
}

void KisColorSelectorSimple::paint(QPainter* painter)
{
    QImage tmpDev(width(), height(), QImage::Format_ARGB32_Premultiplied);

    for(int x=0; x<width(); x++) {
        for(int y=0; y<height(); y++) {
//            tmpDev.setPixel(x, y, QColor::fromHslF(0,x/qreal(width()), 1-y/qreal(height())).rgb());
            tmpDev.setPixel(x, y, colorAt(x, y));
        }
    }

    painter->drawImage(0,0, tmpDev);
}

QRgb KisColorSelectorSimple::colorAt(int x, int y)
{
    if(m_type==KisColorSelector::Square || m_type==KisColorSelector::Slider) {
        qreal xRel = x/qreal(width());
        qreal yRel = 1.-y/qreal(height());
        qreal relPos;
        if(height()>width())
            relPos = 1.-y/qreal(height());
        else
            relPos = 1.-x/qreal(width());

        switch(m_parameter) {
        case KisColorSelector::SL:
            return KoColor(QColor::fromHslF(parameter1(), xRel, yRel).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::SV:
            return KoColor(QColor::fromHsvF(parameter1(), xRel, yRel).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::SH:
            return KoColor(QColor::fromHsvF(xRel, yRel, parameter1()).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::VH:
            return KoColor(QColor::fromHsvF(xRel, parameter1(), yRel).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::LH:
            return KoColor(QColor::fromHslF(xRel, parameter1(), yRel).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::H:
            return KoColor(QColor::fromHsvF(relPos, parameter1(), parameter2()).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::S:
            return KoColor(QColor::fromHsvF(parameter1(), relPos, parameter2()).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::V:
            return KoColor(QColor::fromHsvF(parameter1(), parameter2(), relPos).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::L:
            return KoColor(QColor::fromHslF(parameter1(), parameter2(), relPos).rgb(), colorSpace()).toQColor().rgb();
            break;
        default:
            return qRgb(255,0,0);
        }
    }
    else {
        //wheel
        qreal xRel = x-width()/2.;
        qreal yRel = y-height()/2.;

        qreal radius = sqrt(xRel*xRel+yRel*yRel);
        if(radius>qMin(width(), height())/2)
            return qRgba(0,0,0,0);
        radius/=qMin(width(), height())/2.;

        qreal angle = std::atan2(yRel, xRel);
        angle+=M_PI;
        angle/=2*M_PI;


        switch(m_parameter) {
        case KisColorSelector::SH:
            return KoColor(QColor::fromHsvF(angle, radius, parameter1()).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::VH:
            return KoColor(QColor::fromHsvF(angle, parameter1(), radius).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::LH:
            return KoColor(QColor::fromHslF(angle, parameter1(), radius).rgb(), colorSpace()).toQColor().rgb();
            break;
        default:
            return qRgb(255,0,0);
        }
    }
}
