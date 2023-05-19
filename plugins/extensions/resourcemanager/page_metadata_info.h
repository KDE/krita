#ifndef PAGE_METADATA_INFO_H
#define PAGE_METADATA_INFO_H

#include <QWizardPage>

namespace Ui {
class PageMetadataInfo;
}

class PageMetadataInfo : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageMetadataInfo(QWidget *parent = nullptr);
    ~PageMetadataInfo();

private:
    Ui::PageMetadataInfo *m_ui;
};

#endif // PAGE_METADATA_INFO_H
