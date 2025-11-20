/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2010 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KOSHAPEPRIVATE_H
#define KOSHAPEPRIVATE_H

#include "KoShape.h"

#include <QPoint>
#include <QPaintDevice>
#include <QTransform>
#include <QScopedPointer>
#include <QSharedData>

#include <KoClipMask.h>

class KoShapeManager;


class KoShape::SharedData : public QSharedData
{
public:
    explicit SharedData();
    virtual ~SharedData();

    explicit SharedData(const SharedData &rhs);


public:
    // Members

    mutable QSizeF size; // size in pt
    QString shapeId;
    QString name; ///< the shapes names

    QTransform localMatrix; ///< the shapes local transformation matrix

    QScopedPointer<KoShapeUserData> userData;
    QSharedPointer<KoShapeStrokeModel> stroke; ///< points to a stroke, or 0 if there is no stroke
    QSharedPointer<KoShapeBackground> fill; ///< Stands for the background color / fill etc.
    bool inheritBackground = false;
    bool inheritStroke = false;
    // XXX: change this to instance instead of pointer
    QScopedPointer<KoClipPath> clipPath; ///< the current clip path
    QScopedPointer<KoClipMask> clipMask; ///< the current clip mask
    QMap<QString, QString> additionalAttributes;
    QMap<QByteArray, QString> additionalStyleAttributes;
    qreal transparency; ///< the shapes transparency
    QString hyperLink; //hyperlink for this shape

    int zIndex : 16; // keep maxZIndex in sync!
    int runThrough : 16;
    int visible : 1;
    int printable : 1;
    int geometryProtected : 1;
    int keepAspect : 1;
    int selectable : 1;
    int protectContent : 1;

    KoShape::TextRunAroundSide textRunAroundSide;
    qreal textRunAroundDistanceLeft;
    qreal textRunAroundDistanceTop;
    qreal textRunAroundDistanceRight;
    qreal textRunAroundDistanceBottom;
    qreal textRunAroundThreshold;
    KoShape::TextRunAroundContour textRunAroundContour;

    QVector<PaintOrder> paintOrder {Fill, Stroke, Markers};
    bool inheritPaintOrder = true;
};

class KoShape::Private
{
private:
    /**
     * A special listener class to track the lifetime of the dependees
     * and remove them from the notification list when they are deleted
     */
    struct DependeesLifetimeListener : KoShape::ShapeChangeListener
    {
        DependeesLifetimeListener(QList<KoShape*> &dependees)
            : m_dependees(dependees)
        {
        }

        void notifyShapeChanged(KoShape::ChangeType type, KoShape *shape) override {
            if (type == KoShape::Deleted && m_dependees.contains(shape)) {
                m_dependees.removeOne(shape);
                // the listener will be unlinked on destruction automatically
            }
        }

    private:
        QList<KoShape*> &m_dependees;
    };

public:
    Private() : dependeesLifetimeListener(dependees) {}
    Private(const Private &rhs) = delete;

    KoShapeContainer *parent = nullptr;
    QSet<KoShapeManager *> shapeManagers;
    QSet<KoShape *> toolDelegates;
    QList<KoShape*> dependees; ///< list of shape dependent on this shape
    QList<KoShape::ShapeChangeListener*> listeners;
    DependeesLifetimeListener dependeesLifetimeListener;
};

#endif
