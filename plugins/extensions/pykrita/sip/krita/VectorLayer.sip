class VectorLayer : Node
{
%TypeHeaderCode
#include "nodes/VectorLayer.h"
%End
   VectorLayer(const VectorLayer & __0);
public:
    virtual ~VectorLayer();
    virtual QString type() const;
    QList<Shape*> shapes() const;
Q_SIGNALS:
private:
};
