#include "KisRecentDocumentsModelWrapper.h"
#include "kis_icon_utils.h"

#include <utils/KisFileIconCreator.h>

#include <QtConcurrent>
#include <QSharedPointer>
#include <QUrl>


/*
 * Paramaters that we can map (using a function) to a
 * @see IconFetched
 */
struct GetFileIconParameters
{
    QUrl m_documentUrl;
    QSize m_iconSize;
    qreal m_devicePixelRatioF;
    int m_row;
    int m_workerId;
};
using ManyGetFileIconParameters=QVector<GetFileIconParameters>;
using ManyGetFileIconParametersPtr=QSharedPointer<ManyGetFileIconParameters>;
/**
 * An iterator that preserves its original container of file icon parameters
 *
 * Isn't it simpler to use a QObject wrapper and connect deleteLater() with FutureIconWatcher::finished?
 * That's a good idea. Unfortunately, QObjects cannot be shared among threads
 */
class SelfContainedIterator : public ManyGetFileIconParameters::const_iterator
{
    using Iterator=ManyGetFileIconParameters::const_iterator;
    ManyGetFileIconParametersPtr m_originalManyGetFileIconParameters;
    SelfContainedIterator(ManyGetFileIconParametersPtr originalManyGetFileIconParameters)
        : Iterator((*originalManyGetFileIconParameters).cbegin())
        , m_originalManyGetFileIconParameters(originalManyGetFileIconParameters)
    {
    }
    SelfContainedIterator() = delete; // ensure an end() is properly built
    /**
     * end() iterator
     */
    SelfContainedIterator(ManyGetFileIconParameters::const_iterator iter)
        :ManyGetFileIconParameters::const_iterator(iter)
    {
    }
public:
    /**
     * Single entry construction of iterators where a begin() should always go with an end()
     */
    static QPair<SelfContainedIterator,SelfContainedIterator> range(ManyGetFileIconParametersPtr originalManyGetFileIconParameters){
        return qMakePair(SelfContainedIterator(originalManyGetFileIconParameters),
                SelfContainedIterator(originalManyGetFileIconParameters->cend()));
    }

}; // class SelfContainedIterator
struct SelfContainedIteratorRange
{
    SelfContainedIterator m_begin;
    SelfContainedIterator m_end;
    SelfContainedIteratorRange(QPair<SelfContainedIterator,SelfContainedIterator> range)
        : m_begin(range.first)
        , m_end(range.second)
    {
    }
};

KisRecentDocumentsModelWrapper::IconFetchResult getFileIcon(GetFileIconParameters gfip)
{

    KisFileIconCreator iconCreator;
    KisRecentDocumentsModelWrapper::IconFetchResult iconFetched;
    iconFetched.m_workerId = gfip.m_workerId;
    iconFetched.m_row = gfip.m_row;
    iconFetched.m_documentUrl = gfip.m_documentUrl;
    iconFetched.m_iconWasFetchedOk = iconCreator.createFileIcon(gfip.m_documentUrl.toLocalFile(),
                                                              iconFetched.m_icon,
                                                              gfip.m_devicePixelRatioF,
                                                              gfip.m_iconSize);
    return iconFetched;
}


KisRecentDocumentsModelWrapper::KisRecentDocumentsModelWrapper(){
    connect(&m_iconWorkerWatcher, SIGNAL(resultReadyAt(int)),
            this, SLOT(slotIconReady(int)));
    connect(&m_iconWorkerWatcher, SIGNAL(finished()),
            this, SLOT(slotIconFetchingFinished()));
    connect(&m_iconWorkerWatcher, SIGNAL(canceled()),
            this, SLOT(slotIconFetchingFinished()));
}

