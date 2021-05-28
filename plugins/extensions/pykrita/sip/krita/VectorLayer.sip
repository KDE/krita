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
Q_SIGNALS:
private:
};
