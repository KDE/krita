%ModuleHeaderCode
#include "TransparencyMask.h"
%End

class TransparencyMask : Node
{
%TypeHeaderCode
#include "TransparencyMask.h"
%End

%ConvertToSubClassCode
    if(qobject_cast<TransparencyMask*>(sipCpp))
        sipType = sipType_TransparencyMask;
    else
        sipType = nullptr;
%End

    TransparencyMask(const TransparencyMask & __0);
public:
    virtual ~TransparencyMask();
    virtual QString type() const;
    Selection* selection() const;
    void setSelection(Selection *selection);
Q_SIGNALS:
private:
};

