/*  This file is part of the KDE project
    Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

 */
#ifndef KOCOLORSET
#define KOCOLORSET

#include <QObject>
#include <QColor>
#include <QVector>
#include <QScopedPointer>

#include <resources/KoResource.h>
#include "KoColor.h"

struct KoColorSetEntry {
    KoColorSetEntry() {}
    KoColorSetEntry(const KoColor &_color, const QString &_name)
        : color(_color), name(_name) {}

    KoColor color;
    QString name;
    QString id;
    bool spotColor {false};

    bool operator==(const KoColorSetEntry& rhs) const {
        return color == rhs.color && name == rhs.name;
    }
};

/**
 * Open Gimp, Photoshop or RIFF palette files. This is a straight port
 * from the Gimp.
 */
class KRITAPIGMENT_EXPORT KoColorSet : public QObject, public KoResource
{
    Q_OBJECT
public:

    enum PaletteType {
        UNKNOWN = 0,
        GPL,                // GIMP
        RIFF_PAL,           // RIFF
        ACT,                // Photoshop binary
        PSP_PAL,            // PaintShop Pro
        ACO,                // Photoshop Swatches
        XML                 // XML palette (Scribus)
    };


    /**
     * Load a color set from a file. This can be a Gimp
     * palette, a RIFF palette or a Photoshop palette.
     */
    explicit KoColorSet(const QString &filename);

    /// Create an empty color set
    KoColorSet();

    /// Explicit copy constructor (KoResource copy constructor is private)
    KoColorSet(const KoColorSet& rhs);

    virtual ~KoColorSet();

    virtual bool load();
    virtual bool loadFromDevice(QIODevice *dev);
    virtual bool save();
    virtual bool saveToDevice(QIODevice* dev) const;

    virtual QString defaultFileExtension() const;

    void setColumnCount(int columns);
    int columnCount();

public:

    void add(const KoColorSetEntry &);
    void remove(const KoColorSetEntry &);
    void removeAt(quint32 index);
    KoColorSetEntry getColor(quint32 index);
    qint32 nColors();
    void clear();

    /**
     * @brief getIndexClosestColor
     * function that matches the color to all colors in the colorset, and returns the index
     * of the closest match.
     * @param color the color you wish to compare.
     * @param useGivenColorSpace whether to use the color space of the color given
     * when the two colors' colorspaces don't match. Else it'll use the entry's colorspace.
     * @return returns the int of the closest match.
     */
    qint32 getIndexClosestColor(KoColor color, bool useGivenColorSpace = true);

    /**
     * @brief closestColorName
     * convenience function to get the name of the closest match.
     * @param color
     * @param useGivenColorSpace
     * @return
     */
    QString closestColorName(KoColor color, bool useGivenColorSpace = true);

private:


    bool init();

    bool loadGpl();
    bool loadAct();
    bool loadRiff();
    bool loadPsp();
    bool loadAco();
    bool loadXml();

    struct Private;
    const QScopedPointer<Private> d;

};
#endif // KOCOLORSET

