/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "timeline_layers_header.h"

#include "kis_debug.h"
#include "kis_icon_utils.h"
#include "kis_global.h"

#include <QPainter>
#include <QHelpEvent>
#include <QToolTip>

#include "timeline_frames_model.h"
#include "timeline_color_scheme.h"


struct TimelineLayersHeader::Private
{
    Private(TimelineLayersHeader *_q) : q(_q) {}

    TimelineLayersHeader *q;

    int numIcons(int logicalIndex) const;
    int iconSectionWidth(int layerIndex) const;
    QPoint getSectionLocalPostion(int layerIndex, QPoint widgetPosition) const;
    QRect getSectionRect(int layerIndex) const;
    QRect propertyIconRect(int logicalIndex, int iconIndex) const;
    int propertyIconAt(int logicalIndex, const QPoint &pt);

    TimelineFramesModel::Property* getPropertyAt(TimelineFramesModel::PropertyList &props, int index);
};


TimelineLayersHeader::TimelineLayersHeader(QWidget *parent)
   : QHeaderView(Qt::Vertical, parent),
     m_d(new Private(this))
{
}

TimelineLayersHeader::~TimelineLayersHeader()
{
}

void TimelineLayersHeader::paintSection(QPainter *painter, const QRect &areaRect, int layerIndex) const
{
    QRect remainingArea = areaRect;

    {   // Paint background..
        QColor bgFillColor = palette().color(QPalette::Base);

        QVariant variant = model()->headerData(layerIndex, orientation(), Qt::BackgroundRole);
        if (variant.canConvert<QBrush>()) {
            QBrush brush = qvariant_cast<QBrush>(variant);
            painter->setBrush(brush);
            painter->setPen(Qt::NoPen);
            painter->drawRect(areaRect);

            bgFillColor = brush.color();
        }

        QColor rimlight = bgFillColor.lighter(115);
        painter->setPen(QPen(rimlight, 2));
        painter->setBrush(rimlight);
        painter->drawLine(areaRect.topLeft(), areaRect.topRight());
    }

    // Paint active highlight..
    const bool isLayerActive = model()->headerData(layerIndex, orientation(), TimelineFramesModel::ActiveLayerRole).toBool();

    if (isLayerActive) {
        QColor lineColor = TimelineColorScheme::instance()->activeLayerBackground();
        const int lineWidth = 2;

        painter->setPen(QPen(lineColor, lineWidth));
        painter->setBrush(lineColor);

        QVector<QLine> lines;
        lines << QLine(areaRect.topLeft(), areaRect.topRight()).translated(0,1);
        lines << QLine(areaRect.bottomLeft(), areaRect.bottomRight()).translated(0,-1);
        painter->drawLines(lines);
    }

    // Paint pushpin icon..
    painter->save();
    const bool isPinned = model()->headerData(layerIndex, orientation(), TimelineFramesModel::PinnedToTimelineRole).toBool();
    const uint pinWidth = areaRect.height() - 4;
    QRect pinArea = kisTrimLeft(pinWidth, remainingArea);
    const uint difference = pinArea.height() - pinWidth;
    pinArea.setHeight(pinWidth); // Square to width.
    pinArea.translate(0, difference / 2); // Center.

    QIcon icon = KisIconUtils::loadIcon("krita_tool_reference_images");
    QRect iconRect = pinArea - QMargins(5,5,5,5);

    if (!isPinned) {
        iconRect = pinArea - QMargins(6,4,4,6);
        painter->setOpacity(0.35);
    }

    icon.paint(painter, iconRect);
    painter->restore();

    // Paint layer name..
    const int textSpace = remainingArea.width() - m_d->iconSectionWidth(layerIndex);
    const QRect textArea = kisTrimLeft(textSpace, remainingArea);
    QString text = model()->headerData(layerIndex, orientation(), Qt::DisplayRole).toString();
    text = fontMetrics().elidedText(text, textElideMode(), textArea.width());
    style()->drawItemText(painter, textArea, Qt::AlignLeft | Qt::AlignVCenter, palette(), isEnabled(), text, QPalette::ButtonText);

    // Paint property (hidden, alpha inherit, etc.) icons..
    QVariant value =  model()->headerData(layerIndex, orientation(), TimelineFramesModel::TimelinePropertiesRole);
    TimelineFramesModel::PropertyList props = value.value<TimelineFramesModel::PropertyList>();

    const int numIcons = m_d->numIcons(layerIndex);
    for (int i = 0; i < numIcons; i++) {
        TimelineFramesModel::Property *prop = m_d->getPropertyAt(props, i);

        const bool isActive = prop->state.toBool();
        QIcon icon = isActive ? prop->onIcon : prop->offIcon;
        if (!isActive) {
            painter->setOpacity(0.35);
        }
        QRect iconRect = m_d->propertyIconRect(layerIndex, i).translated(areaRect.topLeft());
        icon.paint(painter, iconRect);
        painter->setOpacity(1.0);
    }
}

TimelineFramesModel::Property*
TimelineLayersHeader::Private::getPropertyAt(TimelineFramesModel::PropertyList &props, int index)
{
    int logical = 0;
    for (int i = 0; i < props.size(); i++) {
        if (props[i].isMutable) {
            if (logical == index) {
                return &props[i];
            }

            logical++;
        }
    }

    return 0;
}

