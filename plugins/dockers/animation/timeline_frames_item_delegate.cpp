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

#include "timeline_frames_item_delegate.h"

#include <QPen>
#include <QPainter>
#include <QApplication>

#include "timeline_frames_model.h"
#include "timeline_color_scheme.h"


TimelineFramesItemDelegate::TimelineFramesItemDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

TimelineFramesItemDelegate::~TimelineFramesItemDelegate()
{
}

void TimelineFramesItemDelegate::paintActiveFrameSelector(QPainter *painter, const QRect &rc, bool isCurrentFrame)
{
    QColor lineColor = TimelineColorScheme::instance()->selectorColor();
    const int lineWidth = rc.width() > 20 ? 4 : 2;

    const int x0 = rc.x();
    const int y0 = rc.y();
    const int x1 = rc.right();
    const int y1 = rc.bottom();

    QVector<QLine> linesDark;
    linesDark << QLine(x0 + lineWidth / 2, y0, x0  + lineWidth / 2, y1);
    linesDark << QLine(x1 -  lineWidth / 2 + 1, y0, x1 - lineWidth / 2 + 1, y1);

    QPen oldPen = painter->pen();
    painter->setPen(QPen(lineColor, lineWidth));
    painter->drawLines(linesDark);
    painter->setPen(oldPen);

    if (isCurrentFrame) {
        QPen oldPen = painter->pen();
        QBrush oldBrush(painter->brush());

        painter->setPen(QPen(lineColor, 0));
        painter->setBrush(lineColor);

        painter->drawEllipse(rc.center(), 2,2);

        painter->setBrush(oldBrush);
        painter->setPen(oldPen);
    }
}

void TimelineFramesItemDelegate::drawBackground(QPainter *painter, const QModelIndex &index, const QRect &rc)
{
    bool active = index.data(TimelineFramesModel::ActiveLayerRole).toBool();
    bool present = index.data(TimelineFramesModel::FrameExistsRole).toBool();
    bool editable = index.data(TimelineFramesModel::FrameEditableRole).toBool();

    QColor color = TimelineColorScheme::instance()->frameColor(present, active);

    if (!editable && color.alpha() > 0) {
        const int l = color.lightness();
        color = QColor(l, l, l);
    }

    painter->fillRect(rc, color);
}

void TimelineFramesItemDelegate::drawFocus(QPainter *painter,
                                           const QStyleOptionViewItem &option,
                                           const QRect &rect) const
{
    // copied form Qt 4.8!

    if ((option.state & QStyle::State_HasFocus) == 0 || !rect.isValid())
        return;
    QStyleOptionFocusRect o;
    o.QStyleOption::operator=(option);
    o.rect = rect;
    o.state |= QStyle::State_KeyboardFocusChange;
    o.state |= QStyle::State_Item;
    QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
                              ? QPalette::Normal : QPalette::Disabled;
    o.backgroundColor = option.palette.color(cg, (option.state & QStyle::State_Selected)
                                             ? QPalette::Highlight : QPalette::Window);
    const QWidget *widget = qobject_cast<QWidget*>(parent());
    QStyle *style = widget ? widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter, widget);
}

void TimelineFramesItemDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    drawBackground(painter, index, option.rect);

    if (option.showDecorationSelected &&
        (option.state & QStyle::State_Selected)) {

        QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
            ? QPalette::Normal : QPalette::Disabled;
        if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
            cg = QPalette::Inactive;

        QBrush brush = TimelineColorScheme::instance()->selectionColor();

        int oldOpacity = painter->opacity();
        painter->setOpacity(0.5);
        painter->fillRect(option.rect, brush);
        painter->setOpacity(oldOpacity);
    }

    drawFocus(painter, option, option.rect);

    bool active = index.data(TimelineFramesModel::ActiveFrameRole).toBool();
    bool layerIsCurrent = index.data(TimelineFramesModel::ActiveLayerRole).toBool();
    if (active) {
        paintActiveFrameSelector(painter, option.rect, layerIsCurrent);
    }
}