KisRecentDocumentsModelWrapper::~KisRecentDocumentsModelWrapper(){
    cancelAndWaitIfRunning();
}
void KisRecentDocumentsModelWrapper::setFiles(const URLs &urls, qreal devicePixelRatioF){
    const QSize iconSize(ICON_SIZE_LENGTH, ICON_SIZE_LENGTH);

    m_currentWorkerId++;
    // before launching any thread, make sure that there's no working being done
    cancelAndWaitIfRunning();

    // update model
    QList<QStandardItem *> items;
    const QIcon stubIcon = KisIconUtils::loadIcon("media-floppy");
    {
        Q_FOREACH(const QUrl &recentFileUrl, urls){
            const QString recentFileUrlPath = recentFileUrl.toLocalFile();
            QStandardItem *recentItem = new QStandardItem(stubIcon, recentFileUrl.fileName());
            recentItem->setData(recentFileUrl);
            recentItem->setToolTip(recentFileUrlPath);
            items.append(recentItem);
        }
    }
    m_filesAndThumbnailsModel.clear(); // clear existing data before it gets re-populated
    Q_FOREACH(QStandardItem *item, items) {
        m_filesAndThumbnailsModel.appendRow(item);
    }
    ManyGetFileIconParametersPtr manyFileIconParametersPtr = ManyGetFileIconParametersPtr::create();
    ManyGetFileIconParameters &manyFileIconParameters = *manyFileIconParametersPtr;

    // launch thread
    // first, collect items without a cached icon

    int row=0;
    Q_FOREACH(const QUrl &recentFileUrl, urls){

        // if icon is not in cache, generate icon
        // else, use the one in cache
        if(!m_filePathToIconCache.contains(recentFileUrl.toLocalFile()))
            manyFileIconParameters.push_back(GetFileIconParameters{ recentFileUrl, iconSize, devicePixelRatioF, row, m_currentWorkerId });
        else
            m_filesAndThumbnailsModel.item(row)->setIcon(
               m_filePathToIconCache[recentFileUrl.toLocalFile()]);
        row++;
    }

    if (manyFileIconParameters.empty()) {
        slotIconFetchingFinished();
        return;
    }

    // use c++17's decomposition declaration when available
    SelfContainedIteratorRange range(SelfContainedIterator::range(manyFileIconParametersPtr));
    QFuture<IconFetchResult> mapFuture =
        QtConcurrent::mapped( range.m_begin, range.m_end, getFileIcon );
    m_iconWorkerWatcher.setFuture(mapFuture);
}
void KisRecentDocumentsModelWrapper::slotIconReady(int row){
    IconFetchResult iconFetched = m_iconWorkerWatcher.resultAt(row);
    // if we have a valid icon, we want to keep it in cache… regardless of whether it ends up being useful
    if(iconFetched.m_iconWasFetchedOk && !m_filePathToIconCache.contains(iconFetched.m_documentUrl.toLocalFile()))
        m_filePathToIconCache[iconFetched.m_documentUrl.toLocalFile()] = iconFetched.m_icon;

    if(m_currentWorkerId != iconFetched.m_workerId)
        return; // the icon arrived too late: we've changed the model since
    QStandardItem *updatingItem = m_filesAndThumbnailsModel.item(iconFetched.m_row);
    updatingItem->setEnabled(iconFetched.m_iconWasFetchedOk);
    if(!iconFetched.m_iconWasFetchedOk)
    {
        emit sigInvalidDocumentForIcon(iconFetched.m_documentUrl);
        return; // nothing to do with the icon

    }

    // set icon
    updatingItem->setIcon(iconFetched.m_icon);
}

QStandardItemModel &KisRecentDocumentsModelWrapper::model(){
    return m_filesAndThumbnailsModel;
}

void KisRecentDocumentsModelWrapper::slotIconFetchingFinished(){
    QVector<int> shouldRemoveTheseRows;
    for(int row=m_filesAndThumbnailsModel.rowCount()-1; row >= 0 ; row--)
    {
        QStandardItem *item = m_filesAndThumbnailsModel.item(row);
        if(!item->isEnabled())
            shouldRemoveTheseRows.push_back(row);
    }

    Q_FOREACH(int rowToBeRemoved, shouldRemoveTheseRows){
        m_filesAndThumbnailsModel.removeRow(rowToBeRemoved);
    }
    emit sigModelIsUpToDate();
}

void KisRecentDocumentsModelWrapper::cancelAndWaitIfRunning(){
    if(m_iconWorkerWatcher.isRunning())
    {
       m_iconWorkerWatcher.cancel();
       m_iconWorkerWatcher.waitForFinished();
    }
}
