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

#include "timeline_frames_model_base.h"

struct TimelineLayersHeader::Private
{
    Private(TimelineLayersHeader *_q) : q(_q) {}

    TimelineLayersHeader *q;

    int numIcons(int logicalIndex) const;
    QRect iconRect(int logicalIndex, int iconIndex) const;
    int iconAt(int logicalIndex, const QPoint &pt);
};


TimelineLayersHeader::TimelineLayersHeader(QWidget *parent)
   : QHeaderView(Qt::Vertical, parent),
     m_d(new Private(this))
{
    connect(this, SIGNAL(sectionPressed(int)), SLOT(slotActivateSection(int)));
}

TimelineLayersHeader::~TimelineLayersHeader()
{
}

int TimelineLayersHeader::Private::numIcons(int logicalIndex) const
{
    int result = 0;

    QVariant value =  q->model()->headerData(logicalIndex, q->orientation(), TimelineFramesModelBase::PropertiesRole);
    if (value.isValid()) {
        TimelineFramesModelBase::PropertyList props = value.value<TimelineFramesModelBase::PropertyList>();
        result = props.size();
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
    KIS_ASSERT_RECOVER(iconIndex >= 0 && iconIndex < 5) { return QRect(); }

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

    QVariant value =  model()->headerData(logicalIndex, orientation(), TimelineFramesModelBase::PropertiesRole);
    TimelineFramesModelBase::PropertyList props = value.value<TimelineFramesModelBase::PropertyList>();

    for (int i = 0; i < props.size(); i++) {
        const bool isActive = props[i].state.toBool();

        QIcon icon = isActive ? props[i].onIcon : props[i].offIcon;
        QRect rc = m_d->iconRect(logicalIndex, i).translated(rect.topLeft());
        icon.paint(painter, rc);
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

                QVariant value =  model()->headerData(logical, orientation(), TimelineFramesModelBase::PropertiesRole);
                TimelineFramesModelBase::PropertyList props = value.value<TimelineFramesModelBase::PropertyList>();

                QString text = QString("%1 (%2)")
                    .arg(props[iconIndex].name)
                    .arg(props[iconIndex].state.toBool() ? "on" : "off");

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

            QVariant value =  model()->headerData(logical, orientation(), TimelineFramesModelBase::PropertiesRole);
            TimelineFramesModelBase::PropertyList props = value.value<TimelineFramesModelBase::PropertyList>();

            bool currentState = props[iconIndex].state.toBool();
            props[iconIndex].state = !currentState;

            value.setValue(props);

            model()->setHeaderData(logical, orientation(), value, TimelineFramesModelBase::PropertiesRole);
            return;

        } else if (e->button() == Qt::RightButton) {
            model()->setHeaderData(logical, orientation(), true, TimelineFramesModelBase::ActiveLayerRole);
            emit sigRequestContextMenu(e->globalPos());
            return;
        }
    }

    QHeaderView::mousePressEvent(e);
}

void TimelineLayersHeader::slotActivateSection(int logicalIndex)
{
    model()->setHeaderData(logicalIndex, orientation(), true, TimelineFramesModelBase::ActiveLayerRole);
}


