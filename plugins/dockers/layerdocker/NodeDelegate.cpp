/*
  Copyright (c) 2006 Gábor Lehel <illissius@gmail.com>
  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
  Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
#include "kis_config.h"
#include "NodeDelegate.h"
#include "kis_node_model.h"
#include "NodeToolTip.h"
#include "NodeView.h"
#include "KisPart.h"
#include "input/kis_input_manager.h"

#include <QtDebug>
#include <QApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QModelIndex>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QStyle>
#include <QStyleOptionViewItem>

#include <klocalizedstring.h>
#include "kis_node_view_color_scheme.h"
#include "kis_icon_utils.h"
#include "kis_layer_properties_icons.h"
#include "krita_utils.h"
#include "kis_config_notifier.h"

typedef KisBaseNode::Property* OptionalProperty;

#include <kis_base_node.h>

class NodeDelegate::Private
{
public:
    Private() : view(0), edit(0) { }

    NodeView *view;
    QPointer<QWidget> edit;
    NodeToolTip tip;

    QColor checkersColor1;
    QColor checkersColor2;

    QList<OptionalProperty> rightmostProperties(const KisBaseNode::PropertyList &props) const;
    int numProperties(const QModelIndex &index) const;
    OptionalProperty findProperty(KisBaseNode::PropertyList &props, const OptionalProperty &refProp) const;
    OptionalProperty findVisibilityProperty(KisBaseNode::PropertyList &props) const;

    void toggleProperty(KisBaseNode::PropertyList &props, OptionalProperty prop, bool controlPressed, const QModelIndex &index);
};

NodeDelegate::NodeDelegate(NodeView *view, QObject *parent)
    : QAbstractItemDelegate(parent)
    , d(new Private)
{
    d->view = view;

    QApplication::instance()->installEventFilter(this);
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
    slotConfigChanged();
}

NodeDelegate::~NodeDelegate()
{
    delete d;
}

QSize NodeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    KisNodeViewColorScheme scm;
    return QSize(option.rect.width(), scm.rowHeight());
}

void NodeDelegate::paint(QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &index) const
{
    p->save();

    {
        QStyleOptionViewItem option = getOptions(o, index);
        QStyle *style = option.widget ? option.widget->style() : QApplication::style();
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, p, option.widget);

        bool shouldGrayOut = index.data(KisNodeModel::ShouldGrayOutRole).toBool();
        if (shouldGrayOut) {
            option.state &= ~QStyle::State_Enabled;
        }

        p->setFont(option.font);
        drawColorLabel(p, option, index);
        drawFrame(p, option, index);
        drawThumbnail(p, option, index);
        drawText(p, option, index); // BUG: Creating group moves things around (RTL-layout alignment)
        drawIcons(p, option, index);
        drawVisibilityIconHijack(p, option, index); // TODO hide when dragging
        drawDecoration(p, option, index);
        drawExpandButton(p, option, index);
        drawBranch(p, option, index);

        drawProgressBar(p, option, index);
    }
    p->restore();
}

void NodeDelegate::drawBranch(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QModelIndex tmp = index.parent();

    // there is no indention if we have no parent group, so don't draw a branch
    if (!tmp.isValid()) return;

    KisNodeViewColorScheme scm;

    int rtlNum = (option.direction == Qt::RightToLeft) ? 1 : -1;

    QRect baseRect = scm.relThumbnailRect();

    // Move to current index
    baseRect.moveTop(option.rect.topLeft().y());
    // Move to correct location.
    if (option.direction == Qt::RightToLeft) {
        baseRect.moveLeft(option.rect.topRight().x());
    } else {
        baseRect.moveRight(option.rect.topLeft().x());
    }

    QPoint base = baseRect.adjusted(rtlNum*scm.indentation(), 0,
                                    rtlNum*scm.indentation(), 0).center() + QPoint(0, scm.iconSize()/4);

    QPen oldPen = p->pen();
    const qreal oldOpacity = p->opacity(); // remember previous opacity
    p->setOpacity(1.0);

    QColor color = scm.gridColor(option, d->view);
    QColor bgColor = option.state & QStyle::State_Selected ?
        qApp->palette().color(QPalette::Base) :
        qApp->palette().color(QPalette::Text);
    color = KritaUtils::blendColors(color, bgColor, 0.9);

    // TODO: if we are a mask type, use dotted lines for the branch style
    // p->setPen(QPen(p->pen().color(), 2, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin));
    p->setPen(QPen(color, 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    QPoint p2 = base - QPoint(rtlNum*(scm.iconSize()/2), 0);
    QPoint p3 = base - QPoint(0, scm.iconSize()/2);
    p->drawLine(base, p2);
    p->drawLine(base, p3);

     // draw parent lines (keep drawing until x position is less than 0
     QPoint parentBase1 = base + QPoint(rtlNum*scm.indentation(), 0);
     QPoint parentBase2 = p3 + QPoint(rtlNum*scm.indentation(), 0);

     // indent lines needs to be very subtle to avoid making the docker busy looking
     color = KritaUtils::blendColors(color, bgColor, 0.9); // makes it a little lighter than L lines
     p->setPen(QPen(color, 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

     if (tmp.isValid()) {
         tmp = tmp.parent(); // Ignore the first group as it was already painted
     }
     while (tmp.isValid()) {
         p->drawLine(parentBase1, parentBase2);

         parentBase1 += QPoint(rtlNum*scm.indentation(), 0);
         parentBase2 += QPoint(rtlNum*scm.indentation(), 0);

         tmp = tmp.parent();
     }

     p->setPen(oldPen);
     p->setOpacity(oldOpacity);
}

void NodeDelegate::drawColorLabel(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    KisNodeViewColorScheme scm;
    const int label = index.data(KisNodeModel::ColorLabelIndexRole).toInt();
    QColor color = scm.colorLabel(label);
    if (color.alpha() <= 0) return;

    QColor bgColor = qApp->palette().color(QPalette::Base);
    if ((option.state & QStyle::State_MouseOver) && !(option.state & QStyle::State_Selected)) {
        color = KritaUtils::blendColors(color, bgColor, 0.6);
    } else {
        color = KritaUtils::blendColors(color, bgColor, 0.3);
    }

    QRect optionRect = option.rect.adjusted(0, 0, scm.indentation(), 0);
    if (option.state & QStyle::State_Selected) {
        optionRect = iconsRect(option, index);
    }

    if (option.direction == Qt::RightToLeft) {
        optionRect.moveLeft(option.rect.topLeft().x());
    } else {
        optionRect.moveRight(option.rect.topRight().x());
    }

    p->fillRect(optionRect, color);
}

void NodeDelegate::drawFrame(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    KisNodeViewColorScheme scm;

    QPen oldPen = p->pen();
    p->setPen(scm.gridColor(option, d->view));

    const QRect visibilityRect = visibilityClickRect(option, index);
    const QRect thumbnailRect  = thumbnailClickRect(option, index);
    const QRect decorationRect = decorationClickRect(option, index);
    const QRect iconsRectR     = iconsRect(option, index);

    const float topY = thumbnailRect.topLeft().y();
    const float bottomY = thumbnailRect.bottomLeft().y();

    QPoint bottomLeftPoint;
    QPoint bottomRightPoint;
    if (option.direction == Qt::RightToLeft) {
        bottomLeftPoint = iconsRectR.bottomLeft();
        bottomRightPoint = visibilityRect.bottomRight();
    } else {
        bottomLeftPoint = visibilityRect.bottomLeft();
        bottomRightPoint = iconsRectR.bottomRight();
    }

    // bottom running horizontal line
    p->drawLine(bottomLeftPoint.x(), bottomY,
                bottomRightPoint.x(), bottomY);

    // visiblity icon vertical line - left
    p->drawLine(visibilityRect.topLeft().x()-1, topY,
                visibilityRect.bottomLeft().x()-1, bottomY);

    // visiblity icon vertical line - right
    p->drawLine(visibilityRect.topRight().x()+1, topY,
                visibilityRect.bottomRight().x()+1, bottomY);

    // thumbnail vertical line - left
    p->drawLine(thumbnailRect.topLeft().x(), topY,
                thumbnailRect.bottomLeft().x(), bottomY);

    // thumbnail vertical line - right
    p->drawLine(thumbnailRect.topRight().x(), topY,
                thumbnailRect.bottomRight().x(), bottomY);

    // decoration vertical line - left
    p->drawLine(decorationRect.topLeft().x(), topY,
                decorationRect.bottomLeft().x(), bottomY);

    // decoration vertical line - right
    p->drawLine(decorationRect.topRight().x(), topY,
                decorationRect.bottomRight().x(), bottomY);

    // icons' lines are drawn by drawIcons

    //// For debugging purposes only
    p->setPen(Qt::blue);
    //KritaUtils::renderExactRect(p, iconsRectR);
    //KritaUtils::renderExactRect(p, textRect(option, index));
    //KritaUtils::renderExactRect(p, visibilityRect);

    p->setPen(oldPen);
}

QRect NodeDelegate::thumbnailClickRect(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    KisNodeViewColorScheme scm;

    QRect rc = scm.relThumbnailRect();

    // Move to current index
    rc.moveTop(option.rect.topLeft().y());
    // Move to correct location.
    if (option.direction == Qt::RightToLeft) {
        rc.moveLeft(option.rect.topRight().x());
    } else {
        rc.moveRight(option.rect.topLeft().x());
    }

    return rc;
}

void NodeDelegate::drawThumbnail(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    KisNodeViewColorScheme scm;

    const int thumbSize = scm.thumbnailSize();
    const qreal oldOpacity = p->opacity(); // remember previous opacity

    QImage img = index.data(int(KisNodeModel::BeginThumbnailRole) + thumbSize).value<QImage>();
    if (!(option.state & QStyle::State_Enabled)) {
        p->setOpacity(0.35);
    }

    QRect fitRect = thumbnailClickRect(option, index);
    // Shrink to icon rect
    fitRect = kisGrowRect(fitRect, -(scm.thumbnailMargin()+scm.border()));

    // paint in a checkerboard pattern behind the layer contents to represent transparent
    const int step = scm.thumbnailSize() / 6;
    QImage checkers(2 * step, 2 * step, QImage::Format_ARGB32);
    QPainter gc(&checkers);
    gc.fillRect(QRect(0, 0, step, step), d->checkersColor1);
    gc.fillRect(QRect(step, 0, step, step), d->checkersColor2);
    gc.fillRect(QRect(step, step, step, step), d->checkersColor1);
    gc.fillRect(QRect(0, step, step, step), d->checkersColor2);

    QBrush brush(checkers);
    p->fillRect(fitRect, brush);

    p->drawImage(fitRect, img);
    p->setOpacity(oldOpacity); // restore old opacity

    QRect borderRect = kisGrowRect(fitRect, 1);
    KritaUtils::renderExactRect(p, borderRect, scm.gridColor(option, d->view));
}

QRect NodeDelegate::iconsRect(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    KisNodeViewColorScheme scm;

    int propCount = d->numProperties(index);

    const int iconsWidth =
        propCount * (scm.iconSize() + 2 * scm.iconMargin()) +
        (propCount + 1) * scm.border();

    QRect fitRect = QRect(0, 0,
                          iconsWidth, scm.rowHeight() - scm.border());
    // Move to current index
    fitRect.moveTop(option.rect.topLeft().y());
    // Move to correct location.
    if (option.direction == Qt::RightToLeft) {
        fitRect.moveLeft(option.rect.topLeft().x());
    } else {
        fitRect.moveRight(option.rect.topRight().x());
    }

    return fitRect;
}

QRect NodeDelegate::textRect(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    KisNodeViewColorScheme scm;

    static QFont f;
    static int minbearing = 1337 + 666; //can be 0 or negative, 2003 is less likely
    if (minbearing == 2003 || f != option.font) {
        f = option.font; //getting your bearings can be expensive, so we cache them
        minbearing = option.fontMetrics.minLeftBearing() + option.fontMetrics.minRightBearing();
    }

    const QRect decoRect = decorationClickRect(option, index);
    const QRect iconRect = iconsRect(option, index);

    QRect rc = QRect((option.direction == Qt::RightToLeft) ? iconRect.topRight() : decoRect.topRight(),
                     (option.direction == Qt::RightToLeft) ? decoRect.bottomLeft() : iconRect.bottomLeft());
    rc.adjust(-(scm.border()+minbearing), 0,
               (scm.border()+minbearing), 0);

    return rc;
}

void NodeDelegate::drawText(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    KisNodeViewColorScheme scm;
    const QRect rc = textRect(option, index).adjusted(scm.textMargin(), 0,
                                                      -scm.textMargin(), 0);

    QPen oldPen = p->pen();
    const qreal oldOpacity = p->opacity(); // remember previous opacity

    p->setPen(option.palette.color(QPalette::Active,QPalette::Text ));

    if (!(option.state & QStyle::State_Enabled)) {
        p->setOpacity(0.55);
    }

    const QString text = index.data(Qt::DisplayRole).toString();
    const QString elided = p->fontMetrics().elidedText(text, Qt::ElideRight, rc.width());
    p->drawText(rc, Qt::AlignLeft | Qt::AlignVCenter, elided);

    p->setPen(oldPen); // restore pen settings
    p->setOpacity(oldOpacity);
}

QList<OptionalProperty> NodeDelegate::Private::rightmostProperties(const KisBaseNode::PropertyList &props) const
{
    QList<OptionalProperty> list;
    QList<OptionalProperty> prependList;
    list << OptionalProperty(0);
    list << OptionalProperty(0);
    list << OptionalProperty(0);

    KisBaseNode::PropertyList::const_iterator it = props.constBegin();
    KisBaseNode::PropertyList::const_iterator end = props.constEnd();
    for (; it != end; ++it) {
        if (!it->isMutable) continue;

        if (it->id == KisLayerPropertiesIcons::visible.id()) {
            // noop...
        } else if (it->id == KisLayerPropertiesIcons::locked.id()) {
            list[0] = OptionalProperty(&(*it));
        } else if (it->id == KisLayerPropertiesIcons::inheritAlpha.id()) {
            list[1] = OptionalProperty(&(*it));
        } else if (it->id == KisLayerPropertiesIcons::alphaLocked.id()) {
            list[2] = OptionalProperty(&(*it));
        } else {
            prependList.prepend(OptionalProperty(&(*it)));
        }
    }

    {
        QMutableListIterator<OptionalProperty> i(prependList);
        i.toBack();
        while (i.hasPrevious()) {
            OptionalProperty val = i.previous();

            int emptyIndex = list.lastIndexOf(0);
            if (emptyIndex < 0) break;

            list[emptyIndex] = val;
            i.remove();
        }
    }

    return prependList + list;
}

int NodeDelegate::Private::numProperties(const QModelIndex &index) const
{
    KisBaseNode::PropertyList props = index.data(KisNodeModel::PropertiesRole).value<KisBaseNode::PropertyList>();
    QList<OptionalProperty> realProps = rightmostProperties(props);
    return realProps.size();
}

OptionalProperty NodeDelegate::Private::findProperty(KisBaseNode::PropertyList &props, const OptionalProperty &refProp) const
{
    KisBaseNode::PropertyList::iterator it = props.begin();
    KisBaseNode::PropertyList::iterator end = props.end();
    for (; it != end; ++it) {
        if (it->id == refProp->id) {
            return &(*it);
        }
    }

    return 0;
}

OptionalProperty NodeDelegate::Private::findVisibilityProperty(KisBaseNode::PropertyList &props) const
{
    KisBaseNode::PropertyList::iterator it = props.begin();
    KisBaseNode::PropertyList::iterator end = props.end();
    for (; it != end; ++it) {
        if (it->id == KisLayerPropertiesIcons::visible.id()) {
            return &(*it);
        }
    }

    return 0;
}

void NodeDelegate::drawIcons(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    KisNodeViewColorScheme scm;
    const QRect rc = iconsRect(option, index);

    QTransform oldTransform = p->transform();
    QPen oldPen = p->pen();
    p->setTransform(QTransform::fromTranslate(rc.x(), rc.y()));
    p->setPen(scm.gridColor(option, d->view));

    int x = 0;
    const int y = (scm.rowHeight() - scm.border() - scm.iconSize()) / 2;
    KisBaseNode::PropertyList props = index.data(KisNodeModel::PropertiesRole).value<KisBaseNode::PropertyList>();
    QList<OptionalProperty> realProps = d->rightmostProperties(props);

    if (option.direction == Qt::RightToLeft) {
        std::reverse(realProps.begin(), realProps.end());
    }

    Q_FOREACH (OptionalProperty prop, realProps) {
        if (option.direction == Qt::LeftToRight)
            p->drawLine(x, 0, x, scm.rowHeight() - scm.border());

        x += scm.iconMargin();
        if (prop) {
            QIcon icon = prop->state.toBool() ? prop->onIcon : prop->offIcon;
            bool fullColor = prop->state.toBool() && option.state & QStyle::State_Enabled;

            const qreal oldOpacity = p->opacity(); // remember previous opacity
            if (fullColor) {
                 p->setOpacity(1.0);
            }
            else {
                p->setOpacity(0.35);
            }

            p->drawPixmap(x, y, icon.pixmap(scm.iconSize(), QIcon::Normal));
            p->setOpacity(oldOpacity); // restore old opacity
        }
        x += scm.iconSize() + scm.iconMargin();

        if (!(option.direction == Qt::LeftToRight))
            p->drawLine(x, 0, x, scm.rowHeight() - scm.border());
        x += scm.border();
    }

    p->setTransform(oldTransform);
    p->setPen(oldPen);
}

QRect NodeDelegate::visibilityClickRect(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    KisNodeViewColorScheme scm;

    QRect rc = scm.relVisibilityRect();
    rc.setHeight(scm.rowHeight());

    // Move to current index
    rc.moveCenter(option.rect.center());
    // Move to correct location.
    if (option.direction == Qt::RightToLeft) {
        // HACK: Without the -5, the right edge is outside the view
        rc.moveRight(d->view->width()-5);
    } else {
        rc.moveLeft(0);
    }

    return rc;
}

QRect NodeDelegate::decorationClickRect(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    KisNodeViewColorScheme scm;

    QRect rc = scm.relDecorationRect();

    // Move to current index
    rc.moveTop(option.rect.topLeft().y());
    rc.setHeight(scm.rowHeight());
    // Move to correct location.
    if (option.direction == Qt::RightToLeft) {
        rc.moveRight(option.rect.topRight().x());
    } else {
        rc.moveLeft(option.rect.topLeft().x());
    }

    return rc;
}

void NodeDelegate::drawVisibilityIconHijack(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    /**
     * Small hack Alert:
     *
     * Here wepaint over the area that sits basically outside our layer's
     * row. Anyway, just update it later...
     */

    KisNodeViewColorScheme scm;

    KisBaseNode::PropertyList props = index.data(KisNodeModel::PropertiesRole).value<KisBaseNode::PropertyList>();
    OptionalProperty prop = d->findVisibilityProperty(props);
    if (!prop) return;

    QRect fitRect = visibilityClickRect(option, index);
    // Shrink to icon rect
    fitRect = kisGrowRect(fitRect, -(scm.visibilityMargin()+scm.border()));

    QIcon icon = prop->state.toBool() ? prop->onIcon : prop->offIcon;

    // if we are not showing the layer, make the icon slightly transparent like other inactive icons
    const qreal oldOpacity = p->opacity();

    if (!prop->state.toBool()) {
        p->setOpacity(0.35);
    }

    p->drawPixmap(fitRect.x(), fitRect.center().y() - scm.visibilitySize() / 2,
                  icon.pixmap(scm.visibilitySize(),  QIcon::Normal));
    p->setOpacity(oldOpacity);

    //// For debugging purposes only
