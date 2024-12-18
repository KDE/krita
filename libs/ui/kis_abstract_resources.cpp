#include "kis_abstract_resources.h"
#include "kis_canvas_resource_provider.h"


/*********************************************************************/
/*                 ToolOpacityAbstractResource                       */
/*********************************************************************/
ToolOpacityAbstractResource::ToolOpacityAbstractResource(int key, qreal value)
    :KoAbstractCanvasResourceInterface(key, "debug")
    , m_value(value)
{
}

QVariant ToolOpacityAbstractResource::value() const {
    return QVariant::fromValue(m_value);
}

void ToolOpacityAbstractResource::setValue(const QVariant value) {
    m_value = value.value<qreal>();
}
