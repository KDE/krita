/*
 *  This file is part of Calligra tests
 *
 *  Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
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
#ifndef MOCKSHAPES_H
#define MOCKSHAPES_H

#include <KoShapeGroup.h>
#include <KoCanvasBase.h>
#include <KoShapeBasedDocumentBase.h>
#include <KoShapeContainerModel.h>
#include <QPainter>
#include "KoShapeManager.h"
#include "kdebug.h"
#include "KoSnapData.h"

class MockShape : public KoShape
{
public:
    MockShape() : paintedCount(0) {}
    void paint(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
        //qDebug() << "Shape" << kBacktrace( 10 );
        paintedCount++;
    }
    virtual void saveOdf(KoShapeSavingContext &) const {}
    virtual bool loadOdf(const KoXmlElement &, KoShapeLoadingContext &) {
        return true;
    }
    int paintedCount;
};

class MockContainer : public KoShapeContainer
{
public:
    MockContainer(KoShapeContainerModel *model = 0) : KoShapeContainer(model), paintedCount(0) {}
    void paintComponent(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
        //qDebug() << "Container:" << kBacktrace( 10 );
        paintedCount++;
    }

    virtual void saveOdf(KoShapeSavingContext &) const {}
    virtual bool loadOdf(const KoXmlElement &, KoShapeLoadingContext &) {
        return true;
    }
    int paintedCount;
};

class MockGroup : public KoShapeGroup
{
    void paintComponent(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
    }
};

class KoToolProxy;

class MockCanvas : public KoCanvasBase
{
public:
    MockCanvas(KoShapeBasedDocumentBase *aKoShapeBasedDocumentBase =0)//made for TestSnapStrategy.cpp
            : KoCanvasBase(aKoShapeBasedDocumentBase), m_shapeManager(new KoShapeManager(this)), m_guideData(0) {}
    ~MockCanvas() {}
    void setHorz(qreal pHorz){
        m_horz = pHorz;
    }
    void setVert(qreal pVert){
        m_vert = pVert;
    }
    void setGuidesData(KoGuidesData* pGuideData){
        m_guideData = pGuideData;
    }
    void gridSize(qreal *horizontal, qreal *vertical) const {
        *horizontal = m_horz;
        *vertical = m_vert;
    }
    bool snapToGrid() const  {
        return true;
    }
    void addCommand(KUndo2Command*) { }
    KoShapeManager *shapeManager() const  {
        return m_shapeManager;
    }
    void updateCanvas(const QRectF&)  {}
    KoToolProxy * toolProxy() const {
        return 0;
    }
    KoViewConverter *viewConverter() const {
        return 0;
    }
    QWidget* canvasWidget() {
        return 0;
    }
    const QWidget* canvasWidget() const {
        return 0;
    }
    KoUnit unit() const {
        return KoUnit(KoUnit::Millimeter);
    }
    KoGuidesData *guidesData(){
        return m_guideData;
    }
    void updateInputMethodInfo() {}
    void setCursor(const QCursor &) {}
    private:
        KoShapeManager *m_shapeManager;
        qreal m_horz;
        qreal m_vert;
        KoGuidesData *m_guideData;
};

class MockShapeController : public KoShapeBasedDocumentBase
{
public:
    void addShape(KoShape* shape) {
        m_shapes.insert(shape);
    }
    void removeShape(KoShape* shape) {
        m_shapes.remove(shape);
    }
    bool contains(KoShape* shape) {
        return m_shapes.contains(shape);
    }
private:
    QSet<KoShape * > m_shapes;
};

class MockContainerModel : public KoShapeContainerModel
{
public:
    MockContainerModel() {
        resetCounts();
    }

    /// reimplemented
    void add(KoShape *child) {
        m_children.append(child); // note that we explicitly do not check for duplicates here!
    }
    /// reimplemented
    void remove(KoShape *child) {
        m_children.removeAll(child);
    }

    /// reimplemented
    void setClipped(const KoShape *, bool) { }  // ignored
    /// reimplemented
    bool isClipped(const KoShape *) const {
        return false;
    }// ignored
    /// reimplemented
    bool isChildLocked(const KoShape *child) const {
        return child->isGeometryProtected();
    }
    /// reimplemented
    int count() const {
        return m_children.count();
    }
    /// reimplemented
    QList<KoShape*> shapes() const {
        return m_children;
    }
    /// reimplemented
    void containerChanged(KoShapeContainer *, KoShape::ChangeType) {
        m_containerChangedCalled++;
    }
    /// reimplemented
    void proposeMove(KoShape *, QPointF &) {
        m_proposeMoveCalled++;
    }
    /// reimplemented
    void childChanged(KoShape *, KoShape::ChangeType) {
        m_childChangedCalled++;
    }
    void setInheritsTransform(const KoShape *, bool) {
    }
    bool inheritsTransform(const KoShape *) const {
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