// //     // p->save();
// //     // p->setPen(Qt::blue);
// //     // KritaUtils::renderExactRect(p, visibilityClickRect(option, index));
// //     // p->restore();
}

void NodeDelegate::drawDecoration(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    KisNodeViewColorScheme scm;
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();

    if (!icon.isNull()) {
        QPixmap pixmap = icon.pixmap(scm.decorationSize(),
                                     (option.state & QStyle::State_Enabled) ?
                                     QIcon::Normal : QIcon::Disabled);

        QRect rc = decorationClickRect(option, index);

        // Shrink to icon rect
        rc = kisGrowRect(rc, -(scm.decorationMargin()+scm.border()));

        const qreal oldOpacity = p->opacity(); // remember previous opacity

        if (!(option.state & QStyle::State_Enabled)) {
            p->setOpacity(0.35);
        }

        p->drawPixmap(rc.topLeft()-QPoint(0, 1), pixmap);
        p->setOpacity(oldOpacity); // restore old opacity
    }
}

void NodeDelegate::drawExpandButton(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    KisNodeViewColorScheme scm;

    QRect rc = decorationClickRect(option, index);

    // Move to current index
//     rc.moveTop(option.rect.topLeft().y());
    // Shrink to icon rect
    rc = kisGrowRect(rc, -(scm.decorationMargin()+scm.border()));

    if (!(option.state & QStyle::State_Children)) return;


    QString iconName = option.state & QStyle::State_Open ?
        "arrow-down" : ((option.direction == Qt::RightToLeft) ? "arrow-left" : "arrow-right");
    QIcon icon = KisIconUtils::loadIcon(iconName);
    QPixmap pixmap = icon.pixmap(rc.width(),
                                 (option.state & QStyle::State_Enabled) ?
                                 QIcon::Normal : QIcon::Disabled);
    p->drawPixmap(rc.bottomLeft()-QPoint(0, scm.decorationSize()-1), pixmap);
}

