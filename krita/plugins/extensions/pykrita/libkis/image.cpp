#include "image.h"

Image::Image(KisImageWSP image, QObject *parent)
    : QObject(parent)
    , m_image(image)
{
}
