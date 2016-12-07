class Filter : QObject
{
%TypeHeaderCode
#include "Filter.h"
%End
    Filter(const Filter & __0);
public:
    Filter(QObject*  parent /TransferThis/ = 0);
    virtual ~Filter();
    InfoObject * configuration() const;
    void setConfiguration(InfoObject*  value);
public Q_SLOTS:
    void Apply(int x, int y, int w, int h);
Q_SIGNALS:
private:
};
