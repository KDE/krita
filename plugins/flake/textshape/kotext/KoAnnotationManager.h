/* This file is part of the KDE project
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2012 Inge Wallin <inge@lysator.liu.se>
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

#ifndef KOANNOTATIONMANAGER_H
#define KOANNOTATIONMANAGER_H

#include "kritatext_export.h"

#include <QObject>
#include <QList>

class KoAnnotation;
class KoAnnotationManagerPrivate;

/**
 * A manager for all annotations in a document. Every annotation is identified by a unique name.
 * Note that only SinglePosition and StartAnnotation annotations can be retrieved from this
 * manager. An end annotation should be retrieved from it's parent (StartAnnotation) using
 * KoAnnotation::endAnnotation()
 * This class also maintains a list of annotation names so that it can be easily used to
 * show all available annotation.
 */
class KRITATEXT_EXPORT KoAnnotationManager : public QObject
{
    Q_OBJECT
public:
    /// constructor
    KoAnnotationManager();
    ~KoAnnotationManager();

    /// @return an annotation with the specified name, or 0 if there is none
    KoAnnotation *annotation(const QString &name) const;

    /// @return a list of QString containing all annotation names
    QList<QString> annotationNameList() const;

public Q_SLOTS:
    /**
     * Insert a new annotation to this manager. The name of the annotation
     * will be set to @param name, no matter what name has been set on
     * it.
     * @param name the name of the annotation
     * @param annotation the annotation object to insert
     */
    void insert(const QString &name, KoAnnotation *annotation);

    /**
     * Remove an annotation from this manager.
     * @param name the name of the annotation to remove
     */
    void remove(const QString &name);

    /**
     * Rename an annotation
     * @param oldName the old name of the annotation
     * @param newName the new name of the annotation
     */
    void rename(const QString &oldName, const QString &newName);

private:
    KoAnnotationManagerPrivate * const d;
};

#endif
