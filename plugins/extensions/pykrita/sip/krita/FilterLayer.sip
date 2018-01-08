%ModuleHeaderCode
#include "FilterLayer.h"
%End

class FilterLayer : Node
{
%TypeHeaderCode
#include "FilterLayer.h"
%End

%ConvertToSubClassCode
    if(qobject_cast<FilterLayer*>(sipCpp))
        sipType = sipType_FilterLayer;
    else
        sipType = nullptr;
%End

    FilterLayer(const FilterLayer & __0);
public:
    virtual ~FilterLayer();
    virtual QString type() const;
    void setFilter(Filter &filter);
    Filter * filter();
Q_SIGNALS:
private:
};
