/*
 *  SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "wdg_side.h"
#include "ui_wdgside.h"
#include <kis_icon.h>
#include <QToolButton>
#include <QPalette>

#include <QLabel>
#include <QVBoxLayout>
#include <QApplication>

WdgSide::WdgSide(KoResourceBundleSP bundle, QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::WdgSide)
    , m_bundle(bundle)
{
    m_ui->setupUi(this);

    QCoreApplication* coreApp = QCoreApplication::instance();
    QApplication* app = qobject_cast<QApplication*>(coreApp);
    QPalette appPalette = app->palette();
    QColor brightColor = appPalette.color(QPalette::Highlight);
    QString styleSheet = QString("QToolButton { background-color: %1; color: %2; }")
                        .arg(brightColor.name())
                        .arg(brightColor.lightnessF() < 0.5 ? "#FFFFFF" : "#000000");

    m_ui->btnChooseRes->setIcon(KisIconUtils::loadIcon("document-edit"));
    m_ui->btnChooseRes->setIconSize(QSize(28, 28));
    m_ui->btnChooseRes->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    m_ui->btnChooseTags->setIcon(KisIconUtils::loadIcon("bookmarks"));
    m_ui->btnChooseTags->setIconSize(QSize(28, 28));
    m_ui->btnChooseTags->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    m_ui->btnBundleInfo->setIcon(KisIconUtils::loadIcon("configure"));
    m_ui->btnBundleInfo->setIconSize(QSize(28, 28));
    m_ui->btnBundleInfo->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    m_ui->btnSaveLocation->setIcon(KisIconUtils::loadIcon("folder"));
    m_ui->btnSaveLocation->setIconSize(QSize(28, 28));
    m_ui->btnSaveLocation->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    m_ui->btnChooseRes->setStyleSheet(styleSheet);
}

void WdgSide::focusLabel(int id)
{
    QCoreApplication* coreApp = QCoreApplication::instance();
    QApplication* app = qobject_cast<QApplication*>(coreApp);
    QPalette appPalette = app->palette();
    QColor brightColor = appPalette.color(QPalette::Midlight);
    QString styleSheetHighlight = QString("QToolButton { background-color: %1; color: %2; }")
                        .arg(brightColor.name())
                        .arg(brightColor.lightnessF() < 0.5 ? "#FFFFFF" : "#000000");

    QColor normalColor = appPalette.color(QPalette::Base);
    QString styleSheetNormal = QString("QToolButton { background-color: %1; color: %2; }")
                        .arg(normalColor.name())
                        .arg(normalColor.lightnessF() < 0.5 ? "#FFFFFF" : "#000000");


    switch(id) {
    case 1: {
                m_ui->btnChooseRes->setStyleSheet(styleSheetHighlight);
                m_ui->btnChooseTags->setStyleSheet(styleSheetNormal);
                m_ui->btnBundleInfo->setStyleSheet(styleSheetNormal);
                m_ui->btnSaveLocation->setStyleSheet(styleSheetNormal);
                break;
            }
    case 2: {
                m_ui->btnChooseRes->setStyleSheet(styleSheetNormal);
                m_ui->btnChooseTags->setStyleSheet(styleSheetHighlight);
                m_ui->btnBundleInfo->setStyleSheet(styleSheetNormal);
                m_ui->btnSaveLocation->setStyleSheet(styleSheetNormal);
                break;
            }
    case 3: {
                m_ui->btnChooseRes->setStyleSheet(styleSheetNormal);
                m_ui->btnChooseTags->setStyleSheet(styleSheetNormal);
                m_ui->btnBundleInfo->setStyleSheet(styleSheetHighlight);
                m_ui->btnSaveLocation->setStyleSheet(styleSheetNormal);
                break;
            }
    case 4: {
                m_ui->btnChooseRes->setStyleSheet(styleSheetNormal);
                m_ui->btnChooseTags->setStyleSheet(styleSheetNormal);
                m_ui->btnBundleInfo->setStyleSheet(styleSheetNormal);
                m_ui->btnSaveLocation->setStyleSheet(styleSheetHighlight);
                break;
            }
    }

}

WdgSide::~WdgSide()
{
    delete m_ui;
}