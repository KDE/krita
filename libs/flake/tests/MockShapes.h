#include <KoShapeGroup.h>
#include <KoCanvasBase.h>
#include <KoShapeControllerBase.h>

#include "kdebug.h"

#ifndef MOCKSHAPES_H
#define MOCKSHAPES_H

class MockShape : public KoShape {
public:
    MockShape() : paintedCount(0) {}
    void paint(QPainter &painter, const KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
        //qDebug() << "Shape" << kBacktrace( 10 );
        paintedCount++;
    }
    int paintedCount;
};

class MockContainer : public KoShapeContainer {
public:
    MockContainer() : paintedCount(0) {}
    void paintComponent(QPainter &painter, const KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
        //qDebug() << "Container:" << kBacktrace( 10 );
        paintedCount++;
    }

    int paintedCount;
};

class MockGroup : public KoShapeGroup {
    void paintComponent(QPainter &painter, const KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
    }
};

class KCommand;
class KoToolProxy;

class MockCanvas : public KoCanvasBase {
public:
    MockCanvas()
    : KoCanvasBase( 0 ) 
    {}
    ~MockCanvas() {}

    void gridSize(double *, double *) const {}
    bool snapToGrid() const  { return false; }
    void addCommand(QUndoCommand*) { }
    KoShapeManager *shapeManager() const  { return 0; }
    void updateCanvas(const QRectF& )  {}
    KoToolProxy * toolProxy() { return 0; }
    KoViewConverter *viewConverter() { return 0; }
    QWidget* canvasWidget() { return 0; }
    KoUnit unit() { return KoUnit(KoUnit::Millimeter); }
};

class MockViewConverter : public KoViewConverter {
    QPointF documentToView( const QPointF &documentPoint ) const { return documentPoint; }
    QPointF viewToDocument( const QPointF &viewPoint ) const { return viewPoint; }
    QRectF documentToView( const QRectF &documentRect ) const { return documentRect; }
    QRectF viewToDocument( const QRectF &viewRect ) const { return viewRect; }
    QSizeF documentToView( const QSizeF& documentSize ) const { return documentSize; }
    QSizeF viewToDocument( const QSizeF& viewSize ) const { return viewSize; }
    double documentToViewX( double documentX ) const { return documentX; }
    double documentToViewY( double documentY ) const{ return documentY; }
    double viewToDocumentX( double viewX ) const { return viewX; }
    double viewToDocumentY( double viewY ) const { return viewY; }
    void zoom(double *zoomX, double *zoomY) const { *zoomX = 1.0; *zoomY = 1.0; }
};

class MockShapeController : public KoShapeControllerBase
{
public:
    void addShape( KoShape* shape )
    {
        m_shapes.insert( shape );
    }
    void removeShape( KoShape* shape )
    {
        m_shapes.remove( shape );
    }
    bool contains(  KoShape* shape )
    {
        return m_shapes.contains( shape );
    }
private:
    QSet<KoShape * > m_shapes;
};

#endif
