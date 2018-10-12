#ifndef KISDELETELATERWRAPPER_H
#define KISDELETELATERWRAPPER_H

#include "kritaglobal_export.h"
#include <QObject>

namespace KisDeleteLaterWrapperPrivate {
KRITAGLOBAL_EXPORT void moveToGuiThread(QObject *object);
}

template <typename T>
class KisDeleteLaterWrapper : public QObject
{
public:
    KisDeleteLaterWrapper(T value)
        : m_value(value)
    {
        KisDeleteLaterWrapperPrivate::moveToGuiThread(this);
    }

private:
    T m_value;
};

template <typename T>
class KisDeleteLaterWrapper<T*> : public QObject
{
public:
    KisDeleteLaterWrapper(T* value)
        : m_value(value)
    {
        KisDeleteLaterWrapperPrivate::moveToGuiThread(this);
    }

    ~KisDeleteLaterWrapper() {
        delete m_value;
    }

private:
    T *m_value;
};

template <typename T>
KisDeleteLaterWrapper<T>* makeKisDeleteLaterWrapper(T value) {
    return new KisDeleteLaterWrapper<T>(value);
}

#endif // KISDELETELATERWRAPPER_H
