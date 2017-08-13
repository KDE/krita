#include "KisLoggingManager.h"

#include <QSet>
#include <QLoggingCategory>

using ScopedLogCapturer = KisLoggingManager::ScopedLogCapturer;

namespace
{
    QtMessageHandler oldMessageHandler;
    QLoggingCategory::CategoryFilter oldCategoryFilter;

    QSet<const ScopedLogCapturer *> capturerSet;
} // namespace

class KisLoggingManager::Private
{
    friend class KisLoggingManager;

    static void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        foreach (const ScopedLogCapturer *const &capturer, capturerSet) {
            if (capturer->m_category == context.category) {
                capturer->m_callback(type, context, msg);
            }
        }
        // TODO: Hide capture-only messages from default output
        oldMessageHandler(type, context, msg);
    }

    static void myCategoryFilter(QLoggingCategory *category)
    {
        oldCategoryFilter(category);
        // Enable categories to be captured
        // TODO: Keep track of default filter stage to hide message from output
        foreach (const ScopedLogCapturer *const &capturer, capturerSet) {
            if (capturer->m_category == category->categoryName()) {
                category->setEnabled(QtDebugMsg, true);
                category->setEnabled(QtInfoMsg, true);
                category->setEnabled(QtWarningMsg, true);
                category->setEnabled(QtCriticalMsg, true);
            }
        }
    }

    static void refreshCategoryFilter()
    {
        QLoggingCategory::installFilter(myCategoryFilter);
    }
}; // class KisLoggingManager::Private

void KisLoggingManager::initialize()
{
    // Install our QtMessageHandler for capturing logging messages
    oldMessageHandler = qInstallMessageHandler(KisLoggingManager::Private::myMessageHandler);
    // HACK: Gets the default CategoryFilter because the filter function may
    //       be called synchronously.
    oldCategoryFilter = QLoggingCategory::installFilter(nullptr);
    // Install our CategoryFilter for filtering
    KisLoggingManager::Private::refreshCategoryFilter();
}

ScopedLogCapturer::ScopedLogCapturer(QByteArray category, ScopedLogCapturer::callback_t callback)
    : m_category(category)
    , m_callback(callback)
{
    capturerSet.insert(this);
    KisLoggingManager::Private::refreshCategoryFilter();
}

ScopedLogCapturer::~ScopedLogCapturer()
{
    capturerSet.remove(this);
    KisLoggingManager::Private::refreshCategoryFilter();
}
