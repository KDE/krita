#ifndef KISTOOLCHANGESTRACKERDATA_H
#define KISTOOLCHANGESTRACKERDATA_H

#include <QObject>
#include "kritaui_export.h"
#include <QSharedPointer>

class KRITAUI_EXPORT KisToolChangesTrackerData
{
public:
    virtual ~KisToolChangesTrackerData();
    virtual KisToolChangesTrackerData* clone() const;
};

typedef QSharedPointer<KisToolChangesTrackerData> KisToolChangesTrackerDataSP;

Q_DECLARE_METATYPE(KisToolChangesTrackerDataSP)

#endif // KISTOOLCHANGESTRACKERDATA_H
