%ModuleHeaderCode
#include "FilterMask.h"
%End

class FilterMask : Node
{
%TypeHeaderCode
#include "FilterMask.h"
%End

%ConvertToSubClassCode
    if(qobject_cast<FilterMask*>(sipCpp))
        sipType = sipType_FilterMask;
    else
        sipType = nullptr;
%End

    FilterMask(const FilterMask & __0);
public:
    virtual ~FilterMask();
    virtual QString type() const;
    void setFilter(Filter &filter);
    Filter * filter();
Q_SIGNALS:
private:
};
