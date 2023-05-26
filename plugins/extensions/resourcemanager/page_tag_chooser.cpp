#include "page_tag_chooser.h"
#include "ui_pagetagchooser.h"

PageTagChooser::PageTagChooser(KoResourceBundleSP bundle, QWidget *parent) :
    QWizardPage(parent),
    m_ui(new Ui::PageTagChooser)
    , m_bundle(bundle)
{
    m_ui->setupUi(this);
}

PageTagChooser::~PageTagChooser()
{
    delete m_ui;
}
