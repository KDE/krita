#ifndef PAGE_RESOURCE_CHOOSER_H
#define PAGE_RESOURCE_CHOOSER_H

#include "wdg_resource_preview.h"

#include <QPainter>
#include <QWizardPage>

#include <KoResourceBundle.h>

namespace Ui {
class PageResourceChooser;
}

class PageResourceChooser : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageResourceChooser(KoResourceBundleSP bundle = nullptr, QWidget *parent = nullptr);
    ~PageResourceChooser();

private Q_SLOTS:
    void slotResourcesSelectionChanged(QModelIndex selected);
    void slotresourceTypeSelected(int);
    void slotRemoveSelected(bool);

public:
    QPixmap imageToIcon(const QImage &img, Qt::AspectRatioMode aspectRatioMode);
    QList<int> getSelectedResourcesIds();

private:
    Ui::PageResourceChooser *m_ui;
    WdgResourcePreview *m_wdgResourcePreview;
    QList<int> m_selectedResourcesIds;

    KoResourceBundleSP m_bundle;

};

#endif // PAGE_RESOURCE_CHOOSER_H
