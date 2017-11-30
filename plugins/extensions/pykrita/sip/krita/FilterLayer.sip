class FilterLayer : Node
{
%TypeHeaderCode
#include "nodes/FilterLayer.h"
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
