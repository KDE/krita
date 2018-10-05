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

#ifndef __TIMELINE_FRAMES_ITEM_DELEGATE_H
#define __TIMELINE_FRAMES_ITEM_DELEGATE_H

#include <QItemDelegate>


class TimelineFramesItemDelegate : public QItemDelegate
{
public:
    TimelineFramesItemDelegate(QObject *parent);
    ~TimelineFramesItemDelegate() override;

    static void paintActiveFrameSelector(QPainter *painter, const QRect &rc, bool isCurrentFrame);

    /// the opacity keyframe
    void paintSpecialKeyframeIndicator(QPainter *painter, const QModelIndex &index, const QRect &rc) const;
    void paintActiveInstanceIndicator(QPainter *painter, const QModelIndex &index, const QRect &rc) const;

    void drawBackground(QPainter *painter, const QModelIndex &index, const QRect &rc, bool hasContentFrame) const;
    void drawFocus(QPainter *painter,
                   const QStyleOptionViewItem &option,
                   const QRect &rect) const override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QVector<QColor> labelColors;

    void drawCycleMarker(QPainter *painter, const QRect &rc, const QColor &fgColor, const QColor &bgColor, int cycleMode) const;
};

#endif /* __TIMELINE_FRAMES_ITEM_DELEGATE_H */
