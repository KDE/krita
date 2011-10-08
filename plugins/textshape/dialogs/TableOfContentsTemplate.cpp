/* This file is part of the KDE project
 * Copyright (C) 2011 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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

#include "TableOfContentsTemplate.h"

#include <KoTableOfContentsGeneratorInfo.h>
#include <KoStyleManager.h>
#include <KoParagraphStyle.h>

TableOfContentsTemplate::TableOfContentsTemplate(KoStyleManager *manager):
    m_manager(manager)
{
    Q_ASSERT(manager);
}

QList<KoTableOfContentsGeneratorInfo *> TableOfContentsTemplate::templates()
{
    QList<KoTableOfContentsGeneratorInfo *> predefinedTemplates;
    KoTableOfContentsGeneratorInfo *firstTemplate = new KoTableOfContentsGeneratorInfo();
    firstTemplate->m_indexTitleTemplate.text = "Table Of Contents";

    firstTemplate->m_indexTitleTemplate.styleId = m_manager->defaultTableOfcontentsTitleStyle()->styleId();
    firstTemplate->m_indexTitleTemplate.styleName = m_manager->defaultTableOfcontentsTitleStyle()->name();

    for (int level=1; level <= firstTemplate->m_outlineLevel; ++level) {
        firstTemplate->m_entryTemplate[level - 1].styleId = m_manager->defaultTableOfContentsEntryStyle(level)->styleId();
        firstTemplate->m_entryTemplate[level - 1].styleName = m_manager->defaultTableOfContentsEntryStyle(level)->name();
    }

    KoTableOfContentsGeneratorInfo *secondTemplate = new KoTableOfContentsGeneratorInfo();
    secondTemplate->m_indexTitleTemplate.text = "Contents";

    secondTemplate->m_indexTitleTemplate.styleId = m_manager->defaultTableOfcontentsTitleStyle()->styleId();
    secondTemplate->m_indexTitleTemplate.styleName = m_manager->defaultTableOfcontentsTitleStyle()->name();

    for (int level = 1; level <= firstTemplate->m_outlineLevel; ++level) {
        secondTemplate->m_entryTemplate[level - 1].styleId = m_manager->defaultTableOfContentsEntryStyle(level)->styleId();
        secondTemplate->m_entryTemplate[level - 1].styleName = m_manager->defaultTableOfContentsEntryStyle(level)->name();
    }

    predefinedTemplates.append(firstTemplate);
    predefinedTemplates.append(secondTemplate);
    return predefinedTemplates;
}
