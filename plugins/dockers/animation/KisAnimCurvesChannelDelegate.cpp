/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAnimCurvesChannelDelegate.h"
#include "KisAnimCurvesChannelsModel.h"
#include "krita_utils.h"
#include "kis_icon_utils.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QFontMetrics>

const int CHANNEL_LEGEND_RADIUS = 6;
const int CHANNEL_ICON_SIZE = 16;

KisAnimCurvesChannelDelegate::KisAnimCurvesChannelDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

QSize KisAnimCurvesChannelDelegate::sizeHint(const QStyleOptionViewItem &styleOption, const QModelIndex &index) const
{
    const bool isCurve = index.data(KisAnimCurvesChannelsModel::CurveRole).toBool();
    if (isCurve) {
        return QStyledItemDelegate::sizeHint(styleOption, index);
    } else {
        return QSize(24, 24);
    }
}

bool KisAnimCurvesChannelDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);

        if (me->button() == Qt::LeftButton) {
            const bool isCurve = index.data(KisAnimCurvesChannelsModel::CurveRole).toBool();
            if (isCurve) {
                const QRect visibilityIcon = option.rect.adjusted(option.rect.width() - CHANNEL_ICON_SIZE, 0, 0, 0);

                if (visibilityIcon.contains(me->pos())) {
                    if (me->modifiers() & Qt::ShiftModifier) {
                        bool currentlyIsolated = index.data(KisAnimCurvesChannelsModel::CurveIsIsolatedRole).toBool();
                        if (currentlyIsolated) {
                            showAllChannels(model, index.parent());
                        } else {
                            soloChannelVisibility(model, index);
                        }
                    } else {
                        bool visible = index.data(KisAnimCurvesChannelsModel::CurveVisibilityRole).toBool();
                        model->setData(index, !visible, KisAnimCurvesChannelsModel::CurveVisibilityRole);
                    }

                    return true;
                }
            }
        }
    }

    return false;
}

void KisAnimCurvesChannelDelegate::paint(QPainter *painter, const QStyleOptionViewItem &styleOption, const QModelIndex &index) const
{
    painter->save();

    const bool isNode = !index.data(KisAnimCurvesChannelsModel::CurveRole).toBool();
    QPalette palette = QApplication::palette();

    // Draw background
    if (isNode) {
        QVariant colorData = index.data(KisAnimCurvesChannelsModel::NodeColorRole);
        KIS_ASSERT(colorData.isValid());

        QColor nodeBGColor = colorData.value<QColor>();
        paintNodeBackground(styleOption, painter, nodeBGColor);
    } else {
        styleOption.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem,
                                                   &styleOption, painter,
                                                   styleOption.widget);
    }

    // Draw layer name..
    QString text = index.data().toString();
    const int iconSpace = isNode ? 0 : CHANNEL_ICON_SIZE;
    QRect textArea = styleOption.rect.adjusted(CHANNEL_LEGEND_RADIUS + 4, 0, -iconSpace, 0);
    text = styleOption.fontMetrics.elidedText(text, styleOption.textElideMode, textArea.width());
    painter->setBrush(palette.buttonText());
    styleOption.widget->style()->drawItemText(painter, textArea, Qt::AlignLeft | Qt::AlignVCenter, styleOption.palette, true, text, QPalette::ButtonText);

    if (isNode) {
        // Draw Open / Close Arrow
        QRect arrow = QRect(styleOption.widget->rect().topLeft(), styleOption.rect.bottomLeft());
        painter->setPen(Qt::NoPen);
        QStyleOptionViewItem item = styleOption;
        item.rect = arrow;
        styleOption.widget->style()->drawPrimitive(QStyle::PE_IndicatorBranch,
                                                   &item, painter,
                                                   styleOption.widget);
    } else {
        QVariant colorData = index.data(KisAnimCurvesChannelsModel::CurveColorRole);
        QColor color = colorData.value<QColor>();

        QPen newPen = QPen(color, CHANNEL_LEGEND_RADIUS);
        newPen.setCapStyle(Qt::RoundCap);
        painter->setPen(newPen);

        if (index.data(KisAnimCurvesChannelsModel::CurveVisibilityRole).toBool()) {
            painter->setBrush(color);
        } else {
            painter->setBrush(QBrush());
        }

        const int y = styleOption.rect.top() + styleOption.rect.height() / 2;
        const QPoint left = QPoint(styleOption.rect.left() - CHANNEL_LEGEND_RADIUS, y);
        const QPoint right = QPoint(styleOption.rect.left(), y);
        painter->drawLine(left, right);
    }

    // Draw buttons..
    if (!isNode) {
        QRect iconArea = styleOption.rect.adjusted(styleOption.rect.width() - iconSpace, 0, 0, 0);
        const bool isVisible = index.data(KisAnimCurvesChannelsModel::CurveVisibilityRole).toBool();
        QIcon visibilityIcon = isVisible ? KisIconUtils::loadIcon("visible") : KisIconUtils::loadIcon("novisible");
        visibilityIcon.paint(painter, iconArea);
    }

    painter->restore();
}

void KisAnimCurvesChannelDelegate::paintNodeBackground(const QStyleOptionViewItem &styleOption, QPainter *painter, const QColor &nodeColor) const
{
    const bool hasValidStyle = styleOption.widget ? styleOption.widget->isEnabled() : (styleOption.state & QStyle::State_Enabled);
    QPalette::ColorGroup cg = hasValidStyle ? QPalette::Normal : QPalette::Disabled;

    QRect viewArea = styleOption.rect;
    viewArea.setLeft(styleOption.widget->rect().left());

    { // Highlight, Shadow and Selection Color
        const QColor highlight = nodeColor.lighter(115);
        const QColor shadow = nodeColor.darker(105);
        painter->fillRect(viewArea, highlight);
        painter->fillRect(viewArea.adjusted(0,6,0,0), shadow);

        if ( (styleOption.state & QStyle::State_Selected)
              && styleOption.widget->style()->proxy()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, &styleOption, styleOption.widget)) {
            painter->fillRect(viewArea, styleOption.palette.brush(cg, QPalette::Highlight));
        }
    }

    { // Center "Neutral" Band
        viewArea -= QMargins(0, 2, 0, 2);
        painter->fillRect(viewArea, nodeColor);
    }
}

void KisAnimCurvesChannelDelegate::soloChannelVisibility(QAbstractItemModel *model, const QModelIndex &channelIndex)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(channelIndex.parent().isValid()); //We need to have a parent "node" to isolate.

    const int numCurves = model->rowCount(channelIndex.parent());
    const int clickedCurve = channelIndex.row();
    const QModelIndex& nodeIndex = channelIndex.parent();

    for (int i = 0; i < numCurves; i++) {
        if (i == clickedCurve) {
            model->setData(channelIndex, true, KisAnimCurvesChannelsModel::CurveVisibilityRole);
        } else {
            QModelIndex indexToToggle = model->index(i, channelIndex.column(), nodeIndex);
            model->setData(indexToToggle, false, KisAnimCurvesChannelsModel::CurveVisibilityRole);
        }
    }
}

void KisAnimCurvesChannelDelegate::showAllChannels(QAbstractItemModel *model, const QModelIndex &nodeIndex )
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(nodeIndex.isValid() && !nodeIndex.parent().isValid()); //We should have no parent node here.

    const int numCurves = model->rowCount(nodeIndex);

    for (int i = 0; i < numCurves; i++) {
        QModelIndex curveIndex = model->index(i, 0, nodeIndex);
        model->setData(curveIndex, true, KisAnimCurvesChannelsModel::CurveVisibilityRole);
    }
}
