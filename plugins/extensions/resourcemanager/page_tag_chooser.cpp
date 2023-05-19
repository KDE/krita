#include "page_tag_chooser.h"
#include "ui_pagetagchooser.h"

PageTagChooser::PageTagChooser(QWidget *parent) :
    QWizardPage(parent),
    m_ui(new Ui::PageTagChooser)
{
    m_ui->setupUi(this);
}

PageTagChooser::~PageTagChooser()
{
    delete m_ui;
}
