/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "FilterEffectSceneItems.h"
#include "KoFilterEffect.h"

#include <QPen>
#include <QBrush>
#include <QFont>
#include <QDrag>
#include <QWidget>

const QSizeF ConnectorSize = QSize(20, 20);
const qreal ItemWidth = 15 * ConnectorSize.height();
const qreal FontSize = 0.8 * ConnectorSize.height();

ConnectorItem::ConnectorItem(ConnectorType type, int index, QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent)
    , m_type(type)
    , m_index(index)
{
    if (m_type == Output) {
        setBrush(QBrush(Qt::red));
    } else if (m_type == Input) {
        setBrush(QBrush(Qt::green));
    }
    setAcceptDrops(true);
    setRect(QRectF(QPointF(), ConnectorSize));
}

void ConnectorItem::setCenter(const QPointF &position)
{
    QRectF r = rect();
    r.moveCenter(position);
    setRect(r);
}

ConnectorItem::ConnectorType ConnectorItem::connectorType()
{
    return m_type;
}

int ConnectorItem::connectorIndex() const
{
    return m_index;
}

KoFilterEffect *ConnectorItem::effect() const
{
    if (!parentItem()) {
        return 0;
    }
    EffectItemBase *effectItem = dynamic_cast<EffectItemBase *>(parentItem());
    if (!effectItem) {
        return 0;
    }

    return effectItem->effect();
}

ConnectorMimeData::ConnectorMimeData(ConnectorItem *connector)
    : m_connector(connector)
{
}

ConnectorItem *ConnectorMimeData::connector() const
{
    return m_connector;
}

EffectItemBase::EffectItemBase(KoFilterEffect *effect)
    : QGraphicsRectItem(0), m_effect(effect)
{
    setZValue(1);
    setFlags(QGraphicsItem::ItemIsSelectable);
    setAcceptDrops(true);
    setHandlesChildEvents(true);
}

void EffectItemBase::createText(const QString &text)
{
    QGraphicsSimpleTextItem *textItem = new QGraphicsSimpleTextItem(text, this);
    QFont font = textItem->font();
    font.setPointSize(FontSize);
    textItem->setFont(font);
    QRectF textBox = textItem->boundingRect();
    QPointF offset = rect().center() - textBox.center();
    setTransform(QTransform::fromTranslate(offset.x(), offset.y()), true);
}

void EffectItemBase::createOutput(const QPointF &position, const QString &name)
{
    ConnectorItem *connector = new ConnectorItem(ConnectorItem::Output, 0, this);
    connector->setCenter(position);

    m_outputPosition = position;
    m_outputName = name;
}

void EffectItemBase::createInput(const QPointF &position)
{
    int inputCount = m_inputPositions.count();
    ConnectorItem *connector = new ConnectorItem(ConnectorItem::Input, inputCount, this);
    connector->setCenter(position);

    m_inputPositions.append(position);
}

QPointF EffectItemBase::outputPosition() const
{
    return m_outputPosition;
}

QPointF EffectItemBase::inputPosition(int index) const
{
    if (index < 0 || index >= m_inputPositions.count()) {
        return QPointF();
    }
    return m_inputPositions[index];
}

QString EffectItemBase::outputName() const
{
    return m_outputName;
}

QSizeF EffectItemBase::connectorSize() const
{
    return ConnectorSize;
}

KoFilterEffect *EffectItemBase::effect() const
{
    return m_effect;
}

void EffectItemBase::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    ConnectorItem *connector = connectorAtPosition(event->scenePos());
    if (!connector) {
        return;
    }

    ConnectorMimeData *data = new ConnectorMimeData(connector);

    QDrag *drag = new QDrag(event->widget());
    drag->setMimeData(data);
    drag->exec();
}

