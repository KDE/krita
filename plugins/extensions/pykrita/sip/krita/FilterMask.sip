class FilterMask : Node
{
%TypeHeaderCode
#include "nodes/FilterMask.h"
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
