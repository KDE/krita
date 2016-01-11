
/* This file is part of the KDE project
 * Copyright (C) 2011 Smit Patel <smitpatel24@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "BibliographyTemplate.h"

#include <BibliographyGenerator.h>
#include <KoBibliographyInfo.h>
#include <KoOdfBibliographyConfiguration.h>
#include <KoStyleManager.h>
#include <KoParagraphStyle.h>
#include <klocalizedstring.h>

BibliographyTemplate::BibliographyTemplate(KoStyleManager *manager)
    : m_manager(manager)
{
    Q_ASSERT(manager);
}

QList<KoBibliographyInfo *> BibliographyTemplate::templates()
{
    //if you are adding your own custom styles specifically for bibliography, add it as an unused style in KoStyleManager
    // when the bibliography is used the style will be automatically move to the usedStyle section

    QList<KoBibliographyInfo *> predefinedTemplates;
    KoBibliographyInfo *firstTemplate = new KoBibliographyInfo();
    firstTemplate->m_indexTitleTemplate.text = i18n("Bibliography");

    firstTemplate->m_indexTitleTemplate.styleId = m_manager->defaultBibliographyTitleStyle()->styleId();
    firstTemplate->m_indexTitleTemplate.styleName = m_manager->defaultBibliographyTitleStyle()->name();

    Q_FOREACH (const QString &bibType, KoOdfBibliographyConfiguration::bibTypes) {
        firstTemplate->m_entryTemplate[bibType].styleId = m_manager->defaultBibliographyEntryStyle(bibType)->styleId();
        firstTemplate->m_entryTemplate[bibType].styleName = m_manager->defaultBibliographyEntryStyle(bibType)->name();
    }
    firstTemplate->m_entryTemplate = BibliographyGenerator::defaultBibliographyEntryTemplates();

    KoBibliographyInfo *secondTemplate = new KoBibliographyInfo();
    secondTemplate->m_indexTitleTemplate.text = i18n("References");

    secondTemplate->m_indexTitleTemplate.styleId = m_manager->defaultBibliographyTitleStyle()->styleId();
    secondTemplate->m_indexTitleTemplate.styleName = m_manager->defaultBibliographyTitleStyle()->name();

    Q_FOREACH (const QString &bibType, KoOdfBibliographyConfiguration::bibTypes) {
        secondTemplate->m_entryTemplate[bibType].styleId = m_manager->defaultBibliographyEntryStyle(bibType)->styleId();
        secondTemplate->m_entryTemplate[bibType].styleName = m_manager->defaultBibliographyEntryStyle(bibType)->name();
    }
    secondTemplate->m_entryTemplate = BibliographyGenerator::defaultBibliographyEntryTemplates();

    predefinedTemplates.append(firstTemplate);
    predefinedTemplates.append(secondTemplate);
    return predefinedTemplates;
}

void BibliographyTemplate::moveTemplateToUsed(KoBibliographyInfo *info)
{
    if (m_manager->unusedStyle(info->m_indexTitleTemplate.styleId)) {
        m_manager->moveToUsedStyles(info->m_indexTitleTemplate.styleId);
    }

    Q_FOREACH (const QString &bibType, KoOdfBibliographyConfiguration::bibTypes) {
        if (m_manager->unusedStyle(info->m_entryTemplate[bibType].styleId)) {
            m_manager->moveToUsedStyles(info->m_entryTemplate[bibType].styleId);
        }
    }
}
