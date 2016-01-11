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

#include "FilterEffectScene.h"
#include "FilterEffectSceneItems.h"
#include "KoShape.h"
#include "KoFilterEffect.h"
#include "KoFilterEffectStack.h"

#include <QDebug>
#include <kcombobox.h>

#include <QGraphicsProxyWidget>
#include <QPushButton>

const qreal ItemSpacing = 10.0;
const qreal ConnectionDistance = 10.0;

ConnectionSource::ConnectionSource()
    : m_type(Effect)
    , m_effect(0)
{
}

ConnectionSource::ConnectionSource(KoFilterEffect *effect, SourceType type)
    : m_type(type)
    , m_effect(effect)
{
}

ConnectionSource::SourceType ConnectionSource::type() const
{
    return m_type;
}

KoFilterEffect *ConnectionSource::effect() const
{
    return m_effect;
}

ConnectionSource::SourceType ConnectionSource::typeFromString(const QString &str)
{
    if (str == "SourceGraphic") {
        return SourceGraphic;
    } else if (str == "SourceAlpha") {
        return SourceAlpha;
    } else if (str == "BackgroundImage") {
        return BackgroundImage;
    } else if (str == "BackgroundAlpha") {
        return BackgroundAlpha;
    } else if (str == "FillPaint") {
        return FillPaint;
    } else if (str == "StrokePaint") {
        return StrokePaint;
    } else {
        return Effect;
    }
}

QString ConnectionSource::typeToString(SourceType type)
{
    if (type == SourceGraphic) {
        return "SourceGraphic";
    } else if (type == SourceAlpha) {
        return "SourceAlpha";
    } else if (type == BackgroundImage) {
        return "BackgroundImage";
    } else if (type == BackgroundAlpha) {
        return "BackgroundAlpha";
    } else if (type == FillPaint) {
        return "FillPaint";
    } else if (type == StrokePaint) {
        return "StrokePaint";
    } else {
        return "";
    }
}

ConnectionTarget::ConnectionTarget()
    : m_inputIndex(0)
    , m_effect(0)
{
}

ConnectionTarget::ConnectionTarget(KoFilterEffect *effect, int inputIndex)
    : m_inputIndex(inputIndex)
    , m_effect(effect)
{
}

int ConnectionTarget::inputIndex() const
{
    return m_inputIndex;
}

KoFilterEffect *ConnectionTarget::effect() const
{
    return m_effect;
}