void NodeDelegate::Private::toggleProperty(KisBaseNode::PropertyList &props, OptionalProperty clickedProperty, bool controlPressed, const QModelIndex &index)
{
    QAbstractItemModel *model = view->model();

    // Using Ctrl+click to enter stasis
    if (controlPressed && clickedProperty->canHaveStasis) {
        // STEP 0: Prepare to Enter or Leave control key stasis
        quint16 numberOfLeaves = model->rowCount(index.parent());
        QModelIndex eachItem;
        // STEP 1: Go.
        if (clickedProperty->isInStasis == false) { // Enter
            /* Make every leaf of this node go State = False, saving the old property value to stateInStasis */
            for (quint16 i = 0; i < numberOfLeaves; ++i) { // Foreach leaf in the node (index.parent())
                eachItem = model->index(i, 1, index.parent());
                // The entire property list has to be altered because model->setData cannot set individual properties
                KisBaseNode::PropertyList eachPropertyList = eachItem.data(KisNodeModel::PropertiesRole).value<KisBaseNode::PropertyList>();
                OptionalProperty prop = findProperty(eachPropertyList, clickedProperty);
                if (!prop) continue;
                prop->stateInStasis = prop->state.toBool();
                prop->state = eachItem == index;
                prop->isInStasis = true;
                model->setData(eachItem, QVariant::fromValue(eachPropertyList), KisNodeModel::PropertiesRole);

            }

            for (quint16 i = 0; i < numberOfLeaves; ++i) { // Foreach leaf in the node (index.parent())
                eachItem = model->index(i, 1, index.parent());
                KisBaseNode::PropertyList eachPropertyList = eachItem.data(KisNodeModel::PropertiesRole).value<KisBaseNode::PropertyList>();
                OptionalProperty prop = findProperty(eachPropertyList, clickedProperty);
                if (!prop) continue;
            }
        } else { // Leave
            /* Make every leaf of this node go State = stateInStasis */
            for (quint16 i = 0; i < numberOfLeaves; ++i) {
                eachItem = model->index(i, 1, index.parent());
                // The entire property list has to be altered because model->setData cannot set individual properties
                KisBaseNode::PropertyList eachPropertyList = eachItem.data(KisNodeModel::PropertiesRole).value<KisBaseNode::PropertyList>();
                OptionalProperty prop = findProperty(eachPropertyList, clickedProperty);
                if (!prop) continue;

                prop->state = prop->stateInStasis;
                prop->isInStasis = false;
                model->setData(eachItem, QVariant::fromValue(eachPropertyList), KisNodeModel::PropertiesRole);
            }
        }
    } else {
        clickedProperty->state = !clickedProperty->state.toBool();
        model->setData(index, QVariant::fromValue(props), KisNodeModel::PropertiesRole);
    }
}


