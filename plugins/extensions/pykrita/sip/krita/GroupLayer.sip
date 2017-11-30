class GroupLayer : Node
{
%TypeHeaderCode
#include "nodes/GroupLayer.h"
%End
    GroupLayer(const GroupLayer & __0);
public:
    virtual ~GroupLayer();
    virtual QString type() const;
    void setPassThroughMode(bool passthrough);
    bool passThroughMode();
Q_SIGNALS:
private:
};
