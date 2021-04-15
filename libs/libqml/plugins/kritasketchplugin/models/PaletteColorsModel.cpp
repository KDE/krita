/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "PaletteColorsModel.h"
#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>

#include <resources/KoColorSet.h>

class PaletteColorsModel::Private {
public:
    Private()
        : colorSet(0)
        , view(0)
    {}

    KoColorSetSP colorSet;
    KisViewManager* view;
};

PaletteColorsModel::PaletteColorsModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new Private)
{
}

PaletteColorsModel::~PaletteColorsModel()
{
    delete d;
}

QHash<int, QByteArray> PaletteColorsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ImageRole] = "image";
    roles[TextRole] = "text";

    return roles;
}

int PaletteColorsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    if (!d->colorSet)
        return 0;
    return d->colorSet->colorCount();
}

QVariant PaletteColorsModel::data(const QModelIndex &/*index*/, int /*role*/) const
{
    QVariant result;
    /*
    QColor color;
    if (index.isValid() && d->colorSet)
    {
        switch(role)
        {
        case ImageRole:
            color = d->colorSet->getColorGlobal(index.row(), index.column()).color().toQColor();
            result = QString("image://color/%1,%2,%3,%4").arg(color.redF()).arg(color.greenF()).arg(color.blueF()).arg(color.alphaF());
            break;
        case TextRole:
            result = d->colorSet->getColorGlobal(index.row(), index.column()).name();
            break;
        default:
            break;
        }
    }
    */

    return result;
}

QVariant PaletteColorsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);
    QVariant result;
    if (section == 0)
    {
        switch(role)
        {
        case ImageRole:
            result = QString("Thumbnail");
            break;
        case TextRole:
            result = QString("Name");
            break;
        default:
            break;
        }
    }
    return result;
}

void PaletteColorsModel::setColorSet(QObject */*newColorSet*/)
{
    // XXX SharedPtr We need to wrap KoColorSet

    //d->colorSet = qobject_cast<KoColorSet*>(newColorSet);
    beginResetModel();
    endResetModel();
    emit colorSetChanged();
}

QObject* PaletteColorsModel::colorSet() const
{
    // XXX SharedPtr We need to wrap KoColorSet
    // return d->colorSet;
    return 0;
}


QObject* PaletteColorsModel::view() const
{
    return d->view;
}

void PaletteColorsModel::setView(QObject* newView)
{
    d->view = qobject_cast<KisViewManager*>( newView );
    emit viewChanged();
}

void PaletteColorsModel::activateColor(int index, bool /*setBackgroundColor*/)
{
    if ( !d->view )
        return;

    if (index >= 0 && index < (int)d->colorSet->colorCount()) {
        /*
        if (setBackgroundColor)
            d->view->resourceProvider()->setBGColor(d->colorSet->getColorGlobal(index).color());
        else
            d->view->resourceProvider()->setFGColor( d->colorSet->getColorGlobal(index).color());
        emit colorChanged(d->colorSet->getColorGlobal(index).color().toQColor(), setBackgroundColor);
        */
    }
}