FilterEffectScene::FilterEffectScene(QObject *parent)
    : QGraphicsScene(parent)
    , m_effectStack(0)
{
    m_defaultInputs << "SourceGraphic" << "SourceAlpha";
    m_defaultInputs << "FillPaint" << "StrokePaint";
    m_defaultInputs << "BackgroundImage" << "BackgroundAlpha";

    connect(this, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
}

FilterEffectScene::~FilterEffectScene()
{
}

void FilterEffectScene::initialize(KoFilterEffectStack *effectStack)
{
    m_items.clear();
    m_connectionItems.clear();
    m_outputs.clear();
    clear();

    m_effectStack = effectStack;

    if (!m_effectStack) {
        return;
    }

    QList<KoFilterEffect *> filterEffects = m_effectStack->filterEffects();
    if (!filterEffects.count()) {
        return;
    }

    Q_FOREACH (KoFilterEffect *effect, filterEffects) {
        createEffectItems(effect);
    }

    layoutEffects();
    layoutConnections();
}

void FilterEffectScene::createEffectItems(KoFilterEffect *effect)
{
    const bool isFirstItem = m_items.count() == 0;
    const QString defaultInput = isFirstItem ? "SourceGraphic" : m_items.last()->outputName();

    QList<QString> inputs = effect->inputs();
    for (int i = inputs.count(); i < effect->requiredInputCount(); ++i) {
        inputs.append(defaultInput);
    }

    QSet<QString> defaultItems;
    Q_FOREACH (const QString &currentInput, inputs) {
        const QString &input = currentInput.isEmpty() ? defaultInput : currentInput;
        if (m_defaultInputs.contains(input) && ! defaultItems.contains(input)) {
            DefaultInputItem *item = new DefaultInputItem(input, effect);
            addSceneItem(item);
            m_outputs.insert(item->outputName(), item);
            defaultItems.insert(input);
        }
    }

    EffectItem *effectItem = new EffectItem(effect);

    // create connections
    int index = 0;
    Q_FOREACH (const QString &currentInput, inputs) {
        const QString &input = currentInput.isEmpty() ? defaultInput : currentInput;
        EffectItemBase *outputItem = m_outputs.value(input, 0);
        if (outputItem) {
            ConnectionItem *connectionItem = new ConnectionItem(outputItem, effectItem, index);
            addSceneItem(connectionItem);
        }
        index++;
    }

    addSceneItem(effectItem);

    m_outputs.insert(effectItem->outputName(), effectItem);
}

void FilterEffectScene::addSceneItem(QGraphicsItem *item)
{
    addItem(item);
    EffectItemBase *effectItem = dynamic_cast<EffectItemBase *>(item);
    if (effectItem) {
        m_items.append(effectItem);
    } else {
        ConnectionItem *connectionItem = dynamic_cast<ConnectionItem *>(item);
        if (connectionItem) {
            m_connectionItems.append(connectionItem);
        }
    }
}

void FilterEffectScene::layoutEffects()
{
    QPointF position(25, 25);
    Q_FOREACH (EffectItemBase *item, m_items) {
        item->setPos(position);
        position.ry() += item->rect().height() + ItemSpacing;
    }
}

void FilterEffectScene::layoutConnections()
{
    QList<QPair<int, int> > sortedConnections;

    // calculate connection sizes from item distances
    int connectionIndex = 0;
    Q_FOREACH (ConnectionItem *item, m_connectionItems) {
        int sourceIndex = m_items.indexOf(item->sourceItem());
        int targetIndex = m_items.indexOf(item->targetItem());
        sortedConnections.append(QPair<int, int>(targetIndex - sourceIndex, connectionIndex));
        connectionIndex++;
    }

    qSort(sortedConnections);
    qreal distance = ConnectionDistance;
    int lastSize = -1;
    int connectionCount = sortedConnections.count();
    for (int i = 0; i < connectionCount; ++i) {
        const QPair<int, int> &connection = sortedConnections[i];

        int size = connection.first;
        if (size > lastSize) {
            lastSize = size;
            distance += ConnectionDistance;
        }

        ConnectionItem *connectionItem = m_connectionItems[connection.second];
        if (!connectionItem) {
            continue;
        }
        EffectItemBase *sourceItem = connectionItem->sourceItem();
        EffectItemBase *targetItem = connectionItem->targetItem();
        if (!sourceItem || ! targetItem) {
            continue;
        }

        int targetInput = connectionItem->targetInput();
        QPointF sourcePos = sourceItem->mapToScene(sourceItem->outputPosition());
        QPointF targetPos = targetItem->mapToScene(targetItem->inputPosition(targetInput));
        QPainterPath path;
        path.moveTo(sourcePos + QPointF(0.5 * sourceItem->connectorSize().width(), 0));
        path.lineTo(sourcePos + QPointF(distance, 0));
        path.lineTo(targetPos + QPointF(distance, 0));
        path.lineTo(targetPos + QPointF(0.5 * targetItem->connectorSize().width(), 0));
        connectionItem->setPath(path);
    }
}

void FilterEffectScene::selectionChanged()
{
    if (selectedItems().count()) {
        Q_FOREACH (EffectItemBase *item, m_items) {
            if (item->isSelected()) {
                item->setOpacity(1.0);
            } else {
                item->setOpacity(0.25);
            }
        }
    } else {
        Q_FOREACH (EffectItemBase *item, m_items) {
            item->setOpacity(1);
        }
    }
}

QList<ConnectionSource> FilterEffectScene::selectedEffectItems() const
{
    QList<ConnectionSource> effectItems;

    QList<QGraphicsItem *> selectedGraphicsItems = selectedItems();
    if (!selectedGraphicsItems.count()) {
        return effectItems;
    }
    if (!m_items.count()) {
        return effectItems;
    }

    Q_FOREACH (QGraphicsItem *item, selectedGraphicsItems) {
        EffectItemBase *effectItem = dynamic_cast<EffectItemBase *>(item);
        if (!item) {
            continue;
        }

        ConnectionSource::SourceType type = ConnectionSource::Effect;

        KoFilterEffect *effect = effectItem->effect();
        if (dynamic_cast<DefaultInputItem *>(item)) {
            type = ConnectionSource::typeFromString(effectItem->outputName());
        }

        effectItems.append(ConnectionSource(effect, type));
    }

    return effectItems;
}

void FilterEffectScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    ConnectorItem *dropTargetItem = 0;
    QList<QGraphicsItem *> itemsAtPositon = items(event->scenePos());
    Q_FOREACH (QGraphicsItem *item, itemsAtPositon) {
        dropTargetItem = dynamic_cast<ConnectorItem *>(item);
        if (dropTargetItem) {
            break;
        }
    }
    if (!dropTargetItem) {
        return;
    }

    const ConnectorMimeData *data = dynamic_cast<const ConnectorMimeData *>(event->mimeData());
    if (!data) {
        return;
    }

    ConnectorItem *dropSourceItem = data->connector();
    if (!dropSourceItem) {
        return;
    }

    EffectItemBase *outputParentItem = 0;
    KoFilterEffect *inputEffect = 0;
    KoFilterEffect *outputEffect = 0;
    int inputIndex = 0;

    if (dropTargetItem->connectorType() == ConnectorItem::Input) {
        // dropped output onto an input
        outputParentItem = dynamic_cast<EffectItemBase *>(dropSourceItem->parentItem());
        outputEffect = dropSourceItem->effect();
        inputEffect = dropTargetItem->effect();
        inputIndex = dropTargetItem->connectorIndex();
    } else {
        // dropped input onto an output
        outputParentItem = dynamic_cast<EffectItemBase *>(dropTargetItem->parentItem());
        outputEffect = dropTargetItem->effect();
        inputEffect = dropSourceItem->effect();
        inputIndex = dropSourceItem->connectorIndex();
    }

    ConnectionSource::SourceType outputType = ConnectionSource::Effect;
    // check if item with the output is a predefined one
    if (m_defaultInputs.contains(outputParentItem->outputName())) {
        outputType = ConnectionSource::typeFromString(outputParentItem->outputName());
        outputEffect = 0;
    }
    ConnectionSource source(outputEffect, outputType);
    ConnectionTarget target(inputEffect, inputIndex);
    emit connectionCreated(source, target);
}
