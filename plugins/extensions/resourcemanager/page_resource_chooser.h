#ifndef PAGE_RESOURCE_CHOOSER_H
#define PAGE_RESOURCE_CHOOSER_H

#include <QWizardPage>

namespace Ui {
class PageResourceChooser;
}

class PageResourceChooser : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageResourceChooser(QWidget *parent = nullptr);
    ~PageResourceChooser();

private:
    Ui::PageResourceChooser *m_ui;
};

#endif // PAGE_RESOURCE_CHOOSER_H
