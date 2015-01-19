/*
 *  Copyright (c) 2014 Denis Kuplyakov <dener.kup@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KOSECTIONMANAGER_H
#define KOSECTIONMANAGER_H

#include <QTextDocument>
#include <QMetaType>
#include <QStandardItemModel>

#include <kotext_export.h>

class KoSection;

class KoSectionManagerPrivate
{
public:
    explicit KoSectionManagerPrivate(QTextDocument *_doc);
    ~KoSectionManagerPrivate();

    QTextDocument *doc;
    bool valid; //< is current section info is valid
    QHash<QString, KoSection *> sectionNames; //< stores name -> pointer reference
    int sectionCount; //< how many sections is registered
    QScopedPointer<QStandardItemModel> model;
};
/**
 * Used to handle all the sections in the document
 *
 * Sections are registered in KoSection constructor and unregistered
 * in KoSection destructor.
 *
 * Info is invalidated in DeleteCommand, because we can't check it from here.
 * Registering and unregistering section also calls invalidate().
 *
 * This object is created for QTextDocument on the first query of it.
 */
class KOTEXT_EXPORT KoSectionManager
{
public:
    explicit KoSectionManager(QTextDocument* doc);

    /**
     * Returns pointer to the deepest KoSection that covers @p pos
     * or NULL if there is no such section
     */
    KoSection *sectionAtPosition(int pos);

    /**
     * Returns name for the new section
     */
    QString possibleNewName() const;

    /**
     * Returns if this name is possible.
     */
    bool isValidNewName(const QString &name) const;

    /**
     * Returns tree model of sections to use in views
     */
    QStandardItemModel *sectionsModel();

public slots:
    /**
     * Call this to recalc all sections information
     */
    void update();

    /**
     * Call this to notify manager that info in it is invalidated.
     */
    void invalidate();

    /**
     * Call this to notify that some section changed its name
     */
    void sectionRenamed(const QString &oldName, const QString &name);

    /**
     * Call this to register new section in manager
     */
    void registerSection(KoSection *section);

    /**
     * Call this to unregister section from manager
     */
    void unregisterSection(KoSection *section);

protected:
    const QScopedPointer<KoSectionManagerPrivate> d_ptr;

private:
    Q_DISABLE_COPY(KoSectionManager)
    Q_DECLARE_PRIVATE(KoSectionManager)
};

Q_DECLARE_METATYPE(KoSectionManager *)

#endif //KOSECTIONMANAGER_H
