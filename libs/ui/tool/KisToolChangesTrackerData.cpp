#include "KisToolChangesTrackerData.h"

struct KisToolChangesTrackerDataSPRegistrar {
    KisToolChangesTrackerDataSPRegistrar() {
        qRegisterMetaType<KisToolChangesTrackerDataSP>("KisToolChangesTrackerDataSP");
    }
};
static KisToolChangesTrackerDataSPRegistrar __registrar;


KisToolChangesTrackerData::~KisToolChangesTrackerData()
{
}

KisToolChangesTrackerData *KisToolChangesTrackerData::clone() const
{
    return new KisToolChangesTrackerData(*this);
}
