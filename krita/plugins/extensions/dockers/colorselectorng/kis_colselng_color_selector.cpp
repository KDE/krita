#include "kis_colselng_color_selector.h"

#include <QHBoxLayout>

#include <KoTriangleColorSelector.h>

KisColSelNgColorSelector::KisColSelNgColorSelector(QWidget* parent) : QWidget(parent)
{
    QWidget* triangleSelector = new KoTriangleColorSelector(this);
    QHBoxLayout* layout = new QHBoxLayout(this);

    layout->addWidget(triangleSelector);
}
