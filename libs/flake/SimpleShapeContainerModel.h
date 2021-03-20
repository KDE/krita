/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2011 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef SIMPLESHAPECONTAINERMODEL_H
#define SIMPLESHAPECONTAINERMODEL_H

#include "KoShapeContainerModel.h"
#include <kis_debug.h>
#include <KoShapeManager.h>

/// \internal
class SimpleShapeContainerModel: public KoShapeContainerModel
{
public:
    SimpleShapeContainerModel() {}
    ~SimpleShapeContainerModel() override {}

    SimpleShapeContainerModel(const SimpleShapeContainerModel &rhs)
        : KoShapeContainerModel(rhs),
          m_inheritsTransform(rhs.m_inheritsTransform),
          m_clipped(rhs.m_clipped)
    {
        Q_FOREACH (KoShape *shape, rhs.m_members) {
            KoShape *clone = shape->cloneShape();
            KIS_SAFE_ASSERT_RECOVER_NOOP(clone && "Copying this shape is not implemented!");
            if (clone) {
                m_members << clone;
            }
        }

        KIS_ASSERT_RECOVER(m_members.size() == m_inheritsTransform.size() &&
                           m_members.size() == m_clipped.size())
        {
            qDeleteAll(m_members);
            m_members.clear();
            m_inheritsTransform.clear();
            m_clipped.clear();
        }
    }

    void add(KoShape *child) override {
        if (m_members.contains(child))
            return;
        m_members.append(child);
        m_clipped.append(false);
        m_inheritsTransform.append(true);
    }
    void setClipped(const KoShape *shape, bool value) override {
        const int index = indexOf(shape);
        KIS_SAFE_ASSERT_RECOVER_RETURN(index >= 0);
        m_clipped[index] = value;
    }
    bool isClipped(const KoShape *shape) const override {
        const int index = indexOf(shape);
        KIS_SAFE_ASSERT_RECOVER(index >= 0) { return false;}
        return m_clipped[index];
    }
    void remove(KoShape *shape) override {
        const int index = indexOf(shape);
        KIS_SAFE_ASSERT_RECOVER_RETURN(index >= 0);

        m_members.removeAt(index);
        m_clipped.removeAt(index);
        m_inheritsTransform.removeAt(index);
    }
    int count() const override {
        return m_members.count();
    }
    QList<KoShape*> shapes() const override {
        return QList<KoShape*>(m_members);
    }
    void containerChanged(KoShapeContainer *, KoShape::ChangeType) override { }

    void setInheritsTransform(const KoShape *shape, bool value) override {
        const int index = indexOf(shape);
        KIS_SAFE_ASSERT_RECOVER_RETURN(index >= 0);
        m_inheritsTransform[index] = value;
    }
    bool inheritsTransform(const KoShape *shape) const override {
        const int index = indexOf(shape);
        KIS_SAFE_ASSERT_RECOVER(index >= 0)  { return true;}
        return m_inheritsTransform[index];
    }

    void proposeMove(KoShape *shape, QPointF &move) override
    {
        KoShapeContainer *parent = shape->parent();
        bool allowedToMove = true;
        while (allowedToMove && parent) {
            allowedToMove = parent->isShapeEditable();
            parent = parent->parent();
        }
        if (! allowedToMove) {
            move.setX(0);
            move.setY(0);
        }
    }

    void shapeHasBeenAddedToHierarchy(KoShape *shape, KoShapeContainer *addedToSubtree) override {
        if (m_associatedRootShapeManager) {
            m_associatedRootShapeManager->addShape(shape);
        }
        KoShapeContainerModel::shapeHasBeenAddedToHierarchy(shape, addedToSubtree);
    }

    void shapeToBeRemovedFromHierarchy(KoShape *shape, KoShapeContainer *removedFromSubtree) override {
        if (m_associatedRootShapeManager) {
            m_associatedRootShapeManager->remove(shape);
        }
        KoShapeContainerModel::shapeToBeRemovedFromHierarchy(shape, removedFromSubtree);
    }

    KoShapeManager *associatedRootShapeManager() const {
        return m_associatedRootShapeManager;
    }

    /**
     * If the container is the root of shapes hierarchy, it should also track the content
     * of the shape manager. Add all added/removed shapes should be also
     * added to \p shapeManager.
     */
    void setAssociatedRootShapeManager(KoShapeManager *manager) {
        if (m_associatedRootShapeManager) {
            Q_FOREACH(KoShape *shape, this->shapes()) {
                m_associatedRootShapeManager->remove(shape);
            }
        }

        m_associatedRootShapeManager = manager;

        if (m_associatedRootShapeManager) {
            Q_FOREACH(KoShape *shape, this->shapes()) {
                m_associatedRootShapeManager->addShape(shape);
            }
        }
    }

private:
    int indexOf(const KoShape *shape) const {
        // workaround indexOf constness!
        return m_members.indexOf(const_cast<KoShape*>(shape));
    }

private: // members
    QList <KoShape *> m_members;
    QList <bool> m_inheritsTransform;
    QList <bool> m_clipped;
    KoShapeManager *m_associatedRootShapeManager = 0;
};

#endif