bool NodeDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    KisNodeViewColorScheme scm;

    QStyleOptionViewItem newOption = option;
    newOption.rect = d->view->originalVisualRect(index);

    if ((event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)
        && (index.flags() & Qt::ItemIsEnabled))
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        /**
         * Small hack Alert:
         *
         * Here we handle clicking even when it happened outside
         * the rectangle of the current index. The point is, we
         * use some virtual scroling offset to move the tree to the
         * right of the visibility icon. So the icon itself is placed
         * in an empty area that doesn't belong to any index. But we still
         * handle it.
         */

        const QRect visibilityRect = visibilityClickRect(newOption, index);
        const bool visibilityClicked = visibilityRect.isValid() &&
            visibilityRect.contains(mouseEvent->pos());

        const QRect thumbnailRect = thumbnailClickRect(newOption, index);
        const bool thumbnailClicked = thumbnailRect.isValid() &&
            thumbnailRect.contains(mouseEvent->pos());

        const QRect decorationRect = decorationClickRect(newOption, index);
        const bool decorationClicked = decorationRect.isValid() &&
            decorationRect.contains(mouseEvent->pos());

        const QRect iconsRect = this->iconsRect(newOption, index);
        const bool iconsClicked = iconsRect.isValid() &&
            iconsRect.contains(mouseEvent->pos());

        const bool leftButton = mouseEvent->buttons() & Qt::LeftButton;

        if (leftButton && iconsClicked) {
            KisBaseNode::PropertyList props = index.data(KisNodeModel::PropertiesRole).value<KisBaseNode::PropertyList>();
            QList<OptionalProperty> realProps = d->rightmostProperties(props);
            if (newOption.direction == Qt::RightToLeft) {
                std::reverse(realProps.begin(), realProps.end());
            }
            const int numProps = realProps.size();

            const int iconWidth = scm.iconSize() + 2 * scm.iconMargin() + scm.border();
            const int xPos = mouseEvent->pos().x() - iconsRect.left();
            const int clickedIcon = xPos / iconWidth;
            const int distToBorder = qMin(xPos % iconWidth, iconWidth - xPos % iconWidth);

            if (iconsClicked &&
                clickedIcon >= 0 &&
                clickedIcon < numProps &&
                distToBorder > scm.iconMargin()) {

                OptionalProperty clickedProperty = realProps[clickedIcon];
                if (!clickedProperty) return false;
                d->toggleProperty(props, clickedProperty, mouseEvent->modifiers() == Qt::ControlModifier, index);
                return true;
            }
        } else if (leftButton && visibilityClicked) {
            KisBaseNode::PropertyList props = index.data(KisNodeModel::PropertiesRole).value<KisBaseNode::PropertyList>();
            OptionalProperty clickedProperty = d->findVisibilityProperty(props);
            if (!clickedProperty) return false;
            d->toggleProperty(props, clickedProperty, mouseEvent->modifiers() == Qt::ControlModifier, index);
            return true;
        } else if (leftButton && decorationClicked) {
            bool isExpandable = model->hasChildren(index);
            if (isExpandable) {
                bool isExpanded = d->view->isExpanded(index);
                d->view->setExpanded(index, !isExpanded);
            }
            return true;
        } else if (leftButton && thumbnailClicked) {
            bool hasCorrectModifier = false;
            SelectionAction action = SELECTION_REPLACE;

            if (mouseEvent->modifiers() == Qt::ControlModifier) {
                action = SELECTION_REPLACE;
                hasCorrectModifier = true;
            } else if (mouseEvent->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) {
                action = SELECTION_ADD;
                hasCorrectModifier = true;
            } else if (mouseEvent->modifiers() == (Qt::ControlModifier | Qt::AltModifier)) {
                action = SELECTION_SUBTRACT;
                hasCorrectModifier = true;
            } else if (mouseEvent->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier)) {
                action = SELECTION_INTERSECT;
                hasCorrectModifier = true;
            }

            if (hasCorrectModifier) {
                model->setData(index, QVariant(int(action)), KisNodeModel::SelectOpaqueRole);
            }
            return true; //If not here then the item is !expanded when reaching return false;
        }

        if (mouseEvent->button() == Qt::LeftButton &&
            mouseEvent->modifiers() == Qt::AltModifier) {

            d->view->setCurrentIndex(index);
            model->setData(index, true, KisNodeModel::AlternateActiveRole);
            return true;
        }
    }
    else if (event->type() == QEvent::ToolTip) {
        if (!KisConfig(true).hidePopups()) {
            QHelpEvent *helpEvent = static_cast<QHelpEvent*>(event);
            d->tip.showTip(d->view, helpEvent->pos(), newOption, index);
        }
        return true;
    } else if (event->type() == QEvent::Leave) {
        d->tip.hide();
    }

    return false;
}

