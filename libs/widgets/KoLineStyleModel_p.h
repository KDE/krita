/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KOLINESTYLEMODEL_H
#define KOLINESTYLEMODEL_H

#include <QAbstractListModel>

#include <QVector>

/// The line style model managing the style data
class KoLineStyleModel : public QAbstractListModel
{
public:
    explicit KoLineStyleModel(QObject *parent = 0);
    ~KoLineStyleModel() override {}
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /// adds the given style to the model
    bool addCustomStyle(const QVector<qreal> &style);
    /// selects the given style
    int setLineStyle(Qt::PenStyle style, const QVector<qreal> &dashes);
private:
    QList<QVector<qreal> > m_styles; ///< the added styles
    QVector<qreal> m_tempStyle; ///< a temporary added style
    bool m_hasTempStyle;        ///< state of the temporary style
};

#endif
