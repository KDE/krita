#include "page_resource_chooser.h"
#include "ui_pageresourcechooser.h"
#include "wdg_resource_preview.h"


PageResourceChooser::PageResourceChooser(QWidget *parent) :
    QWizardPage(parent),
    m_ui(new Ui::PageResourceChooser)
{
    m_ui->setupUi(this);

    WdgResourcePreview *wdgResourcePreview = new WdgResourcePreview(1);
    m_ui->formLayout->addWidget(wdgResourcePreview);
}

PageResourceChooser::~PageResourceChooser()
{
    delete m_ui;
}
