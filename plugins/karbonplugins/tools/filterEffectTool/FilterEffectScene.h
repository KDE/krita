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

#ifndef FILTEREFFECTSCENE_H
#define FILTEREFFECTSCENE_H

#include <QGraphicsScene>
#include <QString>
#include <QMap>

class KoFilterEffect;
class KoFilterEffectStack;
class QGraphicsItem;
class EffectItemBase;
class EffectItem;
class ConnectionItem;

class ConnectionSource
{
public:
    enum SourceType {
        Effect,          ///< a complete effect item
        SourceGraphic,   ///< SourceGraphic predefined input image
        SourceAlpha,     ///< SourceAlpha predefined input image
        BackgroundImage, ///< BackgroundImage predefined input image
        BackgroundAlpha, ///< BackgroundAlpha predefined input image
        FillPaint,       ///< FillPaint predefined input image
        StrokePaint      ///< StrokePaint predefined input image
    };
    ConnectionSource();
    ConnectionSource(KoFilterEffect *effect, SourceType type);
    /// Returns the source type
    SourceType type() const;
    /// Returns the corresponding filter effect, or 0 if type == Effect
    KoFilterEffect *effect() const;

    static SourceType typeFromString(const QString &str);
    static QString typeToString(SourceType type);

private:
    SourceType m_type;         ///< the source type
    KoFilterEffect *m_effect;  ///< the corresponding effect if type == Effect, 0 otherwise
};

class ConnectionTarget
{
public:
    ConnectionTarget();
    ConnectionTarget(KoFilterEffect *effect, int inputIndex);

    /// Returns the target input index
    int inputIndex() const;
    /// Returns the corresponding filter effect
    KoFilterEffect *effect() const;

private:
    int m_inputIndex;          ///< the index of the input of the target effect
    KoFilterEffect *m_effect;  ///< the target effect
};

class FilterEffectScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit FilterEffectScene(QObject *parent = 0);
    virtual ~FilterEffectScene();

    /// initializes the scene from the filter effect stack
    void initialize(KoFilterEffectStack *effectStack);

    /// Returns list of selected effect items
    QList<ConnectionSource> selectedEffectItems() const;

Q_SIGNALS:
    void connectionCreated(ConnectionSource source, ConnectionTarget target);

protected:
    /// reimplemented from QGraphicsScene
    virtual void dropEvent(QGraphicsSceneDragDropEvent *event);

private Q_SLOTS:
    void selectionChanged();
private:
    void createEffectItems(KoFilterEffect *effect);
    void addSceneItem(QGraphicsItem *item);
    void layoutConnections();
    void layoutEffects();

    QList<QString> m_defaultInputs;
    KoFilterEffectStack *m_effectStack;
    QList<EffectItemBase *> m_items;
    QList<ConnectionItem *> m_connectionItems;
    QMap<QString, EffectItemBase *> m_outputs;
    QGraphicsProxyWidget *m_defaultInputProxy;
};

#endif // FILTEREFFECTSCENE_H
