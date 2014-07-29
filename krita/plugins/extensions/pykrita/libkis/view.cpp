#include "view.h"

View::View(KisView2 *view, QObject *parent)
    : QObject(parent)
    , m_view(view)
{
}
