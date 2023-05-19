#ifndef PAGE_BUNDLE_SAVER_H
#define PAGE_BUNDLE_SAVER_H

#include <QWizardPage>

namespace Ui {
class PageBundleSaver;
}

class PageBundleSaver : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageBundleSaver(QWidget *parent = nullptr);
    ~PageBundleSaver();

private:
    Ui::PageBundleSaver *m_ui;
};

#endif // PAGE_BUNDLE_SAVER_H
