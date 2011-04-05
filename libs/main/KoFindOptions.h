/* This file is part of the KDE project
 *
 * Copyright (c) 2010 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#ifndef KOFINDOPTIONS_H
#define KOFINDOPTIONS_H

#include <QtCore/QObject>

class KoFindOption;

/**
 * \brief A collection of search option that are supported by the backend.
 *
 * This class manages options for searching. Through this class, backends
 * can support a different set of options for searching. An instance of
 * this class can be retrieved from the backend by calling
 * KoFindBase::options(). The individual options can then be retrieved
 * from the KoFindOptions instance and used to populate the UI.
 */
class KoFindOptions : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor.
     *
     * Constructs an instance without any options.
     */
    explicit KoFindOptions(QObject* parent = 0);
    /**
     * Destructor.
     */
    virtual ~KoFindOptions();

    /**
     * Retrieve a specific option.
     *
     * \param id The id of the option to retrieve.
     *
     * \return The option corresponding to the id.
     */
    KoFindOption option(int id);
    /**
     * Retrieve a list of all properties.
     *
     * \return A list of options.
     */
    const QList< KoFindOption > options() const;

    /**
     * Add an empty option.
     *
     * This will add an option with no title, description or value set.
     * You should set these values yourself before using the option.
     *
     * \return The new option, which has a unique ID for this set of options.
     */
    KoFindOption addOption();
    /**
     * Add a new option.
     *
     * \param title The title of the option, for example "Case Sensitive".
     * \param description A description for the option, for example "Only generate a match if
     * the case of the possibly matched text matches that of the text to search for".
     * \param value The initial value of the option.
     *
     * \return The option just created.
     */
    KoFindOption addOption(const QString &title, const QString &description, const QVariant &value);

    /**
     * Remove an option from the set.
     *
     * \param id The ID of the option to remove.
     */
    void removeOption(int id);
    /**
     * Remove an option from the set.
     *
     * \param opt The option to remove.
     */
    void removeOption(const KoFindOption &remove);

public Q_SLOTS:
    /**
     * Set the value of an option.
     *
     * \param id The id of th option to set.
     * \param value The value to set the option to.
     */
    void setOptionValue(int id, const QVariant &value);
    /**
     * Set an option.
     *
     * This will replace the option corresponding to the id of the option
     * with the option passed.
     */
    void setOption(const KoFindOption &newOption);

private:
    class Private;
    Private * const d;
};

#endif // KOFINDOPTIONS_H
