/* This file is part of the KDE project
   Copyright 2008 Stefan Nikolaus stefan.nikolaus@kdemail.net

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KO_TABLE_INTERFACE
#define KO_TABLE_INTERFACE

#include <KoResourceManager.h>

#include <QHash>
#include <QtPlugin>
#include <QVariant>

class QAbstractItemModel;
class QString;

/**
 * Spreadsheet/Table related classes and functions.
 */
namespace KoTable
{

namespace Resource
{
enum
{
    SourceRangeManager = KoCanvasResource::KSpreadStart + 100,
    TargetRangeManager
};
}

/**
 * An extension for cell range models.
 */
class ModelExtension
{
public:
    virtual ~ModelExtension() {}

    /**
     * \return the current cell range address
     */
    virtual QString regionAddress() const = 0;
};



/**
 * An interface to a spreadsheet for data consumers.
 * Allows creation of a read only data model, that gets updated on value changes
 * and moves along with column/row/cell range insertions/removals.
 */
class SourceRangeManager
{
public:
    virtual ~SourceRangeManager() {}

    /**
     * Creates a read only model for the cell region described by \p regionName.
     * \p regionName has to contain the table name and has to be contiguous.
     * \return an extended model for the described region
     * \return \c 0 if the region name is invalid
     */
    virtual const QAbstractItemModel* createModel(const QString& regionName) = 0;

    /**
     * Removes \p model from the tracked regions.
     * The model gets deleted.
     * \return \c true if the removal was successful
     */
    virtual bool removeModel(const QAbstractItemModel* model) = 0;

    /**
     * \return \c true if the cell region is valid
     */
    virtual bool isCellRegionValid(const QString& regionName) const = 0;
};



/**
 * An interface to a spreadsheet for data providers.
 * Allows to insert data through a model into a specific cell region.
 */
class TargetRangeManager
{
public:
    /**
     * Parameters to be passed to insertData.
     */
    class Parameters
    {
    public:
        Parameters() : model(0), isSelection(false), onUpdateKeepStyle(false),
            onUpdateKeepSize(true), persistentData(true), orientation(Row), containsHeader(true),
            displayFilterButtons(false), refreshDelay(0), showSettingsDialog(true) {}

        /// cell range address, has to contain the table name and has to be contiguous
        QString regionName;
        /// model for the data to be inserted, does not get modified
        const QAbstractItemModel* model;
        /// indicates whether the data is a selection of records or a complete database
        bool isSelection;
        /// indicates whether the cell styles should move with the data
        bool onUpdateKeepStyle;
        /// indicates whether the cell range should be resized to fit the data's size or not
        bool onUpdateKeepSize;
        /// indicates whether the data needs to be saved or not
        bool persistentData;
        /// indicates whether the data should be treated as horizontally aligned
        enum {Column, Row} orientation;
        /// indicates whether the data contains header information
        bool containsHeader;
        /// indicates whether filter buttons should be shown in the first column/row
        bool displayFilterButtons;
        /// the update interval in seconds
        int refreshDelay;
        /// indicates whether a dialog should be shown before assigning the data
        bool showSettingsDialog;
        /// list of config widgets for altering the data provider's settings
        QList<QWidget*> configWidgets;
    };

    virtual ~TargetRangeManager() {}

    /**
     * Inserts data into the cell region described by \p parameters.
     * \return an extended model for the described region
     */
    virtual QAbstractItemModel* insertData(const Parameters& parameters) = 0;

    /**
     * Removes \p model from the list of data consuming cell regions.
     * The data providing model does not get deleted.
     * \return \c true if the removal was successful
     */
    virtual bool removeModel(const QAbstractItemModel* model) = 0;

    /**
     * \return \c true if the cell region is valid
     */
    virtual bool isCellRegionValid(const QString& regionName) const = 0;
};

} // namespace KoTable

Q_DECLARE_INTERFACE(KoTable::ModelExtension, "org.koffice.spreadsheet.ModelExtension:1.0")
Q_DECLARE_INTERFACE(KoTable::SourceRangeManager, "org.koffice.spreadsheet.SourceRangeManager:1.0")
Q_DECLARE_INTERFACE(KoTable::TargetRangeManager, "org.koffice.spreadsheet.TargetRangeManager:1.0")

#endif // KOFFICE_TABLE_INTERFACE
