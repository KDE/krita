class Channel : QObject
{
%TypeHeaderCode
#include "Channel.h"
%End
    Channel(const Channel & __0);
public:
    Channel(QObject*  parent /TransferThis/ = 0);
    virtual ~Channel();
    bool visible() const;
    void setvisible(bool value);
public Q_SLOTS:
Q_SIGNALS:
private:

};
