#include "KisResourceDirtyStateSaver.h"
#include <KoResource.h>

#include "kritaresources_export.h"

KisResourceDirtyStateSaver::KisResourceDirtyStateSaver(KoResourceSP resource)
    : m_resource(resource)
    , m_isDirty(resource->isDirty())

{
}

/// Extra constructor to be called from KoResource itself
KisResourceDirtyStateSaver::KisResourceDirtyStateSaver(KoResource *resource)
    : m_parentResource(resource)
    , m_isDirty(resource->isDirty())
{
}

KisResourceDirtyStateSaver::~KisResourceDirtyStateSaver() {
    if (m_resource) {
        m_resource->setDirty(m_isDirty);
    }
    else if (m_parentResource) {
        m_parentResource->setDirty(m_isDirty);
    }
}

