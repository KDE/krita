class ColorDepth : QObject
{
%TypeHeaderCode
#include "ColorDepth.h"
%End
    ColorDepth(const ColorDepth & __0);
public:
    ColorDepth(QObject*  parent /TransferThis/ = 0);
    virtual ~ColorDepth();
    int depth() const;
    void setdepth(int value);
public Q_SLOTS:
Q_SIGNALS:
private:
};