QWidget *NodeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
    d->edit = new QLineEdit(parent);
    d->edit->setFocusPolicy(Qt::StrongFocus);
    d->edit->installEventFilter(const_cast<NodeDelegate*>(this)); //hack?
    return d->edit;
}

void NodeDelegate::setEditorData(QWidget *widget, const QModelIndex &index) const
{
    QLineEdit *edit = qobject_cast<QLineEdit*>(widget);
    Q_ASSERT(edit);

    edit->setText(index.data(Qt::DisplayRole).toString());
}

void NodeDelegate::setModelData(QWidget *widget, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *edit = qobject_cast<QLineEdit*>(widget);
    Q_ASSERT(edit);

    model->setData(index, edit->text(), Qt::DisplayRole);
}

void NodeDelegate::updateEditorGeometry(QWidget *widget, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    widget->setGeometry(option.rect);
}


// PROTECTED


bool NodeDelegate::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        if (d->edit) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (!QRect(d->edit->mapToGlobal(QPoint()), d->edit->size()).contains(me->globalPos())) {
                emit commitData(d->edit);
                emit closeEditor(d->edit);
            }
        }
    } break;
    case QEvent::KeyPress: {
        QLineEdit *edit = qobject_cast<QLineEdit*>(object);
        if (edit && edit == d->edit) {
            QKeyEvent *ke = static_cast<QKeyEvent*>(event);
            switch (ke->key()) {
            case Qt::Key_Escape:
                emit closeEditor(edit);
                return true;
            case Qt::Key_Tab:
                emit commitData(edit);
                emit closeEditor(edit, EditNextItem);
                return true;
            case Qt::Key_Backtab:
                emit commitData(edit);
                emit closeEditor(edit, EditPreviousItem);
                return true;
            case Qt::Key_Return:
            case Qt::Key_Enter:
                emit commitData(edit);
                emit closeEditor(edit);
                return true;
            default: break;
            }
        }
    } break;
    case QEvent::ShortcutOverride : {
        QLineEdit *edit = qobject_cast<QLineEdit*>(object);
        if (edit && edit == d->edit){
            auto* key = static_cast<QKeyEvent*>(event);
            if (key->modifiers() == Qt::NoModifier){
                switch (key->key()){
                case Qt::Key_Escape:
                case Qt::Key_Tab:
                case Qt::Key_Backtab:
                case Qt::Key_Return:
                case Qt::Key_Enter:
                    event->accept();
                    return true;
                default: break;
                }
            }
        }

    } break;
    case QEvent::FocusOut : {
        QLineEdit *edit = qobject_cast<QLineEdit*>(object);
        if (edit && edit == d->edit) {
            emit commitData(edit);
            emit closeEditor(edit);
        }
    }
    default: break;
    }

    return QAbstractItemDelegate::eventFilter(object, event);
}


