#ifndef PAGE_TAG_CHOOSER_H
#define PAGE_TAG_CHOOSER_H

#include <QWizardPage>
#include <KoResourceBundle.h>


namespace Ui {
class PageTagChooser;
}

class PageTagChooser : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageTagChooser(KoResourceBundleSP bundle = nullptr, QWidget *parent = nullptr);
    ~PageTagChooser();

private:
    Ui::PageTagChooser *m_ui;
    KoResourceBundleSP m_bundle;
};

#endif // PAGE_TAG_CHOOSER_H
