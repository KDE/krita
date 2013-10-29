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
    virtual const QString name() const = 0;
    virtual const QLatin1String icon() const = 0;
    virtual bool shouldDialogCloseBeVetoed() = 0;
    virtual void apply() = 0;
};


#endif // KOPAGEWIDGETITEM_H
