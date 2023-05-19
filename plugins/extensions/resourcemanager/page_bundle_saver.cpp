#include "page_bundle_saver.h"
#include "ui_pagebundlesaver.h"

PageBundleSaver::PageBundleSaver(QWidget *parent) :
    QWizardPage(parent),
    m_ui(new Ui::PageBundleSaver)
{
    m_ui->setupUi(this);
}

PageBundleSaver::~PageBundleSaver()
{
    delete m_ui;
}