int TimelineLayersHeader::Private::numIcons(int logicalIndex) const
{
    int result = 0;

    QVariant value =  q->model()->headerData(logicalIndex, q->orientation(), TimelineFramesModel::TimelinePropertiesRole);
    if (value.isValid()) {
        TimelineFramesModel::PropertyList props = value.value<TimelineFramesModel::PropertyList>();

        Q_FOREACH (const TimelineFramesModel::Property &p, props) {
            if (p.isMutable) {
                result++;
            }
        }
    }

    return result;
}

int TimelineLayersHeader::Private::iconSectionWidth(int layerIndex) const
{
    const int iconSize = 16;
    const int iconPadding = 2;
    return (iconSize + iconPadding) * numIcons(layerIndex);
}

QPoint TimelineLayersHeader::Private::getSectionLocalPostion(int layerIndex, QPoint widgetPosition) const
{
    QPoint sectionTopLeft(0, q->sectionViewportPosition(layerIndex));
    return widgetPosition - sectionTopLeft;
}

QSize TimelineLayersHeader::sectionSizeFromContents(int layerIndex) const
{
    QSize baseSize = QHeaderView::sectionSizeFromContents(layerIndex);
    const int pinSpace = baseSize.height() - 4;
    const int padding = 8;

    baseSize.setWidth(baseSize.width() + pinSpace
                      + m_d->iconSectionWidth(layerIndex) + padding);
    return baseSize;
}

QRect TimelineLayersHeader::Private::getSectionRect(int layerIndex) const
{
    QSize sectionSize(q->viewport()->width(), q->sectionSize(layerIndex));
    QPoint sectionTopLeft(0, q->sectionViewportPosition(layerIndex));
    return QRect(sectionTopLeft, sectionSize);
}

QRect TimelineLayersHeader::Private::propertyIconRect(int logicalIndex, int iconIndex) const
{
    QSize sectionSize(q->viewport()->width(), q->sectionSize(logicalIndex));

    const int iconWidth = 16;
    const int iconHeight = 16;

    const int y = (sectionSize.height() - iconHeight) / 2;
    const int x = sectionSize.width() -
        (numIcons(logicalIndex) - iconIndex) * (iconWidth + 2);

    return QRect(x, y, iconWidth, iconHeight);
}

int TimelineLayersHeader::Private::propertyIconAt(int logicalIndex, const QPoint &pt)
{
    QPoint localPos = getSectionLocalPostion(logicalIndex, pt);

    for (int i = 0; i < numIcons(logicalIndex); i++) {
        QRect rc = propertyIconRect(logicalIndex, i);

        if (rc.contains(localPos)) {
            return i;
        }
    }

    return -1;
}

bool TimelineLayersHeader::viewportEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::ToolTip: {
        /**
         * Override tooltip if the cursor is over the properties icons.
         */
        QHelpEvent *he = static_cast<QHelpEvent*>(event);
        int logical = logicalIndexAt(he->pos());
        if (logical != -1) {

            const int iconIndex = m_d->propertyIconAt(logical, he->pos());
            if (iconIndex != -1) {

                QVariant value =  model()->headerData(logical, orientation(), TimelineFramesModel::TimelinePropertiesRole);
                TimelineFramesModel::PropertyList props = value.value<TimelineFramesModel::PropertyList>();

                TimelineFramesModel::Property *p =
                    m_d->getPropertyAt(props, iconIndex);

                QString text = QString("%1 (%2)")
                    .arg(p->name)
                    .arg(p->state.toBool() ? "on" : "off");

                QToolTip::showText(he->globalPos(), text, this);
                return true;
            }
        }
        break; }
    default:
        break;
    }

    return QHeaderView::viewportEvent(event);
}

void TimelineLayersHeader::mousePressEvent(QMouseEvent *e)
{
    int layerIndex = logicalIndexAt(e->pos());
    if (layerIndex != -1) {
        // Handle pin click..
        QRect layerHeaderRect = m_d->getSectionRect(layerIndex);
        const uint pinWidth = layerHeaderRect.height() - 4;
        QRect pinArea = kisTrimLeft(pinWidth, layerHeaderRect);
        if (pinArea.contains(e->pos())) {
            const bool isPinned = model()->headerData(layerIndex, orientation(), TimelineFramesModel::PinnedToTimelineRole).toBool();
            model()->setHeaderData(layerIndex, orientation(), !isPinned, TimelineFramesModel::PinnedToTimelineRole);
            return;
        }

        // Handle property click..
        const int propertyIconIndex = m_d->propertyIconAt(layerIndex, e->pos());
        if (propertyIconIndex != -1) {
            QVariant value =  model()->headerData(layerIndex, orientation(), TimelineFramesModel::TimelinePropertiesRole);
            TimelineFramesModel::PropertyList props = value.value<TimelineFramesModel::PropertyList>();

            TimelineFramesModel::Property *p =
                m_d->getPropertyAt(props, propertyIconIndex);

            bool currentState = p->state.toBool();
            p->state = !currentState;

            value.setValue(props);

            model()->setHeaderData(layerIndex, orientation(), value, TimelineFramesModel::TimelinePropertiesRole);
            return;

        } else if (e->button() == Qt::RightButton) {
            model()->setHeaderData(layerIndex, orientation(), true, TimelineFramesModel::ActiveLayerRole);
            emit sigRequestContextMenu(e->globalPos());
            return;
        } else if (e->button() == Qt::LeftButton) {
            model()->setHeaderData(layerIndex, orientation(), true, TimelineFramesModel::ActiveLayerRole);
        }
    }

    QHeaderView::mousePressEvent(e);
}
