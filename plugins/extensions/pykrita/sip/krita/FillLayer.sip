%ModuleHeaderCode
#include "FillLayer.h"
%End

class FillLayer : Node
{
%TypeHeaderCode
#include "FillLayer.h"
%End

%ConvertToSubClassCode
    if(qobject_cast<FillLayer*>(sipCpp))
        sipType = sipType_FillLayer;
    else
        sipType = nullptr;
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
