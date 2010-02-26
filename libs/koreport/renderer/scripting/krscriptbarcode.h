/*
 * Kexi Report Plugin
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SCRIPTINGKRSCRIPTBARCODE_H
#define SCRIPTINGKRSCRIPTBARCODE_H

#include <QObject>
#include <krbarcodedata.h>

class KRBarcodeData;

namespace Scripting
{

/**
 @author Adam Pigg <adam@piggz.co.uk>
*/
class Barcode : public QObject
{
    Q_OBJECT
public:
    Barcode(KRBarcodeData *f);

    ~Barcode();

public slots:


    /**
     * Get the position of the barcode
     * @return position in points
     */
    QPointF position();


    /**
     * Sets the position of the barcode in points
     * @param Position
     */
    void setPosition(const QPointF&);

    /**
     * Get the size of the barcode
     * @return size in points
     */
    QSizeF size();

    /**
     * Set the size of the barcode in points
     * @param Size
     */
    void setSize(const QSizeF&);

    /**
     * Get the horizontal alignment
     * -1 Left
     * 0 Center
     * +1 Right
     * @return alignment
    */
    int horizontalAlignment();

    /**
     * Sets the horizontal alignment
     * @param  Alignemnt
     */
    void setHorizonalAlignment(int);


    /**
     * Get the control source (field name) of the barcode
     * @return control source
     */
    QString source();


    /**
     * Set the control source (field name) of the barcode
     * @param controlsource
     */
    void setSource(const QString&);


    /**
     * Get the barcode format
     * @return format as string
     */
    QString format();

    /**
     * Set the barcode format
     * @param format
     */
    void setFormat(const QString&);


private:
    KRBarcodeData *m_barcode;
};

}

#endif
