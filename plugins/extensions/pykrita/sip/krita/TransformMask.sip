%ModuleHeaderCode
#include "TransformMask.h"
%End

class TransformMask : Node
{
%TypeHeaderCode
#include "TransformMask.h"
%End

%ConvertToSubClassCode
    if(qobject_cast<TransformMask*>(sipCpp))
        sipType = sipType_TransformMask;
    else
        sipType = nullptr;
%End

    TransformMask(const TransformMask & __0);
public:
    virtual ~TransformMask();
    virtual QString type() const;
    QTransform finalAffineTransform() const;
Q_SIGNALS:
private:
};
