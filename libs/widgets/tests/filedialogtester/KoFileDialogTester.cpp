#include "KoFileDialogTester.h"
#include "ui_KoFileDialogTester.h"

KoFileDialogTester::KoFileDialogTester(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KoFileDialogTester)
{
    ui->setupUi(this);
}

KoFileDialogTester::~KoFileDialogTester()
{
    delete ui;
}
