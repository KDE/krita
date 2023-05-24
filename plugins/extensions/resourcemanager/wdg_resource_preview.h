#ifndef WDG_RESOURCE_PREVIEW_H
#define WDG_RESOURCE_PREVIEW_H

#include <QWidget>
#include <QScopedPointer>
// #include <QSortFilterProxyModel>
#include <QSharedPointer>
#include <QItemSelection>
#include <KisResourceThumbnailPainter.h>

class KisActionManager;
class KisResourceTypeModel;
class KisStorageModel;
class KisTagModel;
class KisResourceModel;
class KisTagFilterResourceProxyModel;
class KisTag;
class KisWdgTagSelectionControllerOneResource;

namespace Ui {
class WdgResourcePreview;
}

class WdgResourcePreview : public QWidget
{
    Q_OBJECT

public:
    explicit WdgResourcePreview(int type, QWidget *parent = nullptr);
    ~WdgResourcePreview();

Q_SIGNALS:
    void signalResourcesSelectionChanged(QModelIndex selected);
    void resourceTypeSelected(int);

private Q_SLOTS:
    void slotResourceTypeSelected(int);
    void slotTagSelected(int);
    void slotStorageSelected(int);

    void slotFilterTextChanged(const QString& filterText);
    void slotShowDeletedChanged(int newState);

public:
    QString getCurrentResourceType();
    QSharedPointer<KisTag> getCurrentTag();
    QModelIndexList geResourceItemsSelected();
    int getCurrentStorageId();
    QMap<QString, KisTagFilterResourceProxyModel*> getResourceProxyModelsForResourceType();

    QAbstractItemModel* getModel();

private:
    Ui::WdgResourcePreview *m_ui;
    int m_type;

    QList<int> m_selectedResourcesIds;

    KisResourceTypeModel *m_resourceTypeModel {0};
    KisStorageModel *m_storageModel {0};
    QMap<QString, KisTagModel*> m_tagModelsForResourceType;
    KisResourceModel *m_resourceModel {nullptr};

    QMap<QString, KisTagFilterResourceProxyModel*> m_resourceProxyModelsForResourceType;
    KisResourceThumbnailPainter m_thumbnailPainter;
};

#endif // WDG_RESOURCE_PREVIEW_H
