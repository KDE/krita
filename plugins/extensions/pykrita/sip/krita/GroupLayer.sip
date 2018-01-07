class GroupLayer : Node
{
%TypeHeaderCode
#include "GroupLayer.h"
%End
    GroupLayer(const GroupLayer & __0);
public:
    virtual ~GroupLayer();
public Q_SLOTS:
    virtual QString type() const;
    void setPassThroughMode(bool passthrough);
    bool passThroughMode() const;
Q_SIGNALS:
private:
};
