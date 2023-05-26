#include "wdg_side.h"
#include "ui_wdgside.h"

#include <QLabel>
#include <QVBoxLayout>

WdgSide::WdgSide(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::WdgSide)
{
    m_ui->setupUi(this);

    m_ui->lblChooseRes->setStyleSheet("font-weight: bold");

}

void WdgSide::focusLabel(int id)
{
    switch(id) {
    case 1: {
                m_ui->lblChooseRes->setStyleSheet("font-weight: bold");
                m_ui->lblChooseTags->setStyleSheet("font-weight: normal");
                m_ui->lblBundleInfo->setStyleSheet("font-weight: normal");
                m_ui->lblSaveLocation->setStyleSheet("font-weight: normal");
                break;
            }
    case 2: {
                m_ui->lblChooseRes->setStyleSheet("font-weight: normal");
                m_ui->lblChooseTags->setStyleSheet("font-weight: bold");
                m_ui->lblBundleInfo->setStyleSheet("font-weight: normal");
                m_ui->lblSaveLocation->setStyleSheet("font-weight: normal");
                break;
            }
    case 3: {
                m_ui->lblChooseRes->setStyleSheet("font-weight: normal");
                m_ui->lblChooseTags->setStyleSheet("font-weight: normal");
                m_ui->lblBundleInfo->setStyleSheet("font-weight: bold");
                m_ui->lblSaveLocation->setStyleSheet("font-weight: normal");
                break;
            }
    case 4: {
                m_ui->lblChooseRes->setStyleSheet("font-weight: normal");
                m_ui->lblChooseTags->setStyleSheet("font-weight: normal");
                m_ui->lblBundleInfo->setStyleSheet("font-weight: normal");
                m_ui->lblSaveLocation->setStyleSheet("font-weight: bold");
                break;
            }
    }

}

WdgSide::~WdgSide()
{
    delete m_ui;
}
