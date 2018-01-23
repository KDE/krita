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
    bool setGenerator(const QString &filterName, InfoObject *filterConfig);
    QString generatorName();
    InfoObject *filterConfig();
Q_SIGNALS:
private:
};
