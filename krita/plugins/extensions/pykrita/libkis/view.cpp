#include "view.h"

#include <kis_view2.h>

View::View(QObject *view, QObject *parent)
    : QObject(parent)
    , m_view(qobject_cast<KisView2*>(view))
{
}
