#ifndef KISLOGGINGMANAGER_H
#define KISLOGGINGMANAGER_H

#include "kritaglobal_export.h"

#include <QtGlobal>
#include <QByteArray>

#include <functional>
#include <type_traits>

/**
 * This static class is used to control the Qt logging infrastructure to suit
 * our needs.
 *
 * This class assumes no other code calls qInstallMessageHandler and
 * QLoggingCategory::installFilter.
 *
 * This class might or might not be thread-safe, please only use this class
 * on the main GUI thread. There is no checking or synchronization in place.
 */
class KRITAGLOBAL_EXPORT KisLoggingManager
{
public:
    /**
     * Initialize KisLoggingManager globally..
     * This function should be called as early as possible in main().
     */
    static void initialize();

    class ScopedLogCapturer;

private:
    KisLoggingManager() = delete;
    class Private;
}; // class KisLoggingManager

/**
 * This class is used to capture logging output within a certain scope.
 *
 * This class might or might not be thread-safe, please only use this class
 * on the main GUI thread. There is no checking or synchronization in place.
 */
class KRITAGLOBAL_EXPORT KisLoggingManager::ScopedLogCapturer
{
    friend class KisLoggingManager;
    using callback_t = std::function<std::remove_pointer<QtMessageHandler>::type>;

public:
    ScopedLogCapturer(QByteArray category, callback_t callback);
    ~ScopedLogCapturer();

private:
    QByteArray m_category;
    callback_t m_callback;

    ScopedLogCapturer(const ScopedLogCapturer &) = delete;
    ScopedLogCapturer &operator=(const ScopedLogCapturer &) = delete;
}; // class KisLoggingManager::ScopedLogCapturer

#endif // KISLOGGINGMANAGER_H
