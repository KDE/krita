/*
    Copyright (C) 2017 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef USERFEEDBACK_SELECTIONRATIOSOURCE_H
#define USERFEEDBACK_SELECTIONRATIOSOURCE_H

#include "userfeedbackcore_export.h"
#include "abstractdatasource.h"

QT_BEGIN_NAMESPACE
class QItemSelectionModel;
QT_END_NAMESPACE

namespace UserFeedback {

class SelectionRatioSourcePrivate;

/*! Records the time ratio a given entry is selected via a QItemSelectionModel.
 *
 *  An example use-case would be the usage ratio of a applications
 *  views/modes selected using a model-based view sidebar (such as
 *  used in e.g. Kontact).
 */
class USERFEEDBACKCORE_EXPORT SelectionRatioSource : public AbstractDataSource
{
public:
    /*! Create a new selection ratio data source.
     * @param selectionModel The selection to monitor.
     * @param sampleName This is the name of the database field this data source is
     * associated with.
     */
    explicit SelectionRatioSource(QItemSelectionModel *selectionModel, const QString &sampleName);

    /*! Determine which role to consider for the reported value.
     *  By default this is Qt::DisplayRole.
     */
    void setRole(int role);

    QString description() const override;
    /*! Set human-readable and translated description of the data provided by this source.
     *  @note This must be set before adding this source, sources without description are
     *  discarded.
     *  @param desc The description.
     */
    void setDescription(const QString &desc);

    QVariant data() override;
    void load(QSettings *settings) override;
    void store(QSettings *settings) override;
    void reset(QSettings *settings) override;

private:
    Q_DECLARE_PRIVATE(SelectionRatioSource)
};

}

#endif // USERFEEDBACK_SELECTIONRATIOSOURCE_H
