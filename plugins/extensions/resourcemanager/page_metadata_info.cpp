#include "page_metadata_info.h"
#include "ui_pagemetadatainfo.h"

PageMetadataInfo::PageMetadataInfo(QWidget *parent) :
    QWizardPage(parent),
    m_ui(new Ui::PageMetadataInfo)
{
    m_ui->setupUi(this);
}

PageMetadataInfo::~PageMetadataInfo()
{
    delete m_ui;
}
