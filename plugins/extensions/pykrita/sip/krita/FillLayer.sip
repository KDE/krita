class FillLayer : Node
{
%TypeHeaderCode
#include "nodes/FillLayer.h"
%End
    FillLayer(const FillLayer& __0);
public:
    virtual ~FillLayer();
    virtual QString type() const;
    void setFilter(QString &filterName, InfoObject *filterConfig);
    QString filterName();
    InfoObject * filterConfig();
Q_SIGNALS:
private:
};
