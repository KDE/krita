#include "kis_color_selector_component.h"

#include "kis_color_selector_base.h"

#include "KoColorSpace.h"


KisColorSelectorComponent::KisColorSelectorComponent(KisColorSelectorBase* parent) :
    QWidget(parent), m_parent(parent)
{
    Q_ASSERT(parent);
}

const KoColorSpace* KisColorSelectorComponent::colorSpace() const
{
    const KoColorSpace* cs = m_parent->colorSpace();
    Q_ASSERT(cs);
    return cs;
}
