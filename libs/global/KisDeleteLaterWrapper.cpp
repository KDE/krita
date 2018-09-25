#include "KisDeleteLaterWrapper.h"

#include <QApplication>


void KisDeleteLaterWrapperPrivate::moveToGuiThread(QObject *object)
{
    object->moveToThread(qApp->thread());
}
