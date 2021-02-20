/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PALETTECOLORSMODEL_H
#define PALETTECOLORSMODEL_H

#include <QAbstractListModel>
#include <QColor>
#include <KoColorSet.h>

class PaletteColorsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QObject* colorSet READ colorSet WRITE setColorSet NOTIFY colorSetChanged)
    Q_PROPERTY(QObject* view READ view WRITE setView NOTIFY viewChanged)
public:
    enum PaletteColorsRoles {
        ImageRole = Qt::UserRole + 1,
        TextRole
    };

    explicit PaletteColorsModel(QObject *parent = 0);
    virtual ~PaletteColorsModel();
    QHash<int, QByteArray> roleNames() const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    QObject* view() const;
    void setView(QObject* newView);
    QObject* colorSet() const;
    void setColorSet(QObject* newColorSet);

Q_SIGNALS:
    void colorChanged(QColor newColor, bool backgroundChanged);
    void colorSetChanged();
    void viewChanged();

public Q_SLOTS:
    void activateColor(int index, bool setBackgroundColor);

private:
    class Private;
    Private* d;
};

#endif // PALETTECOLORSMODEL_H
