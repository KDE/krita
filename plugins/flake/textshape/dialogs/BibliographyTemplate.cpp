
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
