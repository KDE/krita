%ModuleHeaderCode
#include "VectorLayer.h"
%End

class VectorLayer : Node
{
%TypeHeaderCode
#include "VectorLayer.h"
%End

%ConvertToSubClassCode
    if(qobject_cast<VectorLayer*>(sipCpp))
        sipType = sipType_VectorLayer;
    else
        sipType = nullptr;
%End

    VectorLayer(const VectorLayer & __0);
public:
    virtual ~VectorLayer();
public Q_SLOTS:
    virtual QString type() const;
    QList<Shape *> shapes() const /Factory/;
    QString toSvg();
    QList<Shape *> addShapesFromSvg(const QString &svg) /Factory/;
    Shape* shapeAtPosition(const QPointF &point) const /Factory/;
    QList<Shape *> shapesInRect(const QRectF &rect, bool omitHiddenShapes = true, bool containedMode = false) const /Factory/;
    Shape* createGroupShape(const QString &name, QList<Shape *> shapes) const /Factory/;
Q_SIGNALS:
private:
};
