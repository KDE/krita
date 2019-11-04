/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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
    QRect iconRect(int logicalIndex, int iconIndex) const;
    int iconAt(int logicalIndex, const QPoint &pt);

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

QSize TimelineLayersHeader::sectionSizeFromContents(int logicalIndex) const
{
    QSize baseSize = QHeaderView::sectionSizeFromContents(logicalIndex);

    baseSize.setWidth(baseSize.width() + 6 + (2 + 16) * m_d->numIcons(logicalIndex));

    return baseSize;
}

QRect TimelineLayersHeader::Private::iconRect(int logicalIndex, int iconIndex) const
{
    QSize sectionSize(q->viewport()->width(), q->sectionSize(logicalIndex));

    const int iconWidth = 16;
    const int iconHeight = 16;

    const int y = (sectionSize.height() - iconHeight) / 2;
    const int x = sectionSize.width() -
        (numIcons(logicalIndex) - iconIndex) * (iconWidth + 2);


    return QRect(x, y, iconWidth, iconHeight);
}

void TimelineLayersHeader::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();

    bool isLayerActive = model()->headerData(logicalIndex, orientation(), TimelineFramesModel::ActiveLayerRole).toBool();

    if (isLayerActive) {
        QColor lineColor = TimelineColorScheme::instance()->activeLayerBackground();
        const int lineWidth = 2;

        QPen oldPen = painter->pen();
        QBrush oldBrush(painter->brush());

        painter->setPen(QPen(lineColor, lineWidth));
        painter->setBrush(lineColor);

        const int x0 = rect.x();
        const int y0 = rect.y();
        const int x1 = rect.right();
        const int y1 = rect.bottom();

        QVector<QLine> lines;
        lines << QLine(x0, y0 + lineWidth / 2, x1, y0 + lineWidth / 2);
        lines << QLine(x0, y1 - lineWidth / 2, x1, y1 - lineWidth / 2);

        painter->drawLines(lines);

        painter->setBrush(oldBrush);
        painter->setPen(oldPen);
    }

    QVariant value =  model()->headerData(logicalIndex, orientation(), TimelineFramesModel::TimelinePropertiesRole);
    TimelineFramesModel::PropertyList props = value.value<TimelineFramesModel::PropertyList>();

    const int numIcons = m_d->numIcons(logicalIndex);
    for (int i = 0; i < numIcons; i++) {
        TimelineFramesModel::Property *p =
            m_d->getPropertyAt(props, i);

        const bool isActive = p->state.toBool();
        QIcon icon = isActive ? p->onIcon : p->offIcon;
        if (!isActive) {
            painter->setOpacity(0.35);
        }
        QRect rc = m_d->iconRect(logicalIndex, i).translated(rect.topLeft());
        icon.paint(painter, rc);
        painter->setOpacity(1.0);
    }
}

int TimelineLayersHeader::Private::iconAt(int logicalIndex, const QPoint &pt)
{
    QPoint sectionTopLeft(0,
                          q->sectionViewportPosition(logicalIndex));

    QPoint localPos = pt - sectionTopLeft;

    for (int i = 0; i < numIcons(logicalIndex); i++) {
        QRect rc = iconRect(logicalIndex, i);

        if (rc.contains(localPos)) {
            return i;
        }
    }

    return -1;
}

bool TimelineLayersHeader::viewportEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::ToolTip: {
        /**
         * Override tooltip if the cursor is over the properties icons.
         */
        QHelpEvent *he = static_cast<QHelpEvent*>(e);
        int logical = logicalIndexAt(he->pos());
        if (logical != -1) {

            const int iconIndex = m_d->iconAt(logical, he->pos());
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

    return QHeaderView::viewportEvent(e);
}

void TimelineLayersHeader::mousePressEvent(QMouseEvent *e)
{
    int logical = logicalIndexAt(e->pos());
    if (logical != -1) {

        const int iconIndex = m_d->iconAt(logical, e->pos());
        if (iconIndex != -1) {

            QVariant value =  model()->headerData(logical, orientation(), TimelineFramesModel::TimelinePropertiesRole);
            TimelineFramesModel::PropertyList props = value.value<TimelineFramesModel::PropertyList>();

            TimelineFramesModel::Property *p =
                m_d->getPropertyAt(props, iconIndex);

            bool currentState = p->state.toBool();
            p->state = !currentState;

            value.setValue(props);

            model()->setHeaderData(logical, orientation(), value, TimelineFramesModel::TimelinePropertiesRole);
            return;

        } else if (e->button() == Qt::RightButton) {
            model()->setHeaderData(logical, orientation(), true, TimelineFramesModel::ActiveLayerRole);
            emit sigRequestContextMenu(e->globalPos());
            return;
        } else if (e->button() == Qt::LeftButton) {
            model()->setHeaderData(logical, orientation(), true, TimelineFramesModel::ActiveLayerRole);
        }
    }

    QHeaderView::mousePressEvent(e);
}


