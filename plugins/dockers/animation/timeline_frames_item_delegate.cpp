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
#include "krita_utils.h"
#include "timeline_frames_model.h"
#include "timeline_color_scheme.h"

#include "kis_node_view_color_scheme.h"

TimelineFramesItemDelegate::TimelineFramesItemDelegate(QObject *parent)
    : QItemDelegate(parent)
{
    KisNodeViewColorScheme scm;
    labelColors = scm.allColorLabels();
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

void TimelineFramesItemDelegate::paintSpecialKeyframeIndicator(QPainter *painter, const QModelIndex &index, const QRect &rc) const
{
    bool doesFrameExist = index.data(TimelineFramesModel::FrameExistsRole).toBool();   /// does normal keyframe exist
    bool isEditable = index.data(TimelineFramesModel::FrameEditableRole).toBool();
    bool hasContent = index.data(TimelineFramesModel::FrameHasContent).toBool();     /// find out if frame is empty

    QColor color = qApp->palette().color(QPalette::Highlight);
    QColor baseColor = qApp->palette().color(QPalette::Base);
    QColor noLabelSetColor = qApp->palette().color(QPalette::Highlight); // if no color label is used

    // use color label if it exists. coloring follows very similar logic to the drawBackground() function except this is a bit simpler
    QVariant colorLabel = index.data(TimelineFramesModel::FrameColorLabelIndexRole);
    if (colorLabel.isValid()) {
        color = labelColors.at(colorLabel.toInt());
    } else {
        color = noLabelSetColor;
    }

    if (!isEditable) {
        color = KritaUtils::blendColors(baseColor, color, 0.5);
    }

    if (doesFrameExist && hasContent) {
        color = baseColor; // special keyframe will be over filled in frame, so switch color so it is shown
    }

    QPen oldPen = painter->pen();
    QBrush oldBrush(painter->brush());

    painter->setPen(QPen(color, 0));
    painter->setBrush(color);

    QPointF center = rc.center();
    QPointF points[4] = {
        QPointF(center.x() + 4, center.y()   ),
        QPointF(center.x()    , center.y() - 4),
        QPointF(center.x() - 4, center.y()   ),
        QPointF(center.x()    , center.y() + 4)
    };
    painter->drawConvexPolygon(points, 4);

    painter->setBrush(oldBrush);
    painter->setPen(oldPen);
}

void TimelineFramesItemDelegate::drawBackground(QPainter *painter, const QModelIndex &index, const QRect &rc) const
{
    /// is the current layer actively selected (this is not the same as visibility)
    bool hasActiveLayerRole = index.data(TimelineFramesModel::ActiveLayerRole).toBool();
    bool doesFrameExist = index.data(TimelineFramesModel::FrameExistsRole).toBool();   /// does keyframe exist
    bool isEditable = index.data(TimelineFramesModel::FrameEditableRole).toBool(); /// is layer editable
    bool hasContent = index.data(TimelineFramesModel::FrameHasContent).toBool();     /// find out if frame is empty

    QColor color; // will get re-used for determining color
    QColor noLabelSetColor = qApp->palette().color(QPalette::Highlight); // if no color label is used
    QColor highlightColor = qApp->palette().color(QPalette::Highlight);
    QColor baseColor = qApp->palette().color(QPalette::Base);


    // pass for filling in the active row with slightly color difference
    if (hasActiveLayerRole) {
        color = KritaUtils::blendColors(baseColor, highlightColor, 0.8);
        painter->fillRect(rc, color);
    }

    // assign background color for frame depending on if the frame has a color label or not
    QVariant colorLabel = index.data(TimelineFramesModel::FrameColorLabelIndexRole);
    if (colorLabel.isValid()) {
        color = labelColors.at(colorLabel.toInt());
    } else {
        color = noLabelSetColor;
    }


    // if layer is hidden, make the entire color more subtle.
    // this should be in effect for both fill color and empty outline color
    if (!isEditable) {
        color = KritaUtils::blendColors(baseColor, color, 0.7);
    }


    // how do we fill in a frame that has content
    // a keyframe will be totally filled in. A hold frame will have a line running through it
    if (hasContent && doesFrameExist) {
        painter->fillRect(rc, color);
    }



    // pass of outline for empty keyframes
    if (doesFrameExist && !hasContent) {

        QPen oldPen = painter->pen();
        QBrush oldBrush(painter->brush());

        painter->setPen(QPen(color, 2));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(rc);

        painter->setBrush(oldBrush);
        painter->setPen(oldPen);
    }


    // pass of hold frame line
    if (!doesFrameExist && hasContent) {

        // pretty much the same check as "isValid" above, but that isn't working with hold frames
         if (colorLabel.toInt() == 0) {
             color = noLabelSetColor;

             if (!isEditable) {
                 color = KritaUtils::blendColors(baseColor, color, 0.7);
             }
         }


        QPoint lineStart(rc.x(), (rc.y() + rc.height()/2));
        QPoint lineEnd(rc.x() + rc.width(), (rc.y() + rc.height()/2));

        QPen holdFramePen(color);
        holdFramePen.setWidth(1);

        painter->setPen(holdFramePen);
        painter->drawLine(QLine(lineStart, lineEnd));
    }



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
    // draws background as well as fills normal keyframes
    drawBackground(painter, index, option.rect);

    // creates a semi transparent orange rectangle in the frame that is actively selected on the active row
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

    // not sure what this is drawing
    drawFocus(painter, option, option.rect);

    // opacity keyframe, but NOT normal keyframes
    bool specialKeys = index.data(TimelineFramesModel::SpecialKeyframeExists).toBool();
    if (specialKeys) {
        paintSpecialKeyframeIndicator(painter, index, option.rect);
    }

    // creates a border and dot on the selected frame on the active row
    bool active = index.data(TimelineFramesModel::ActiveFrameRole).toBool();
    bool layerIsCurrent = index.data(TimelineFramesModel::ActiveLayerRole).toBool();
    if (active) {
        paintActiveFrameSelector(painter, option.rect, layerIsCurrent);
    }
}
