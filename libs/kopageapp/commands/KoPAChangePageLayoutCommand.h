/* This file is part of the KDE project
 * Copyright (C) 2009 Fredy Yanardi <fyanardi@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or ( at your option ) any later version.
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

#ifndef KOPACHANGEPAGELAYOUTCOMMAND_H
#define KOPACHANGEPAGELAYOUTCOMMAND_H

#include <QUndoCommand>
#include "KoPageLayout.h"

class KoPADocument;
class KoPAMasterPage;
class KoPAPageBase;

/**
 * Command to change the master page of a page
 */
class KoPAChangePageLayoutCommand : public QUndoCommand
{
public:
    KoPAChangePageLayoutCommand( KoPADocument *document, KoPAMasterPage *page, const KoPageLayout &newPageLayout, bool applyToDocument, QUndoCommand *parent = 0 );
    virtual ~KoPAChangePageLayoutCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    KoPADocument *m_document;
    KoPageLayout m_newPageLayout;
    QMap<KoPAMasterPage *, KoPageLayout> m_oldLayouts;
};

#endif /* KOPACHANGEPAGELAYOUTCOMMAND_H */

