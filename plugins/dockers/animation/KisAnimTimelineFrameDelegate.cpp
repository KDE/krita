/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAnimTimelineFrameDelegate.h"

#include <QPen>
#include <QPainter>
#include <QApplication>
#include <QSvgRenderer>
#include <kis_painting_tweaks.h>
#include "KisAnimTimelineFramesModel.h"
#include "KisAnimTimelineColors.h"

#include "kis_node_view_color_scheme.h"

KisAnimTimelineFrameDelegate::KisAnimTimelineFrameDelegate(QObject *parent)
    : QItemDelegate(parent),
      stripes(64, 64)
{
    KisNodeViewColorScheme scm;
    labelColors = scm.allColorLabels();

    // Clone frame stripes SVG -> Pixmap..
    QImage stripesImage(":diagonal-stripe.svg", "svg");
    stripesImage.save("/tmp/krita_stripes.svg", "svg");
    stripes = QPixmap::fromImage(stripesImage);
}

KisAnimTimelineFrameDelegate::~KisAnimTimelineFrameDelegate()
{
}

void KisAnimTimelineFrameDelegate::paintActiveFrameSelector(QPainter *painter, const QRect &rc, bool isCurrentFrame)
{
    painter->save();

    QColor lineColor = KisAnimTimelineColors::instance()->selectorColor();
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

    painter->restore();
}

void KisAnimTimelineFrameDelegate::paintSpecialKeyframeIndicator(QPainter *painter, const QModelIndex &index, const QRect &rc) const
{
    painter->save();

    bool doesFrameExist = index.data(KisAnimTimelineFramesModel::FrameExistsRole).toBool();   /// does normal keyframe exist
    bool isEditable = index.data(KisAnimTimelineFramesModel::FrameEditableRole).toBool();
    bool hasContent = index.data(KisAnimTimelineFramesModel::FrameHasContent).toBool();     /// find out if frame is empty

    QColor color = qApp->palette().color(QPalette::Highlight);
    QColor baseColor = qApp->palette().color(QPalette::Base);
    QColor noLabelSetColor = qApp->palette().color(QPalette::Highlight); // if no color label is used

    // use color label if it exists. coloring follows very similar logic to the drawBackground() function except this is a bit simpler
    QVariant colorLabel = index.data(KisAnimTimelineFramesModel::FrameColorLabelIndexRole);
    if (colorLabel.isValid()) {
        color = labelColors.at(colorLabel.toInt());
    } else {
        color = noLabelSetColor;
    }

    if (!isEditable) {
        color = KisPaintingTweaks::blendColors(baseColor, color, 0.5);
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

    painter->restore();
}

void KisAnimTimelineFrameDelegate::drawBackground(QPainter *painter, const QModelIndex &index, const QRect &rc) const
{
    painter->save();

    /// is the current layer actively selected (this is not the same as visibility)
    bool hasActiveLayerRole = index.data(KisAnimTimelineFramesModel::ActiveLayerRole).toBool();
    bool doesFrameExist = index.data(KisAnimTimelineFramesModel::FrameExistsRole).toBool();   /// does keyframe exist
    bool isEditable = index.data(KisAnimTimelineFramesModel::FrameEditableRole).toBool(); /// is layer editable
    bool hasContent = index.data(KisAnimTimelineFramesModel::FrameHasContent).toBool();     /// find out if frame is empty

    QColor color; // will get re-used for determining color
    QColor noLabelSetColor = qApp->palette().color(QPalette::Highlight); // if no color label is used
    QColor highlightColor = qApp->palette().color(QPalette::Highlight);
    QColor baseColor = qApp->palette().color(QPalette::Base);


    // pass for filling in the active row with slightly color difference
    if (hasActiveLayerRole) {
        color = KisPaintingTweaks::blendColors(baseColor, highlightColor, 0.8);
        painter->fillRect(rc, color);
    } else {
        color = KisPaintingTweaks::blendColors(baseColor, highlightColor, 0.95);
        painter->fillRect(rc, color);
    }

    // assign background color for frame depending on if the frame has a color label or not
    QVariant colorLabel = index.data(KisAnimTimelineFramesModel::FrameColorLabelIndexRole);
    if (colorLabel.isValid()) {
        color = labelColors.at(colorLabel.toInt());
    } else {
        color = noLabelSetColor;
    }


    // if layer is hidden, make the entire color more subtle.
    // this should be in effect for both fill color and empty outline color
    if (!isEditable) {
        color = KisPaintingTweaks::blendColors(baseColor, color, 0.7);
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
                 color = KisPaintingTweaks::blendColors(baseColor, color, 0.7);
             }
         }


        QPoint lineStart(rc.x(), (rc.y() + rc.height()/2));
        QPoint lineEnd(rc.x() + rc.width(), (rc.y() + rc.height()/2));

        QPen holdFramePen(color);
        holdFramePen.setWidth(1);

        painter->setPen(holdFramePen);
        painter->drawLine(QLine(lineStart, lineEnd));
    }

    painter->restore();
}

