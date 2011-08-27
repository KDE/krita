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
#ifndef KOINLINEBIBLIOGRAPHY_H
#define KOINLINEBIBLIOGRAPHY_H

#include "KoInlineObject.h"
#include "kotext_export.h"

#include "KoXmlReaderForward.h"

class KoShapeLoadingContext;
class KoChangeTracker;
class KoStyleManager;

class QTextFrame;
class QTextCursor;

/**
 * This object is an inline object, which means it is anchored in the text-flow and it can hold
 * bibliography information.
 */

class KOTEXT_EXPORT KoInlineBibliography : public KoInlineObject
{
public:
    enum Type {
        Bibliography
    };
    KoInlineBibliography();

    virtual ~KoInlineBibliography();
    Type type() const;        //return type of bibliography (always Bibliography). But useful when differentiating KoInlineObjects

    /**
     * Set the textframe where we will create our own textframe within
     * Our textframe is the one containing the real cite contents.
     * @param text the new text
     */
//    void setMotherFrame(QTextFrame *text);
};

#endif // KOINLINEBIBLIOGRAPHY_H
