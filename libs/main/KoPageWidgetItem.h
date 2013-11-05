#ifndef KOPAGEWIDGETITEM_H
#define KOPAGEWIDGETITEM_H

#include <kpagewidgetmodel.h>
#include "komain_export.h"

// This class can be implemented when we want to extend the
// dialog with new, specific pages.
class KOMAIN_EXPORT KoPageWidgetItem
{

public:

    virtual ~KoPageWidgetItem() {}
    virtual QWidget *widget() = 0;
    virtual QString name() const = 0;
    virtual QString iconName() const = 0;
    virtual bool shouldDialogCloseBeVetoed() = 0;
    virtual void apply() = 0;
};


#endif // KOPAGEWIDGETITEM_H
