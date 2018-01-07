class VectorLayer : Node
{
%TypeHeaderCode
#include "VectorLayer.h"
%End
   VectorLayer(const VectorLayer & __0);
public:
    virtual ~VectorLayer();
public Q_SLOTS:
    virtual QString type() const;
    QList<Shape *> shapes() const /Factory/;
Q_SIGNALS:
private:
};
