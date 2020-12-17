/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __TIMELINE_FRAMES_ITEM_DELEGATE_H
#define __TIMELINE_FRAMES_ITEM_DELEGATE_H

#include <QItemDelegate>


class KisAnimTimelineFrameDelegate : public QItemDelegate
{
public:
    KisAnimTimelineFrameDelegate(QObject *parent);
    ~KisAnimTimelineFrameDelegate() override;

    static void paintActiveFrameSelector(QPainter *painter, const QRect &rc, bool isCurrentFrame);

    /// the opacity keyframe
    void paintSpecialKeyframeIndicator(QPainter *painter, const QModelIndex &index, const QRect &rc) const;

    void drawBackground(QPainter *painter, const QModelIndex &index, const QRect &rc) const;
    void drawFocus(QPainter *painter,
                   const QStyleOptionViewItem &option,
                   const QRect &rect) const override;
    void drawCloneGraphics(QPainter *painter, const QRect &rect) const;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QVector<QColor> labelColors;
    QPixmap stripes;
};

#endif /* __TIMELINE_FRAMES_ITEM_DELEGATE_H */