void EffectItemBase::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    event->ignore();
    ConnectorItem *targetItem = connectorAtPosition(event->scenePos());
    if (!targetItem) {
        return;
    }

    const ConnectorMimeData *data = dynamic_cast<const ConnectorMimeData *>(event->mimeData());
    if (!data) {
        return;
    }

    ConnectorItem *sourceItem = data->connector();
    int sourceItemType = sourceItem->connectorType();
    int targetItemType = targetItem->connectorType();

    if (sourceItemType == targetItemType) {
        return;
    }

    // do not accept connection within single effect item
    if (sourceItem->parentItem() == targetItem->parentItem()) {
        return;
    }

    if (sourceItemType == ConnectorItem::Input) {
        // we can only connect input with output above
        if (sourceItem->scenePos().y() < targetItem->scenePos().y()) {
            return;
        }
    }
    if (sourceItemType == ConnectorItem::Output) {
        // we can only connect output with input below
        if (sourceItem->scenePos().y() > targetItem->scenePos().y()) {
            return;
        }
    }

    event->accept();
}

void EffectItemBase::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    ConnectorItem *connector = connectorAtPosition(event->scenePos());
    if (!connector) {
        return;
    }

    const ConnectorMimeData *data = dynamic_cast<const ConnectorMimeData *>(event->mimeData());
    if (!data) {
        return;
    }
}

ConnectorItem *EffectItemBase::connectorAtPosition(const QPointF &scenePosition)
{
    Q_FOREACH (QGraphicsItem *childItem, childItems()) {
        ConnectorItem *connector = dynamic_cast<ConnectorItem *>(childItem);
        if (!connector) {
            continue;
        }
        if (connector->contains(connector->mapFromScene(scenePosition))) {
            return connector;
        }
    }

    return 0;
}

DefaultInputItem::DefaultInputItem(const QString &name, KoFilterEffect *effect)
    : EffectItemBase(effect), m_name(name)
{
    setRect(0, 0, ItemWidth, 2 * ConnectorSize.height());

    createOutput(QPointF(ItemWidth, 0.5 * rect().height()), name);
    createText(name);

    QLinearGradient g(QPointF(0, 0), QPointF(1, 1));
    g.setCoordinateMode(QGradient::ObjectBoundingMode);
    g.setColorAt(0, Qt::white);
    g.setColorAt(1, QColor(255, 168, 88));
    setBrush(QBrush(g));
}

EffectItem::EffectItem(KoFilterEffect *effect)
    : EffectItemBase(effect)
{
    Q_ASSERT(effect);
    QRectF circle(QPointF(), ConnectorSize);

    QPointF position(ItemWidth, ConnectorSize.height());

    // create input connectors
    int requiredInputCount = effect->requiredInputCount();
    int usedInputCount = qMax(requiredInputCount, effect->inputs().count());
    for (int i = 0; i < usedInputCount; ++i) {
        createInput(position);
        position.ry() += 1.5 * ConnectorSize.height();
    }

    // create a new input connector when maximal input count in not reached yet
    if (usedInputCount < effect->maximalInputCount()) {
        createInput(position);
        position.ry() += 1.5 * ConnectorSize.height();
    }
    // create output connector
    position.ry() += 0.5 * ConnectorSize.height();
    createOutput(position, effect->output());

    setRect(0, 0, ItemWidth, position.y() + ConnectorSize.height());

    createText(effect->id());

    QLinearGradient g(QPointF(0, 0), QPointF(1, 1));
    g.setCoordinateMode(QGradient::ObjectBoundingMode);
    g.setColorAt(0, Qt::white);
    g.setColorAt(1, QColor(0, 192, 192));
    setBrush(QBrush(g));
}

ConnectionItem::ConnectionItem(EffectItemBase *source, EffectItemBase *target, int targetInput)
    : QGraphicsPathItem(0)
    , m_source(source)
    , m_target(target)
    , m_targetInput(targetInput)
{
    setPen(QPen(Qt::black));
}

EffectItemBase *ConnectionItem::sourceItem() const
{
    return m_source;
}

EffectItemBase *ConnectionItem::targetItem() const
{
    return m_target;
}

int ConnectionItem::targetInput() const
{
    return m_targetInput;
}

void ConnectionItem::setSourceItem(EffectItemBase *source)
{
    m_source = source;
}

void ConnectionItem::setTargetItem(EffectItemBase *target, int targetInput)
{
    m_target = target;
    m_targetInput = targetInput;
}
