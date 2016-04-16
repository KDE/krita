#include "WdgTextEditor.h"
#include "ui_WdgTextEditor.h"

WdgTextEditor::WdgTextEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WdgTextEditor)
{
    ui->setupUi(this);
}

WdgTextEditor::~WdgTextEditor()
{
    delete ui;
}
