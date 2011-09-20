#include "BibliographyConfigureDialog.h"
#include "ui_BibliographyConfigureDialog.h"

BibliographyConfigureDialog::BibliographyConfigureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BibliographyConfigureDialog)
{
    ui->setupUi(this);
}

BibliographyConfigureDialog::~BibliographyConfigureDialog()
{
    delete ui;
}
