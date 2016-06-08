/*
 * This file is part of the KDE project
 * Copyright (C) 2010 Sebastian Sauer <sebsauer@kdab.com>
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

#ifndef AUTORESIZECOMMAND_H
#define AUTORESIZECOMMAND_H

#include <kundo2command.h>
#include <QPointer>
#include <TextTool.h>
#include <KoTextDocumentLayout.h>

class TextShape;

class AutoResizeCommand : public KUndo2Command
{
public:
    AutoResizeCommand(KoTextShapeData *shapeData, KoTextShapeData::ResizeMethod resizeMethod, bool enable);

    virtual void undo();
    virtual void redo();

private:
    KoTextShapeData *m_shapeData;
    KoTextShapeData::ResizeMethod m_resizeMethod;
    bool m_enabled;
    bool m_first;
    KoTextShapeData::ResizeMethod m_prevResizeMethod;
};

#endif // TEXTCUTCOMMAND_H
