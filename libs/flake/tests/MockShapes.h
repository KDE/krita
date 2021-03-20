/*
 *  This file is part of Calligra tests
 *
 *  SPDX-FileCopyrightText: 2006-2010 Thomas Zander <zander@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef MOCKSHAPES_H
#define MOCKSHAPES_H

#include <KoSelectedShapesProxySimple.h>
#include <KoShapeGroup.h>
#include <KoCanvasBase.h>
#include <KoShapeControllerBase.h>
#include <KoShapeContainerModel.h>
#include <QPainter>
#include "KoShapeManager.h"
#include "FlakeDebug.h"
#include "KoSnapData.h"
#include "KoUnit.h"

#include "kritaflake_export.h"
#include "kis_assert.h"


class KRITAFLAKE_EXPORT MockShape : public KoShape
{
public:
    MockShape() : paintedCount(0) {}
    void paint(QPainter &painter, KoShapePaintingContext &) const override {
        Q_UNUSED(painter);
        //qDebug() << "Shape" << kBacktrace( 10 );
        paintedCount++;
    }
    mutable int paintedCount;
};

#include <SimpleShapeContainerModel.h>

class KRITAFLAKE_EXPORT MockContainer : public KoShapeContainer
{
public:
    MockContainer(KoShapeContainerModel *model = new SimpleShapeContainerModel()) : KoShapeContainer(model), paintedCount(0) {}
    void paintComponent(QPainter &painter, KoShapePaintingContext &) const override {
        Q_UNUSED(painter);
        //qDebug() << "Container:" << kBacktrace( 10 );
        paintedCount++;
    }

    mutable int paintedCount;

    bool contains(KoShape *shape) const {
        return shapes().contains(shape);
    }

    void setAssociatedRootShapeManager(KoShapeManager *shapeManager)
    {
        SimpleShapeContainerModel *model = dynamic_cast<SimpleShapeContainerModel*>(this->model());
        KIS_SAFE_ASSERT_RECOVER_RETURN(model);
        model->setAssociatedRootShapeManager(shapeManager);
    }

    KoShapeManager* associatedRootShapeManager() const
    {
        SimpleShapeContainerModel *model = dynamic_cast<SimpleShapeContainerModel*>(this->model());
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(model, 0);
        return model->associatedRootShapeManager();
    }

};

class KRITAFLAKE_EXPORT MockGroup : public KoShapeGroup
{
    void paintComponent(QPainter &painter, KoShapePaintingContext &) const override {
        Q_UNUSED(painter);
        paintedCount++;
    }
public:
    bool contains(KoShape *shape) const {
        return shapes().contains(shape);
    }

    mutable int paintedCount;
};

class KoToolProxy;

class KRITAFLAKE_EXPORT MockShapeController : public KoShapeControllerBase
{
public:
    QRectF documentRectInPixels() const override {
        return QRectF(0,0,100,100);
    }

    qreal pixelsPerInch() const override {
        return 72.0;
    }
};

class KRITAFLAKE_EXPORT MockCanvas : public KoCanvasBase
{
    Q_OBJECT
public:
    MockCanvas(KoShapeControllerBase *aKoShapeControllerBase =0)//made for TestSnapStrategy.cpp
            : KoCanvasBase(aKoShapeControllerBase),
              m_shapeManager(new KoShapeManager(this)),
              m_selectedShapesProxy(new KoSelectedShapesProxySimple(m_shapeManager.data()))
    {
    }

    ~MockCanvas() override {}
    void setHorz(qreal pHorz){
        m_horz = pHorz;
    }
    void setVert(qreal pVert){
        m_vert = pVert;
    }
    void gridSize(QPointF *offset, QSizeF *spacing) const override {
        Q_UNUSED(offset);

        spacing->setWidth(m_horz);
        spacing->setHeight(m_vert);
    }
    bool snapToGrid() const override  {
        return true;
    }
    void addCommand(KUndo2Command*) override { }
    KoShapeManager *shapeManager() const override  {
        return m_shapeManager.data();
    }
    KoSelectedShapesProxy *selectedShapesProxy() const override {
        return m_selectedShapesProxy.data();
    }
    void updateCanvas(const QRectF&) override  {}
    KoToolProxy * toolProxy() const override {
        return 0;
    }
    KoViewConverter *viewConverter() const override {
        return 0;
    }
    QWidget* canvasWidget() override {
        return 0;
    }
    const QWidget* canvasWidget() const override {
        return 0;
    }
    KoUnit unit() const override {
        return KoUnit(KoUnit::Millimeter);
    }
    void updateInputMethodInfo() override {}
    void setCursor(const QCursor &) override {}
    private:
        QScopedPointer<KoShapeManager> m_shapeManager;
        QScopedPointer<KoSelectedShapesProxy> m_selectedShapesProxy;
        qreal m_horz;
        qreal m_vert;
};

class KRITAFLAKE_EXPORT MockContainerModel : public KoShapeContainerModel
{
public:
    MockContainerModel() {
        resetCounts();
    }

    /// reimplemented
    void add(KoShape *child) override {
        m_children.append(child); // note that we explicitly do not check for duplicates here!
    }
    /// reimplemented
    void remove(KoShape *child) override {
        m_children.removeAll(child);
    }

    /// reimplemented
    void setClipped(const KoShape *, bool) override { }  // ignored
    /// reimplemented
    bool isClipped(const KoShape *) const override {
        return false;
    }// ignored
    /// reimplemented
    int count() const override {
        return m_children.count();
    }
    /// reimplemented
    QList<KoShape*> shapes() const override {
        return m_children;
    }
    /// reimplemented
    void containerChanged(KoShapeContainer *, KoShape::ChangeType) override {
        m_containerChangedCalled++;
    }
    /// reimplemented
    void proposeMove(KoShape *, QPointF &) override {
        m_proposeMoveCalled++;
    }
    /// reimplemented
    void childChanged(KoShape *, KoShape::ChangeType) override {
        m_childChangedCalled++;
    }
    void setInheritsTransform(const KoShape *, bool) override {
    }
    bool inheritsTransform(const KoShape *) const override {
        return false;
    }

    int containerChangedCalled() const {
        return m_containerChangedCalled;
    }
    int childChangedCalled() const {
        return m_childChangedCalled;
    }
    int proposeMoveCalled() const {
        return m_proposeMoveCalled;
    }

    void resetCounts() {
        m_containerChangedCalled = 0;
        m_childChangedCalled = 0;
        m_proposeMoveCalled = 0;
    }

private:
    QList<KoShape*> m_children;
    int m_containerChangedCalled, m_childChangedCalled, m_proposeMoveCalled;
};

#endif
