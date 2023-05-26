#ifndef PAGE_BUNDLE_SAVER_H
#define PAGE_BUNDLE_SAVER_H

#include <QWizardPage>
#include <KoResourceBundle.h>


namespace Ui {
class PageBundleSaver;
}

class PageBundleSaver : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageBundleSaver(KoResourceBundleSP bundle = nullptr, QWidget *parent = nullptr);
    ~PageBundleSaver();

    QString saveLocation() const;
    void showWarning();
    void removeWarning();

private Q_SLOTS:
    void selectSaveLocation();


private:
    Ui::PageBundleSaver *m_ui;
    KoResourceBundleSP m_bundle;
};

#endif // PAGE_BUNDLE_SAVER_H
