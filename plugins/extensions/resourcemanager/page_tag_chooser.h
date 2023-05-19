#ifndef PAGE_TAG_CHOOSER_H
#define PAGE_TAG_CHOOSER_H

#include <QWizardPage>

namespace Ui {
class PageTagChooser;
}

class PageTagChooser : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageTagChooser(QWidget *parent = nullptr);
    ~PageTagChooser();

private:
    Ui::PageTagChooser *m_ui;
};

#endif // PAGE_TAG_CHOOSER_H
