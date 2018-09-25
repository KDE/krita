/*
 This file is part of the KDE project
 * Copyright (C) 2009 Ganesh Paramasivam <ganesh@crystalfab.com>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2012 C. Boemann <cbo@boemann.dk>
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

#include "AddAnnotationCommand.h"

#include <KoAnnotation.h>
#include <KoShapeController.h>
#include <KoShapeControllerBase.h>
#include <KoTextDocument.h>

AddAnnotationCommand::AddAnnotationCommand(KoAnnotation *annotation, KUndo2Command *parent)
    : AddTextRangeCommand(annotation, parent)
    , m_annotation(annotation)
    , m_shape(0)
{
    setText(kundo2_noi18n("internal step"));
}

void AddAnnotationCommand::undo()
{
    AddTextRangeCommand::undo();
    KoShapeController *shapeController = KoTextDocument(m_annotation->document()).shapeController();
    m_shape = m_annotation->annotationShape();
    shapeController->documentBase()->removeShape(m_shape);
}

void AddAnnotationCommand::redo()
{
    AddTextRangeCommand::redo();

    KoShapeController *shapeController = KoTextDocument(m_annotation->document()).shapeController();
    shapeController->documentBase()->addShape(m_annotation->annotationShape());
 
    m_shape = 0;

    //it's a textrange so we need to ask for a layout so we know where it is
    m_annotation->document()->markContentsDirty(m_annotation->rangeStart(), 0);
}


AddAnnotationCommand::~AddAnnotationCommand()
{
    // We delete shape at KoShapeDeleteCommand.
    //delete m_annotation->annotationShape();
}
