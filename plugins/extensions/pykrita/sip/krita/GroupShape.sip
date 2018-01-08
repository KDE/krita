%ModuleHeaderCode
#include "GroupShape.h"
%End

class GroupShape : Shape
{
%TypeHeaderCode
#include "GroupShape.h"
%End

%ConvertToSubClassCode
    if(qobject_cast<GroupShape*>(sipCpp))
        sipType = sipType_GroupShape;
    else
        sipType = nullptr;
%End
    GroupShape(const GroupShape & __0);
public:
    virtual ~GroupShape();
    QList <Shape*> children();
Q_SIGNALS:
private:
};
