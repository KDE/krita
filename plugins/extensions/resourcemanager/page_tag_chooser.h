#ifndef PAGE_TAG_CHOOSER_H
#define PAGE_TAG_CHOOSER_H

#include <QWizardPage>
#include <KoResourceBundle.h>
#include "KisTagSelectionWidget.h"
#include <KisTag.h>

// class KisActionManager;
// class KisTagModel;
// class KisTagFilterResourceProxyModel;
// class KisTag;
// class KisWdgTagSelectionControllerBundleTags;


namespace Ui {
class PageTagChooser;
}

class PageTagChooser : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageTagChooser(KoResourceBundleSP bundle = nullptr, QWidget *parent = nullptr);
    ~PageTagChooser();

    QList<int> selectedTagIds();
    void updateTags(bool flag, QString tag);

Q_SIGNALS:
    void tagsUpdated();

private Q_SLOTS:

    void addSelected(KisTagSP tagSP);
    void removeSelected(KisTagSP tagSP);
    void resourceTypeSelected(int idx);

private:
    Ui::PageTagChooser *m_ui;
    KoResourceBundleSP m_bundle;

    QList<int> m_selectedTagIds;
/*
    QScopedPointer<KisWdgTagSelectionControllerBundleTags> m_tagsController;
    KisTagSelectionWidget* m_wdgResourcesTags;*/
};

#endif // PAGE_TAG_CHOOSER_H
