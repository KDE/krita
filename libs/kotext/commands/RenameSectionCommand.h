/*
 * This file is part of the KDE project
 * Copyright (C) 2014-2015 Denis Kuplaykov <dener.kup@gmail.com>
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
 * Boston, MA 02110-1301, USA.*/

#ifndef RENAMESECTIONCOMMAND_H
#define RENAMESECTIONCOMMAND_H

#include <QString>

#include <kundo2qstack.h>

class QTextDocument;

class KoSection;
class KoSectionModel;

class RenameSectionCommand : public KUndo2Command
{
public:

    RenameSectionCommand(KoSection *section, const QString &newName, QTextDocument *document);
    virtual ~RenameSectionCommand();

    virtual void undo();
    virtual void redo();

    virtual bool mergeWith(const KUndo2Command *other);
    virtual int id() const;

private:
    KoSectionModel *m_sectionModel; ///< Pointer to document's KoSectionModel
    KoSection *m_section; ///< Section to rename
    QString m_newName; ///< New section name
    QString m_oldName; ///< Old section name (needed to undo)
    bool m_first; ///< Checks first call of redo
};

#endif // RENAMESECTIONCOMMAND_H
