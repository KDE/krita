class SelectionMask : Node
{
%TypeHeaderCode
#include "nodes/SelectionMask.h"
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
