/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kis_global.h>
#include "kis_painting_tweaks.h"

#include <QPen>
#include <QRegion>
#include <QPainter>
#include <QTransform>

#include "kis_debug.h"


namespace KisPaintingTweaks {

QRegion safeClipRegion(const QPainter &painter)
{
    const QTransform t = painter.transform();

    QRegion region = t.type() <= QTransform::TxScale ?
        painter.clipRegion() :
        QRegion(painter.clipBoundingRect().toAlignedRect());

    if (region.rectCount() > 1000) {
        qWarning() << "WARNING: KisPaintingTweaks::safeClipRegion: too many rectangles in the region!" << ppVar(region.rectCount());
        region = QRegion(painter.clipBoundingRect().toAlignedRect());
    }

    return region;
}

QRect safeClipBoundingRect(const QPainter &painter)
{
    return painter.clipBoundingRect().toAlignedRect();
}

void initAntsPen(QPen *antsPen, QPen *outlinePen,
                 int antLength, int antSpace)
{
    QVector<qreal> antDashPattern;
    antDashPattern << antLength << antSpace;

    *antsPen = QPen(Qt::CustomDashLine);
    antsPen->setDashPattern(antDashPattern);
    antsPen->setCosmetic(true);
    antsPen->setColor(Qt::black);

    *outlinePen = QPen(Qt::SolidLine);
    outlinePen->setCosmetic(true);
    outlinePen->setColor(Qt::white);
}

PenBrushSaver::PenBrushSaver(QPainter *painter)
    : m_painter(painter),
      m_pen(painter->pen()),
      m_brush(painter->brush())
{
}

PenBrushSaver::PenBrushSaver(QPainter *painter, const QPen &pen, const QBrush &brush)
    : PenBrushSaver(painter)
{
    m_painter->setPen(pen);
    m_painter->setBrush(brush);
}

PenBrushSaver::PenBrushSaver(QPainter *painter, const QPair<QPen, QBrush> &pair)
    : PenBrushSaver(painter)
{
    m_painter->setPen(pair.first);
    m_painter->setBrush(pair.second);
}

PenBrushSaver::PenBrushSaver(QPainter *painter, const QPair<QPen, QBrush> &pair, allow_noop_t)
    : m_painter(painter)
{
    if (m_painter) {
        m_pen = m_painter->pen();
        m_brush = m_painter->brush();
        m_painter->setPen(pair.first);
        m_painter->setBrush(pair.second);
    }
}

PenBrushSaver::~PenBrushSaver()
{
    if (m_painter) {
        m_painter->setPen(m_pen);
        m_painter->setBrush(m_brush);
    }
}

QColor blendColors(const QColor &c1, const QColor &c2, qreal r1)
{
    const qreal r2 = 1.0 - r1;

    return QColor::fromRgbF(
        c1.redF() * r1 + c2.redF() * r2,
        c1.greenF() * r1 + c2.greenF() * r2,
        c1.blueF() * r1 + c2.blueF() * r2);
}

qreal colorDifference(const QColor &c1, const QColor &c2)
{
    const qreal dr = c1.redF() - c2.redF();
    const qreal dg = c1.greenF() - c2.greenF();
    const qreal db = c1.blueF() - c2.blueF();

    return std::sqrt(2 * pow2(dr) + 4 * pow2(dg) + 3 * pow2(db));
}

void dragColor(QColor *color, const QColor &baseColor, qreal threshold)
{
    while (colorDifference(*color, baseColor) < threshold) {

        QColor newColor = *color;

        if (newColor.lightnessF() > baseColor.lightnessF()) {
            newColor = newColor.lighter(120);
        } else {
            newColor = newColor.darker(120);
        }

        if (newColor == *color) {
            break;
        }

        *color = newColor;
    }
}

}
