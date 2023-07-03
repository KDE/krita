

#ifndef KISCURSOROVERRIDELOCK_H
#define KISCURSOROVERRIDELOCK_H

#include <kritaglobal_export.h>
#include <QCursor>
#include <mutex>

class KRITAGLOBAL_EXPORT KisCursorOverrideLockAdapter
{
public:
    KisCursorOverrideLockAdapter(const QCursor &cursor);
    ~KisCursorOverrideLockAdapter();

    void lock();
    void unlock();

private:
    QCursor m_cursor;
};

struct KisCursorOverrideLock
    : private KisCursorOverrideLockAdapter,
      public std::unique_lock<KisCursorOverrideLockAdapter>
{
    KisCursorOverrideLock(const QCursor &cursor)
        : KisCursorOverrideLockAdapter(cursor)
        , std::unique_lock<KisCursorOverrideLockAdapter>(
              static_cast<KisCursorOverrideLockAdapter&>(*this))
    {}

    using std::unique_lock<KisCursorOverrideLockAdapter>::lock;
    using std::unique_lock<KisCursorOverrideLockAdapter>::unlock;
};

#endif // KISCURSOROVERRIDELOCK_H
