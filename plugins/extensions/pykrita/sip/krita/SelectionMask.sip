%ModuleHeaderCode
#include "SelectionMask.h"
%End

class SelectionMask : Node
{
%TypeHeaderCode
#include "SelectionMask.h"
%End

%ConvertToSubClassCode
    if(qobject_cast<SelectionMask*>(sipCpp))
        sipType = sipType_SelectionMask;
    else
        sipType = nullptr;
%End

    SelectionMask(const SelectionMask & __0);
public:
    virtual ~SelectionMask();
    virtual QString type() const;
    Selection* selection() const;
    void setSelection(Selection *selection);
Q_SIGNALS:
private:
};
