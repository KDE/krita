/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_shade_selector_lines_settings.h"

#include <QVBoxLayout>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include "kis_shade_selector_line_combo_box.h"

KisShadeSelectorLinesSettings::KisShadeSelectorLinesSettings(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout* l = new QVBoxLayout(this);
    l->setSpacing(0);
    l->setMargin(0);
}

QString KisShadeSelectorLinesSettings::toString() const
{
    QString result;
    for(int i=0; i<m_lineList.size(); i++) {
        result.append(m_lineList.at(i)->configuration());
        result.append(';');
    }

    return result;
}

void KisShadeSelectorLinesSettings::fromString(const QString &stri)
{
    QStringList strili = stri.split(';', QString::SkipEmptyParts);
    setLineCount(strili.size());
    for(int i=0; i<strili.size(); i++) {
        m_lineList.at(i)->setConfiguration(strili.at(i));
    }
}

void KisShadeSelectorLinesSettings::updateSettings()
{
    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");
    fromString(cfg.readEntry("minimalShadeSelectorLineConfig", "0|0.2|0|0"));

    for(int i=0; i<m_lineList.size(); i++) {
        m_lineList.at(i)->updateSettings();
    }
}

void KisShadeSelectorLinesSettings::setLineCount(int count)
{
    bool emitSignal = (m_lineList.size()!=count)?true:false;
    while(count-m_lineList.size() > 0) {
        m_lineList.append(new KisShadeSelectorLineComboBox(this));
        m_lineList.last()->setLineNumber(m_lineList.size()-1);
        layout()->addWidget(m_lineList.last());
    }
    while(count-m_lineList.size() < 0) {
        layout()->removeWidget(m_lineList.last());
        delete m_lineList.takeLast();
    }

    for(int i=0; i<m_lineList.size(); i++) {
        connect(this, SIGNAL(setGradient(bool)),  m_lineList.at(i), SLOT(setGradient(bool)),  Qt::UniqueConnection);
        connect(this, SIGNAL(setPatches(bool)),   m_lineList.at(i), SLOT(setPatches(bool)),   Qt::UniqueConnection);
        connect(this, SIGNAL(setLineHeight(int)), m_lineList.at(i), SLOT(setLineHeight(int)), Qt::UniqueConnection);
        connect(this, SIGNAL(setPatchCount(int)), m_lineList.at(i), SLOT(setPatchCount(int)), Qt::UniqueConnection);
    }

    if(emitSignal)
        emit lineCountChanged(count);
}
