#include "image.h"

Image::Image(QObject *image, QObject *parent)
    : QObject(parent)
    , m_image(qobject_cast<KisImage*>(image))
{
}
