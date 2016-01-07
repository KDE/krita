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

#ifndef TABLEOFCONTENTSTEMPLATE_H
#define TABLEOFCONTENTSTEMPLATE_H

#include <QList>

class KoTableOfContentsGeneratorInfo;
class KoStyleManager;

class TableOfContentsTemplate
{
public:
    explicit TableOfContentsTemplate(KoStyleManager *manager);

    QList<KoTableOfContentsGeneratorInfo *> templates();

    /// this method moves the styles used in info ToC from unused styles list to used
    void moveTemplateToUsed(KoTableOfContentsGeneratorInfo *info);

private:
    KoStyleManager *m_manager;
};

#endif // TABLEOFCONTENTSTEMPLATE_H