void KisAnimTimelineFrameDelegate::drawFocus(QPainter *painter,
                                           const QStyleOptionViewItem &option,
                                           const QRect &rect) const
{
    // copied from Qt 4.8!
    if ((option.state & QStyle::State_HasFocus) == 0 || !rect.isValid())
        return;

    painter->save();


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

    painter->restore();
}

void KisAnimTimelineFrameDelegate::drawCloneGraphics(QPainter *painter, const QRect &rect) const
{
    painter->save();

    QBrush brush(stripes);
    brush.setStyle(Qt::TexturePattern);

    painter->setPen(Qt::NoPen);
    painter->setBrush(brush);
    painter->setOpacity(0.25f);
    painter->drawRect(rect);

    painter->restore();
}

void KisAnimTimelineFrameDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    // draws background as well as fills normal keyframes
    drawBackground(painter, index, option.rect);

    // Clone graphics..
    if (index.data(KisAnimTimelineFramesModel::CloneOfActiveFrame).toBool() && index.data(KisAnimTimelineFramesModel::ActiveLayerRole).toBool()) {
        drawCloneGraphics(painter, option.rect);
    }

    // creates a semi transparent orange rectangle in the frame that is actively selected on the active row
    if (option.showDecorationSelected &&
        (option.state & QStyle::State_Selected)) {
        painter->save();

        const QVariant data = index.data(KisAnimTimelineFramesModel::FrameEditableRole);
        bool isEditable = data.isValid() ? data.toBool() : true;

        QColor highlightColor = KisAnimTimelineColors::instance()->selectionColor();
        highlightColor.setAlpha(isEditable ? 128 : 64);
        QBrush brush = highlightColor;
        painter->fillRect(option.rect, brush);

        painter->restore();
    }

    // not sure what this is drawing
    drawFocus(painter, option, option.rect);

    // opacity keyframe, but NOT normal keyframes
    bool specialKeys = index.data(KisAnimTimelineFramesModel::SpecialKeyframeExists).toBool();
    if (specialKeys) {
        paintSpecialKeyframeIndicator(painter, index, option.rect);
    }

    // creates a border and dot on the selected frame on the active row
    bool active = index.data(KisAnimTimelineFramesModel::ActiveFrameRole).toBool();
    bool layerIsCurrent = index.data(KisAnimTimelineFramesModel::ActiveLayerRole).toBool();
    if (active) {
        paintActiveFrameSelector(painter, option.rect, layerIsCurrent);
    }

    { // Shade over anything that's outside of the animation range...
        if (index.data(KisAnimTimelineFramesModel::WithinClipRange).toBool() == false) {
            painter->save();

            painter->setOpacity(0.50f);
            painter->fillRect(option.rect, qApp->palette().color(QPalette::Base).darker(110));

            painter->restore();
        }
    }
}
