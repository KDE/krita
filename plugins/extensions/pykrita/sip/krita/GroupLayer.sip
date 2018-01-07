%ModuleHeaderCode
#include "GroupLayer.h"
%End

class GroupLayer : Node
{
%TypeHeaderCode
#include "GroupLayer.h"
%End

%ConvertToSubClassCode
    if(qobject_cast<GroupLayer*>(sipCpp))
        sipType = sipType_GroupLayer;
    else
        sipType = nullptr;
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
