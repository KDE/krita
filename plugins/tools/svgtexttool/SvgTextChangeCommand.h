/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef CHANGETEXTNGDATACOMMAND_H
#define CHANGETEXTNGDATACOMMAND_H

#include <kundo2command.h>
#include <QByteArray>

#include "KoSvgTextShape.h"

class SvgTextChangeCommand : public KUndo2Command
{
public:
    SvgTextChangeCommand(KoSvgTextShape *shape,
                         const QString &svg,
                         const QString &defs, bool richTextPreferred,
                         KUndo2Command *parent = 0);
    virtual ~SvgTextChangeCommand();

    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();

private:
    KoSvgTextShape *m_shape;
    QString m_svg;
    QString m_defs;
    QString m_oldSvg;
    QString m_oldDefs;
    bool m_oldRichTextPreferred = true;
    bool m_richTextPreferred = true;
};

#endif /* CHANGETEXTNGDATACOMMAND_H */
