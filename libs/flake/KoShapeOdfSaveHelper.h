/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOSHAPEODFSAVEHELPER_H
#define KOSHAPEODFSAVEHELPER_H

#include "KoDragOdfSaveHelper.h"
#include "flake_export.h"

class KoShape;
class KoShapeOdfSaveHelperPrivate;

/**
 * Save helper for saving shapes to odf.
 *
 * The shapes are saved in an office:text document.
 */
class FLAKE_EXPORT KoShapeOdfSaveHelper : public KoDragOdfSaveHelper
{
public:
    /**
     * Constructor
     *
     * @param shapes The list of shapes to save. If the shapes contain
     *               children these are also saved.
     */
    KoShapeOdfSaveHelper(QList<KoShape *> shapes);

    /// reimplemented
    virtual bool writeBody();

private:
    Q_DECLARE_PRIVATE(KoShapeOdfSaveHelper)
};

#endif /* KOSHAPEODFSAVEHELPER_H */