// PRIVATE


QStyleOptionViewItem NodeDelegate::getOptions(const QStyleOptionViewItem &o, const QModelIndex &index)
{
    QStyleOptionViewItem option = o;
    QVariant v = index.data(Qt::FontRole);
    if (v.isValid()) {
        option.font = v.value<QFont>();
        option.fontMetrics = QFontMetrics(option.font);
    }
    v = index.data(Qt::TextAlignmentRole);
    if (v.isValid())
        option.displayAlignment = QFlag(v.toInt());
    v = index.data(Qt::TextColorRole);
    if (v.isValid())
        option.palette.setColor(QPalette::Text, v.value<QColor>());
    v = index.data(Qt::BackgroundColorRole);
    if (v.isValid())
        option.palette.setColor(QPalette::Window, v.value<QColor>());

   return option;
}

void NodeDelegate::drawProgressBar(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant value = index.data(KisNodeModel::ProgressRole);
    if (!value.isNull() && (value.toInt() >= 0 && value.toInt() <= 100)) {

        /// The progress bar will display under the layer name area. The bars have accurate data, so we
        /// probably don't need to also show the actual number for % complete

        KisNodeViewColorScheme scm;

        const QRect thumbnailRect = thumbnailClickRect(option, index);
        const QRect iconsRectR    = iconsRect(option, index);
        const int height = 5;
        const QRect rc = QRect(
            ((option.direction == Qt::RightToLeft) ? iconsRectR.bottomRight()
                                                   : thumbnailRect.bottomRight()) - QPoint(0, height),
            ((option.direction == Qt::RightToLeft) ? thumbnailRect.bottomLeft()
                                                   : iconsRectR.bottomLeft()));

        p->save();
        {
            p->setClipRect(rc);
            QStyle* style = QApplication::style();
            QStyleOptionProgressBar opt;

            opt.minimum = 0;
            opt.maximum = 100;
            opt.progress = value.toInt();
            opt.textVisible = false;
            opt.textAlignment = Qt::AlignHCenter;
            opt.text = i18n("%1 %", opt.progress);
            opt.orientation = Qt::Horizontal;
            opt.state = option.state;
            style->drawControl(QStyle::CE_ProgressBar, &opt, p, 0);
        }
        p->restore();
    }
}

void NodeDelegate::slotConfigChanged()
{
    KisConfig cfg(true);

    d->checkersColor1 = cfg.checkersColor1();
    d->checkersColor2 = cfg.checkersColor2();
}

void NodeDelegate::slotUpdateIcon()
{
   KisLayerPropertiesIcons::instance()->updateIcons();
}
