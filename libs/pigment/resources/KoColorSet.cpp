/*  This file is part of the KDE project
   Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
   Copyright (c) 2016 L. E. Segovia <leo.segovia@siggraph.org>


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
#include <resources/KoColorSet.h>

#include <sys/types.h>

#include <QFile>
#include <QFileInfo>
#include <QBuffer>
#include <QVector>
#include <QTextStream>
#include <QTextCodec>
#include <QHash>
#include <QList>
#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>
#include <QString>
#include <QStringList>
#include <QImage>
#include <QPainter>
#include <QByteArray>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QtEndian> // qFromLittleEndian
#include <QStringList>

#include <DebugPigment.h>
#include <klocalizedstring.h>

#include <KoStore.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoColorProfile.h>
#include <KoColorModelStandardIds.h>
#include "KisSwatch.h"

#include "KoColorSet.h"
#include "KoColorSet_p.h"


KoColorSet::KoColorSet(const QString& filename)
    : KoResource(filename)
    , d(new Private(this))
{
    if (!filename.isEmpty()) {
        setValid(true);
    }
}

/// Create an copied palette
KoColorSet::KoColorSet(const KoColorSet& rhs)
    : QObject(0)
    , KoResource(rhs)
    , d(new Private(this))
{
    d->comment = rhs.d->comment;
    d->groupNames = rhs.d->groupNames;
    d->groups = rhs.d->groups;
}

KoColorSet::~KoColorSet()
{ }

bool KoColorSet::load()
{
    QFile file(filename());
    if (file.size() == 0) return false;
    if (!file.open(QIODevice::ReadOnly)) {
        warnPigment << "Can't open file " << filename();
        return false;
    }
    bool res = loadFromDevice(&file);
    file.close();
    return res;
}

bool KoColorSet::loadFromDevice(QIODevice *dev)
{
    if (!dev->isOpen()) dev->open(QIODevice::ReadOnly);

    d->data = dev->readAll();

    Q_ASSERT(d->data.size() != 0);

    return d->init();
}


bool KoColorSet::save()
{
    if (d->isGlobal) {
        // save to resource dir
        /*
        QFile file(filename());
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            return false;
        }
        saveToDevice(&file);
        file.close();
        */
        return true;
    }
    return true; // is not global
}

bool KoColorSet::saveToDevice(QIODevice *dev) const
{
    bool res;
    switch(d->paletteType) {
    case GPL:
        res = d->saveGpl(dev);
        break;
    default:
        res = d->saveKpl(dev);
    }
    if (res) {
        KoResource::saveToDevice(dev);
    }
    return res;
}

QByteArray KoColorSet::toByteArray() const
{
    QBuffer s;
    s.open(QIODevice::WriteOnly);
    if (!saveToDevice(&s)) {
        warnPigment << "saving palette failed:" << name();
        return QByteArray();
    }
    s.close();
    s.open(QIODevice::ReadOnly);
    QByteArray res = s.readAll();
    s.close();
    return res;
}

bool KoColorSet::fromByteArray(QByteArray &data)
{
    QBuffer buf(&data);
    buf.open(QIODevice::ReadOnly);
    return loadFromDevice(&buf);
}

KoColorSet::PaletteType KoColorSet::paletteType() const
{
    return d->paletteType;
}

void KoColorSet::setPaletteType(PaletteType paletteType)
{
    d->paletteType = paletteType;
}


quint32 KoColorSet::colorCount() const
{
    return d->groups[Private::GLOBAL_GROUP_NAME].colorCount();
}

QString KoColorSet::closestColorName(const KoColor color, bool useGivenColorSpace)
{
    quint8 highestPercentage = 0;
    quint8 testPercentage = 0;
    KoColor compare = color;
    QString res;
    for (int x = 0; x < d->groups[Private::GLOBAL_GROUP_NAME].columnCount(); x++) {
        for (int y = 0; y < rowCount(); y++ ) {
            KisSwatch entry = getColorGlobal(x, y);
            KoColor color = entry.color();
            if (useGivenColorSpace == true && compare.colorSpace() != color.colorSpace()) {
                color.convertTo(compare.colorSpace());

            } else if (compare.colorSpace() != color.colorSpace()) {
                compare.convertTo(color.colorSpace());
            }
            testPercentage = (255 - compare.colorSpace()->difference(compare.data(), color.data()));
            if (testPercentage > highestPercentage)
            {
                res = entry.name();
                highestPercentage = testPercentage;
            }
        }
    }
    return res;
}

void KoColorSet::add(const KisSwatch &c, const QString &groupName)
{
    KisSwatchGroup &modifiedGroup = d->groups.contains(groupName)
            ? d->groups[groupName] : d->global();
    modifiedGroup.addEntry(c);
}

void KoColorSet::setEntry(const KisSwatch &e, int x, int y, const QString &groupName)
{
    KisSwatchGroup &modifiedGroup = d->groups.contains(groupName)
            ? d->groups[groupName] : d->global();
    modifiedGroup.setEntry(e, x, y);
}

void KoColorSet::clear()
{
    d->groups.clear();
    d->groupNames.clear();
}

KisSwatch KoColorSet::getColorGlobal(quint32 x, quint32 y) const
{
    KisSwatch e;
    int yInGroup = y;
    QString nameGroupFoundIn;
    for (const QString &groupName : d->groupNames) {
        if (yInGroup < d->groups[groupName].rowCount()) {
            nameGroupFoundIn = groupName;
            break;
        } else {
            yInGroup -= d->groups[groupName].rowCount();
        }
    }
    const KisSwatchGroup &groupFoundIn = nameGroupFoundIn == QString()
            ? d->global() : d->groups[nameGroupFoundIn];
    Q_ASSERT(groupFoundIn.checkEntry(x, yInGroup));
    return groupFoundIn.getEntry(x, yInGroup);
}

KisSwatch KoColorSet::getColorGroup(quint32 x, quint32 y, QString groupName)
{
    KisSwatch e;
    const KisSwatchGroup &sourceGroup = groupName == QString()
            ? d->global() : d->groups[groupName];
    if (sourceGroup.checkEntry(x, y)) {
        e = sourceGroup.getEntry(x, y);
    }
    return e;
}

QStringList KoColorSet::getGroupNames()
{
    if (d->groupNames.size() != d->groups.size()) {
        warnPigment << "mismatch between groups and the groupnames list.";
        return QStringList(d->groups.keys());
    }
    return d->groupNames;
}

bool KoColorSet::changeGroupName(QString oldGroupName, QString newGroupName)
{
    if (d->groupNames.contains(oldGroupName)==false) {
        return false;
    }
    KisSwatchGroup tmp = d->groups.value(oldGroupName);
    d->groups.remove(oldGroupName);
    d->groups[newGroupName] = tmp;
    //rename the string in the stringlist;
    int index = d->groupNames.indexOf(oldGroupName);
    d->groupNames.replace(index, newGroupName);
    return true;
}

void KoColorSet::setColumnCount(int columns)
{
    d->groups[Private::GLOBAL_GROUP_NAME].setColumnCount(columns);
    for (KisSwatchGroup &g : d->groups.values()) { // Q_FOREACH doesn't accept non const refs
        g.setColumnCount(columns);
    }
}

int KoColorSet::columnCount() const
{
    return d->groups[Private::GLOBAL_GROUP_NAME].columnCount();
}

QString KoColorSet::comment()
{
    return d->comment;
}

void KoColorSet::setComment(QString comment)
{
    d->comment = comment;
}

bool KoColorSet::addGroup(const QString &groupName)
{
    if (d->groups.contains(groupName) || d->groupNames.contains(groupName)) {
        return false;
    }
    d->groupNames.append(groupName);
    d->groups[groupName] = KisSwatchGroup();
    return true;
}

bool KoColorSet::moveGroup(const QString &groupName, const QString &groupNameInsertBefore)
{
    if (d->groupNames.contains(groupName)==false || d->groupNames.contains(groupNameInsertBefore)==false) {
        return false;
    }
    d->groupNames.removeAt(d->groupNames.indexOf(groupName));
    int index = d->groupNames.size();
    if (groupNameInsertBefore!=QString()) {
        index = d->groupNames.indexOf(groupNameInsertBefore);
    }
    d->groupNames.insert(index, groupName);
    return true;
}

bool KoColorSet::removeGroup(const QString &groupName, bool keepColors)
{
    if (!d->groups.contains(groupName)) {
        return false;
    }

    if (keepColors) {
        // put all colors directly below global
        int startingRow = d->groups[Private::GLOBAL_GROUP_NAME].rowCount();
        for (int x = 0; x < d->groups[groupName].columnCount(); x++) {
            for (int y = 0; y < d->groups[groupName].rowCount(); y++) {
                if (d->groups[groupName].checkEntry(x, y)) {
                    d->groups[Private::GLOBAL_GROUP_NAME].setEntry(d->groups[groupName].getEntry(x, y),
                                       x,
                                       y + startingRow);
                }
            }
        }
    }

    for (int n = 0; n<d->groupNames.size(); n++) {
        if (d->groupNames.at(n) == groupName) {
            d->groupNames.removeAt(n);
        }
    }

    d->groups.remove(groupName);
    return true;
}

QString KoColorSet::defaultFileExtension() const
{
    return QString(".kpl");
}


int KoColorSet::rowCount() const
{
    int res = 0;
    for (const QString &name : d->groupNames) {
        res += d->groups[name].rowCount();
    }
    return res;
}

KisSwatchGroup *KoColorSet::getGroup(const QString &name)
{
    if (name.isEmpty()) {
        return &(d->global());
    }
    if (!d->groups.contains(name)) {
        return Q_NULLPTR;
    }
    return &(d->groups[name]);
}

bool KoColorSet::isGlobal() const
{
    return d->isGlobal;
}

void KoColorSet::setIsGlobal(bool isGlobal)
{
    d->isGlobal = isGlobal;
}

/*
QString KoColorSet::findGroupByGlobalIndex(quint32 x, quint32 y)
{
    *index = globalIndex;
    QString groupName = QString();
    if ((quint32)d->colors.size()<=*index) {
        *index -= (quint32)d->colors.size();
        if (!d->groups.empty() || !d->groupNames.empty()) {
            QStringList groupNames = getGroupNames();
            Q_FOREACH (QString name, groupNames) {
                quint32 size = (quint32)d->groups.value(name).size();
                if (size<=*index) {
                    *index -= size;
                } else {
                    groupName = name;
                    return groupName;
                }
            }

        }
    }
    return groupName;
}

QString KoColorSet::findGroupByColorName(const QString &name, quint32 *x, quint32 *y)
{
    QString groupName = QString();
    Q_FOREACH (const KisSwatchGroup::Column & col, d->global.colors()) {
        Q_FOREACH (const KisSwatch & entry, col.values()) {
            if (entry.name() == name) {
                *x = static_cast<quint32>(entry.x());
                *y = static_cast<quint32>(entry.y());
                return groupName;
            }
        }
    }
    QStringList groupNames = getGroupNames();
    Q_FOREACH (QString name, groupNames) {
        Q_FOREACH (const KisSwatchGroup::Column & col, d->groups[name].colors()) {
            Q_FOREACH (const KisSwatch & entry, col.values()) {
                if (entry.name() == name) {
                    *x = static_cast<quint32>(entry.x());
                    *y = static_cast<quint32>(entry.y());
                    return groupName;
                }
            }
        }
    }
    return groupName;
}

QString KoColorSet::findGroupByID(const QString &id, quint32 *index) {
    *index = 0;
    QString groupName = QString();
    for (int i = 0; i<d->colors.size(); i++) {
        if(d->colors.at(i).id() == id) {
            *index = (quint32)i;
            return groupName;
        }
    }
    QStringList groupNames = getGroupNames();
    Q_FOREACH (QString name, groupNames) {
        for (int i=0; i<d->groups[name].size(); i++) {
            if(d->groups[name].at(i).id() == id) {
                *index = (quint32)i;
                groupName = name;
                return groupName;
            }
        }
    }
    return groupName;
    return QString();
}

const KisSwatchGroup &KoColorSet::getGroupByName(const QString &groupName, bool &success) const
{
    if (groupName.isEmpty()) {
        success = true;
        return d->global;
    }
    if (d->groupNames.contains(groupName)) {
        success = true;
        return d->groups[groupName];
    }
    success = false;
    return d->global;
}

bool KoColorSet::changeColorSetEntry(KisSwatch entry,
                                     QString groupName)
{
    if (!d->groupNames.contains(groupName)) {
        return false;
    }
    if (groupName.isEmpty()) {
        d->global.setEntry(entry);
    } else {
        d->groups[groupName].setEntry(entry);
    }
    return true;
}

quint32 KoColorSet::getIndexClosestColor(const KoColor color, bool useGivenColorSpace)
{
    quint32 closestIndex = 0;
    quint8 highestPercentage = 0;
    quint8 testPercentage = 0;
    KoColor compare = color;
    for (quint32 i=0; i < colorCount(); i++) {
        KoColor entry = getColorGlobal(i).color();
        if (useGivenColorSpace == true && compare.colorSpace() != entry.colorSpace()) {
            entry.convertTo(compare.colorSpace());

        } else if(compare.colorSpace()!=entry.colorSpace()) {
            compare.convertTo(entry.colorSpace());
        }
        testPercentage = (255 - compare.colorSpace()->difference(compare.data(), entry.data()));
        if (testPercentage>highestPercentage)
        {
            closestIndex = i;
            highestPercentage = testPercentage;
        }
    }
    return closestIndex;
}

void KoColorSet::removeAt(quint32 x, quint32 y, QString groupName)
{
    if (d->groups.contains(groupName)){
        if ((quint32)d->groups.value(groupName).size()>x) {
            d->groups[groupName].remove(x);
        }
    } else {
        if ((quint32)d->colors.size()>x) {
            d->colors.remove(x);
        }
    }
    if (x >= d->columns || x < 0)
        return;
    if (d->groups.contains(groupName)){
            d->groups[groupName].removeEntry(x, y);
    } else {
            d->global.removeEntry(x, y);
    }
}

quint32 KoColorSet::insertBefore(const KisSwatch &c, qint32 index, const QString &groupName)
{
    quint32 newIndex = index;
    if (d->groups.contains(groupName)) {
        d->groups[groupName].insert(index, c);
    } else if (groupName.isEmpty()){
        d->colors.insert(index, c);
    } else {
        warnPigment << "Couldn't find group to insert to";
    }
    return newIndex;
    return 0;
}
*/

/********************************KoColorSet::Private**************************/

QString KoColorSet::Private::GLOBAL_GROUP_NAME = QString();

KoColorSet::PaletteType KoColorSet::Private::detectFormat(const QString &fileName, const QByteArray &ba)
{
    QFileInfo fi(fileName);

    // .pal
    if (ba.startsWith("RIFF") && ba.indexOf("PAL data", 8)) {
        return KoColorSet::RIFF_PAL;
    }
    // .gpl
    else if (ba.startsWith("GIMP Palette")) {
        return KoColorSet::GPL;
    }
    // .pal
    else if (ba.startsWith("JASC-PAL")) {
        return KoColorSet::PSP_PAL;
    }
    else if (fi.suffix().toLower() == "aco") {
        return KoColorSet::ACO;
    }
    else if (fi.suffix().toLower() == "act") {
        return KoColorSet::ACT;
    }
    else if (fi.suffix().toLower() == "xml") {
        return KoColorSet::XML;
    }
    else if (fi.suffix().toLower() == "kpl") {
        return KoColorSet::KPL;
    }
    else if (fi.suffix().toLower() == "sbz") {
        return KoColorSet::SBZ;
    }
    return KoColorSet::UNKNOWN;
}

void KoColorSet::Private::scribusParseColor(KoColorSet *set, QXmlStreamReader *xml)
{
    KisSwatch colorEntry;
    // It's a color, retrieve it
    QXmlStreamAttributes colorProperties = xml->attributes();

    QStringRef colorName = colorProperties.value("NAME");
    colorEntry.setName(colorName.isEmpty() || colorName.isNull() ? i18n("Untitled") : colorName.toString());

    // RGB or CMYK?
    if (colorProperties.hasAttribute("RGB")) {
        dbgPigment << "Color " << colorProperties.value("NAME") << ", RGB " << colorProperties.value("RGB");

        KoColor currentColor(KoColorSpaceRegistry::instance()->rgb8());
        QStringRef colorValue = colorProperties.value("RGB");

        if (colorValue.length() != 7 && colorValue.at(0) != '#') { // Color is a hexadecimal number
            xml->raiseError("Invalid rgb8 color (malformed): " + colorValue);
            return;
        } else {
            bool rgbOk;
            quint32 rgb = colorValue.mid(1).toUInt(&rgbOk, 16);
            if  (!rgbOk) {
                xml->raiseError("Invalid rgb8 color (unable to convert): " + colorValue);
                return;
            }

            quint8 r = rgb >> 16 & 0xff;
            quint8 g = rgb >> 8 & 0xff;
            quint8 b = rgb & 0xff;

            dbgPigment << "Color parsed: "<< r << g << b;

            currentColor.data()[0] = r;
            currentColor.data()[1] = g;
            currentColor.data()[2] = b;
            currentColor.setOpacity(OPACITY_OPAQUE_U8);
            colorEntry.setColor(currentColor);

            set->add(colorEntry);

            while(xml->readNextStartElement()) {
                //ignore - these are all unknown or the /> element tag
                xml->skipCurrentElement();
            }
            return;
        }
    }
    else if (colorProperties.hasAttribute("CMYK")) {
        dbgPigment << "Color " << colorProperties.value("NAME") << ", CMYK " << colorProperties.value("CMYK");

        KoColor currentColor(KoColorSpaceRegistry::instance()->colorSpace(CMYKAColorModelID.id(), Integer8BitsColorDepthID.id(), QString()));

        QStringRef colorValue = colorProperties.value("CMYK");
        if (colorValue.length() != 9 && colorValue.at(0) != '#') { // Color is a hexadecimal number
            xml->raiseError("Invalid cmyk color (malformed): " % colorValue);
            return;
        }
        else {
            bool cmykOk;
            quint32 cmyk = colorValue.mid(1).toUInt(&cmykOk, 16); // cmyk uses the full 32 bits
            if  (!cmykOk) {
                xml->raiseError("Invalid cmyk color (unable to convert): " % colorValue);
                return;
            }

            quint8 c = cmyk >> 24 & 0xff;
            quint8 m = cmyk >> 16 & 0xff;
            quint8 y = cmyk >> 8 & 0xff;
            quint8 k = cmyk & 0xff;

            dbgPigment << "Color parsed: "<< c << m << y << k;

            currentColor.data()[0] = c;
            currentColor.data()[1] = m;
            currentColor.data()[2] = y;
            currentColor.data()[3] = k;
            currentColor.setOpacity(OPACITY_OPAQUE_U8);
            colorEntry.setColor(currentColor);

            set->add(colorEntry);

            while(xml->readNextStartElement()) {
                //ignore - these are all unknown or the /> element tag
                xml->skipCurrentElement();
            }
            return;
        }
    }
    else {
        xml->raiseError("Unknown color space for color " + colorEntry.name());
    }
}

bool KoColorSet::Private::loadScribusXmlPalette(KoColorSet *set, QXmlStreamReader *xml)
{

    //1. Get name
    QXmlStreamAttributes paletteProperties = xml->attributes();
    QStringRef paletteName = paletteProperties.value("Name");
    dbgPigment << "Processed name of palette:" << paletteName;
    set->setName(paletteName.toString());

    //2. Inside the SCRIBUSCOLORS, there are lots of colors. Retrieve them

    while(xml->readNextStartElement()) {
        QStringRef currentElement = xml->name();
        if(QStringRef::compare(currentElement, "COLOR", Qt::CaseInsensitive) == 0) {
            scribusParseColor(set, xml);
        }
        else {
            xml->skipCurrentElement();
        }
    }

    if(xml->hasError()) {
        return false;
    }

    return true;
}

quint16 readShort(QIODevice *io) {
    quint16 val;
    quint64 read = io->read((char*)&val, 2);
    if (read != 2) return false;
    return qFromBigEndian(val);
}


KoColorSet::Private::Private(KoColorSet *a_colorSet)
    : colorSet(a_colorSet)
    , isGlobal(true)
{
    groups[GLOBAL_GROUP_NAME] = KisSwatchGroup();
    groupNames.append(GLOBAL_GROUP_NAME);
}

bool KoColorSet::Private::init()
{
    // just in case this is a reload (eg by KoEditColorSetDialog),
    groupNames.clear();
    groups.clear();
    groupNames.append(GLOBAL_GROUP_NAME);
    groups[GLOBAL_GROUP_NAME] = KisSwatchGroup();

    if (colorSet->filename().isNull()) {
        warnPigment << "Cannot load palette" << colorSet->name() << "there is no filename set";
        return false;
    }
    if (data.isNull()) {
        QFile file(colorSet->filename());
        if (file.size() == 0) {
            warnPigment << "Cannot load palette" << colorSet->name() << "there is no data available";
            return false;
        }
        file.open(QIODevice::ReadOnly);
        data = file.readAll();
        file.close();
    }

    bool res = false;
    paletteType = detectFormat(colorSet->filename(), data);
    switch(paletteType) {
    case GPL:
        res = loadGpl();
        break;
    case ACT:
        res = loadAct();
        break;
    case RIFF_PAL:
        res = loadRiff();
        break;
    case PSP_PAL:
        res = loadPsp();
        break;
    case ACO:
        res = loadAco();
        break;
    case XML:
        res = loadXml();
        break;
    case KPL:
        res = loadKpl();
        break;
    case SBZ:
        res = loadSbz();
        break;
    default:
        res = false;
    }
    colorSet->setValid(res);

    QImage img(global().columnCount() * 4, global().rowCount() * 4, QImage::Format_ARGB32);
    QPainter gc(&img);
    gc.fillRect(img.rect(), Qt::darkGray);
    for (const KisSwatchGroup::SwatchInfo &info : global().infoList()) {
        QColor c = info.swatch.color().toQColor();
        gc.fillRect(info.column * 4, info.row * 4, 4, 4, c);
    }
    colorSet->setImage(img);
    colorSet->setValid(res);

    // save some memory
    data.clear();
    return res;
}

bool KoColorSet::Private::saveGpl(QIODevice *dev) const
{
    Q_ASSERT(dev->isOpen());
    Q_ASSERT(dev->isWritable());

    QTextStream stream(dev);
    stream << "GIMP Palette\nName: " << colorSet->name() << "\nColumns: " << colorSet->columnCount() << "\n#\n";

    /*
     * Qt doesn't provide an interface to get a const reference to a QHash, that is
     * the underlying data structure of groups. Therefore, directly use
     * groups[GLOBAL_GROUP_NAME] so that saveGpl can stay const
     */

    for (int y = 0; y < groups[GLOBAL_GROUP_NAME].rowCount(); y++) {
        for (int x = 0; x < colorSet->columnCount(); x++) {
            if (!groups[GLOBAL_GROUP_NAME].checkEntry(x, y)) {
                continue;
            }
            const KisSwatch& entry = groups[GLOBAL_GROUP_NAME].getEntry(x, y);
            QColor c = entry.color().toQColor();
            stream << c.red() << " " << c.green() << " " << c.blue() << "\t";
            if (entry.name().isEmpty())
                stream << "Untitled\n";
            else
                stream << entry.name() << "\n";
        }
    }

    return true;
}

bool KoColorSet::Private::loadGpl()
{
    QString s = QString::fromUtf8(data.data(), data.count());

    if (s.isEmpty() || s.isNull() || s.length() < 50) {
        warnPigment << "Illegal Gimp palette file: " << colorSet->filename();
        return false;
    }

    quint32 index = 0;

    QStringList lines = s.split('\n', QString::SkipEmptyParts);

    if (lines.size() < 3) {
        warnPigment << "Not enough lines in palette file: " << colorSet->filename();
        return false;
    }

    QString columnsText;
    qint32 r, g, b;
    KisSwatch e;

    // Read name
    if (!lines[0].startsWith("GIMP") || !lines[1].toLower().contains("name")) {
        warnPigment << "Illegal Gimp palette file: " << colorSet->filename();
        return false;
    }

    colorSet->setName(i18n(lines[1].split(":")[1].trimmed().toLatin1()));

    index = 2;

    // Read columns
    int columns = 0;
    if (lines[index].toLower().contains("columns")) {
        columnsText = lines[index].split(":")[1].trimmed();
        columns = columnsText.toInt();
        global().setColumnCount(columns);
        index = 3;
    }


    for (qint32 i = index; i < lines.size(); i++) {
        if (lines[i].startsWith('#')) {
            comment += lines[i].mid(1).trimmed() + ' ';
        } else if (!lines[i].isEmpty()) {
            QStringList a = lines[i].replace('\t', ' ').split(' ', QString::SkipEmptyParts);

            if (a.count() < 3) {
                break;
            }

            r = qBound(0, a[0].toInt(), 255);
            g = qBound(0, a[1].toInt(), 255);
            b = qBound(0, a[2].toInt(), 255);

            e.setColor(KoColor(QColor(r, g, b), KoColorSpaceRegistry::instance()->rgb8()));

            QString name = a.join(" ");
            e.setName(name.isEmpty() ? i18n("Untitled") : name);

            global().addEntry(e);
        }
    }
    return true;
}

bool KoColorSet::Private::loadAct()
{
    QFileInfo info(colorSet->filename());
    colorSet->setName(info.baseName());
    KisSwatch e;
    for (int i = 0; i < data.size(); i += 3) {
        quint8 r = data[i];
        quint8 g = data[i+1];
        quint8 b = data[i+2];
        e.setColor(KoColor(QColor(r, g, b), KoColorSpaceRegistry::instance()->rgb8()));
        global().addEntry(e);
    }
    return true;
}

bool KoColorSet::Private::loadRiff()
{
    // http://worms2d.info/Palette_file
    QFileInfo info(colorSet->filename());
    colorSet->setName(info.baseName());
    KisSwatch e;

    RiffHeader header;
    memcpy(&header, data.constData(), sizeof(RiffHeader));
    header.colorcount = qFromBigEndian(header.colorcount);

    for (int i = sizeof(RiffHeader);
         (i < (int)(sizeof(RiffHeader) + header.colorcount) && i < data.size());
         i += 4) {
        quint8 r = data[i];
        quint8 g = data[i+1];
        quint8 b = data[i+2];
        e.setColor(KoColor(QColor(r, g, b), KoColorSpaceRegistry::instance()->rgb8()));
        groups[GLOBAL_GROUP_NAME].addEntry(e);
    }
    return true;
}


bool KoColorSet::Private::loadPsp()
{
    QFileInfo info(colorSet->filename());
    colorSet->setName(info.baseName());
    KisSwatch e;
    qint32 r, g, b;

    QString s = QString::fromUtf8(data.data(), data.count());
    QStringList l = s.split('\n', QString::SkipEmptyParts);
    if (l.size() < 4) return false;
    if (l[0] != "JASC-PAL") return false;
    if (l[1] != "0100") return false;

    int entries = l[2].toInt();

    for (int i = 0; i < entries; ++i)  {

        QStringList a = l[i + 3].replace('\t', ' ').split(' ', QString::SkipEmptyParts);

        if (a.count() != 3) {
            continue;
        }

        r = qBound(0, a[0].toInt(), 255);
        g = qBound(0, a[1].toInt(), 255);
        b = qBound(0, a[2].toInt(), 255);

        e.setColor(KoColor(QColor(r, g, b),
                           KoColorSpaceRegistry::instance()->rgb8()));

        QString name = a.join(" ");
        e.setName(name.isEmpty() ? i18n("Untitled") : name);

        groups[GLOBAL_GROUP_NAME].addEntry(e);
    }
    return true;
}

bool KoColorSet::Private::loadKpl()
{
    QBuffer buf(&data);
    buf.open(QBuffer::ReadOnly);

    QScopedPointer<KoStore> store(KoStore::createStore(&buf, KoStore::Read, "application/x-krita-palette", KoStore::Zip));
    if (!store || store->bad()) return false;

    if (store->hasFile("profiles.xml")) {
        if (!store->open("profiles.xml")) { return false; }
        QByteArray data;
        data.resize(store->size());
        QByteArray ba = store->read(store->size());
        store->close();

        QDomDocument doc;
        doc.setContent(ba);
        QDomElement e = doc.documentElement();
        QDomElement c = e.firstChildElement("Profiles");
        while (!c.isNull()) {
            QString name = c.attribute("name");
            QString filename = c.attribute("filename");
            QString colorModelId = c.attribute("colorModelId");
            QString colorDepthId = c.attribute("colorDepthId");
            if (!KoColorSpaceRegistry::instance()->profileByName(name)) {
                store->open(filename);
                QByteArray data;
                data.resize(store->size());
                data = store->read(store->size());
                store->close();

                const KoColorProfile *profile = KoColorSpaceRegistry::instance()->createColorProfile(colorModelId, colorDepthId, data);
                if (profile && profile->valid()) {
                    KoColorSpaceRegistry::instance()->addProfile(profile);
                }
            }

            c = c.nextSiblingElement();
        }
    }

    {
        if (!store->open("colorset.xml")) { return false; }
        QByteArray data;
        data.resize(store->size());
        QByteArray ba = store->read(store->size());
        store->close();

        QDomDocument doc;
        doc.setContent(ba);
        QDomElement e = doc.documentElement();
        colorSet->setName(e.attribute("name"));
        colorSet->setColumnCount(e.attribute("columns").toInt());
        comment = e.attribute("comment");

        QDomElement c = e.firstChildElement("ColorSetEntry");
        while (!c.isNull()) {
            QString colorDepthId = c.attribute("bitdepth", Integer8BitsColorDepthID.id());
            KisSwatch entry;

            entry.setColor(KoColor::fromXML(c.firstChildElement(), colorDepthId));
            entry.setName(c.attribute("name"));
            entry.setId(c.attribute("id"));
            entry.setSpotColor(c.attribute("spot", "false") == "true" ? true : false);
            groups[GLOBAL_GROUP_NAME].addEntry(entry);

            c = c.nextSiblingElement("ColorSetEntry");
        }
        QDomElement g = e.firstChildElement("Group");
        while (!g.isNull()) {
            QString groupName = g.attribute("name");
            colorSet->addGroup(groupName);
            QDomElement cg = g.firstChildElement("ColorSetEntry");
            while (!cg.isNull()) {
                QString colorDepthId = cg.attribute("bitdepth", Integer8BitsColorDepthID.id());
                KisSwatch entry;

                entry.setColor(KoColor::fromXML(cg.firstChildElement(), colorDepthId));
                entry.setName(cg.attribute("name"));
                entry.setId(cg.attribute("id"));
                entry.setSpotColor(cg.attribute("spot", "false") == "true" ? true : false);
                groups[groupName].addEntry(entry);

                cg = cg.nextSiblingElement("ColorSetEntry");

            }
            g = g.nextSiblingElement("Group");
        }
    }

    buf.close();
    return true;
}

bool KoColorSet::Private::loadAco()
{
    QFileInfo info(colorSet->filename());
    colorSet->setName(info.baseName());

    QBuffer buf(&data);
    buf.open(QBuffer::ReadOnly);

    quint16 version = readShort(&buf);
    quint16 numColors = readShort(&buf);
    KisSwatch e;

    if (version == 1 && buf.size() > 4+numColors*10) {
        buf.seek(4+numColors*10);
        version = readShort(&buf);
        numColors = readShort(&buf);
    }

    const quint16 quint16_MAX = 65535;

    for (int i = 0; i < numColors && !buf.atEnd(); ++i) {

        quint16 colorSpace = readShort(&buf);
        quint16 ch1 = readShort(&buf);
        quint16 ch2 = readShort(&buf);
        quint16 ch3 = readShort(&buf);
        quint16 ch4 = readShort(&buf);

        bool skip = false;
        if (colorSpace == 0) { // RGB
            const KoColorProfile *srgb = KoColorSpaceRegistry::instance()->rgb8()->profile();
            KoColor c(KoColorSpaceRegistry::instance()->rgb16(srgb));
            reinterpret_cast<quint16*>(c.data())[0] = ch3;
            reinterpret_cast<quint16*>(c.data())[1] = ch2;
            reinterpret_cast<quint16*>(c.data())[2] = ch1;
            c.setOpacity(OPACITY_OPAQUE_U8);
            e.setColor(c);
        }
        else if (colorSpace == 1) { // HSB
            QColor qc;
            qc.setHsvF(ch1 / 65536.0, ch2 / 65536.0, ch3 / 65536.0);
            KoColor c(qc, KoColorSpaceRegistry::instance()->rgb16());
            c.setOpacity(OPACITY_OPAQUE_U8);
            e.setColor(c);
        }
        else if (colorSpace == 2) { // CMYK
            KoColor c(KoColorSpaceRegistry::instance()->colorSpace(CMYKAColorModelID.id(), Integer16BitsColorDepthID.id(), QString()));
            reinterpret_cast<quint16*>(c.data())[0] = quint16_MAX - ch1;
            reinterpret_cast<quint16*>(c.data())[1] = quint16_MAX - ch2;
            reinterpret_cast<quint16*>(c.data())[2] = quint16_MAX - ch3;
            reinterpret_cast<quint16*>(c.data())[3] = quint16_MAX - ch4;
            c.setOpacity(OPACITY_OPAQUE_U8);
            e.setColor(c);
        }
        else if (colorSpace == 7) { // LAB
            KoColor c = KoColor(KoColorSpaceRegistry::instance()->lab16());
            reinterpret_cast<quint16*>(c.data())[0] = ch3;
            reinterpret_cast<quint16*>(c.data())[1] = ch2;
            reinterpret_cast<quint16*>(c.data())[2] = ch1;
            c.setOpacity(OPACITY_OPAQUE_U8);
            e.setColor(c);
        }
        else if (colorSpace == 8) { // GRAY
            KoColor c(KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), Integer16BitsColorDepthID.id(), QString()));
            reinterpret_cast<quint16*>(c.data())[0] = ch1 * (quint16_MAX / 10000);
            c.setOpacity(OPACITY_OPAQUE_U8);
            e.setColor(c);
        }
        else {
            warnPigment << "Unsupported colorspace in palette" << colorSet->filename() << "(" << colorSpace << ")";
            skip = true;
        }
        if (version == 2) {
            quint16 v2 = readShort(&buf); //this isn't a version, it's a marker and needs to be skipped.
            Q_UNUSED(v2);
            quint16 size = readShort(&buf) -1; //then comes the length
            if (size>0) {
                QByteArray ba = buf.read(size*2);
                if (ba.size() == size*2) {
                    QTextCodec *Utf16Codec = QTextCodec::codecForName("UTF-16BE");
                    e.setName(Utf16Codec->toUnicode(ba));
                } else {
                    warnPigment << "Version 2 name block is the wrong size" << colorSet->filename();
                }
            }
            v2 = readShort(&buf); //end marker also needs to be skipped.
            Q_UNUSED(v2);
        }
        if (!skip) {
            groups[GLOBAL_GROUP_NAME].addEntry(e);
        }
    }
    return true;
}

bool KoColorSet::Private::loadSbz() {
    QBuffer buf(&data);
    buf.open(QBuffer::ReadOnly);

    // &buf is a subclass of QIODevice
    QScopedPointer<KoStore> store(KoStore::createStore(&buf, KoStore::Read, "application/x-swatchbook", KoStore::Zip));
    if (!store || store->bad()) return false;

    if (store->hasFile("swatchbook.xml")) { // Try opening...

        if (!store->open("swatchbook.xml")) { return false; }
        QByteArray data;
        data.resize(store->size());
        QByteArray ba = store->read(store->size());
        store->close();

        dbgPigment << "XML palette: " << colorSet->filename() << ", SwatchBooker format";

        QDomDocument doc;
        int errorLine, errorColumn;
        QString errorMessage;
        bool status = doc.setContent(ba, &errorMessage, &errorLine, &errorColumn);
        if (!status) {
            warnPigment << "Illegal XML palette:" << colorSet->filename();
            warnPigment << "Error (line" << errorLine << ", column" << errorColumn << "):" << errorMessage;
            return false;
        }

        QDomElement e = doc.documentElement(); // SwatchBook

        // Start reading properties...
        QDomElement metadata = e.firstChildElement("metadata");

        if (e.isNull()) {
            warnPigment << "Palette metadata not found";
            return false;
        }

        QDomElement title = metadata.firstChildElement("dc:title");
        QString colorName = title.text();
        colorName = colorName.isEmpty() ? i18n("Untitled") : colorName;
        colorSet->setName(colorName);
        dbgPigment << "Processed name of palette:" << colorSet->name();
        // End reading properties

        // Now read colors...
        QDomElement materials = e.firstChildElement("materials");
        if (materials.isNull()) {
            warnPigment << "Materials (color definitions) not found";
            return false;
        }
        // This one has lots of "color" elements
        QDomElement colorElement = materials.firstChildElement("color");
        if (colorElement.isNull()) {
            warnPigment << "Color definitions not found (line" << materials.lineNumber() << ", column" << materials.columnNumber() << ")";
            return false;
        }

        // Also read the swatch book...
        QDomElement book = e.firstChildElement("book");
        if (book.isNull()) {
            warnPigment << "Palette book (swatch composition) not found (line" << e.lineNumber() << ", column" << e.columnNumber() << ")";
            return false;
        }
        // Which has lots of "swatch"es (todo: support groups)
        QDomElement swatch = book.firstChildElement();
        if (swatch.isNull()) {
            warnPigment << "Swatches/groups definition not found (line" << book.lineNumber() << ", column" << book.columnNumber() << ")";
            return false;
        }

        // We'll store colors here, and as we process swatches
        // we'll add them to the palette
        QHash<QString, KisSwatch> materialsBook;
        QHash<QString, const KoColorSpace*> fileColorSpaces;

        // Color processing
        for(; !colorElement.isNull(); colorElement = colorElement.nextSiblingElement("color"))
        {
            KisSwatch currentEntry;
            // Set if color is spot
            currentEntry.setSpotColor(colorElement.attribute("usage") == "spot");

            // <metadata> inside contains id and name
            // one or more <values> define the color
            QDomElement currentColorMetadata = colorElement.firstChildElement("metadata");
            QDomNodeList currentColorValues = colorElement.elementsByTagName("values");
            // Get color name
            QDomElement colorTitle = currentColorMetadata.firstChildElement("dc:title");
            QDomElement colorId = currentColorMetadata.firstChildElement("dc:identifier");
            // Is there an id? (we need that at the very least for identifying a color)
            if (colorId.text().isEmpty()) {
                warnPigment << "Unidentified color (line" << colorId.lineNumber()<< ", column" << colorId.columnNumber() << ")";
                return false;
            }
            if (materialsBook.contains(colorId.text())) {
                warnPigment << "Duplicated color definition (line" << colorId.lineNumber()<< ", column" << colorId.columnNumber() << ")";
                return false;
            }

            // Get a valid color name
            currentEntry.setId(colorId.text());
            currentEntry.setName(colorTitle.text().isEmpty() ? colorId.text() : colorTitle.text());

            // Get a valid color definition
            if (currentColorValues.isEmpty()) {
                warnPigment << "Color definitions not found (line" << colorElement.lineNumber() << ", column" << colorElement.columnNumber() << ")";
                return false;
            }

            bool firstDefinition = false;
            const KoColorProfile *srgb = KoColorSpaceRegistry::instance()->rgb8()->profile();
            // Priority: Lab, otherwise the first definition found
            for(int j = 0; j < currentColorValues.size(); j++) {
                QDomNode colorValue = currentColorValues.at(j);
                QDomElement colorValueE = colorValue.toElement();
                QString model = colorValueE.attribute("model", QString());

                // sRGB,RGB,HSV,HSL,CMY,CMYK,nCLR: 0 -> 1
                // YIQ: Y 0 -> 1 : IQ -0.5 -> 0.5
                // Lab: L 0 -> 100 : ab -128 -> 127
                // XYZ: 0 -> ~100
                if (model == "Lab") {
                    QStringList lab = colorValueE.text().split(" ");
                    if (lab.length() != 3) {
                        warnPigment << "Invalid Lab color definition (line" << colorValueE.lineNumber() << ", column" << colorValueE.columnNumber() << ")";
                    }
                    float l = lab.at(0).toFloat(&status);
                    float a = lab.at(1).toFloat(&status);
                    float b = lab.at(2).toFloat(&status);
                    if (!status) {
                        warnPigment << "Invalid float definition (line" << colorValueE.lineNumber() << ", column" << colorValueE.columnNumber() << ")";
                    }

                    KoColor c(KoColorSpaceRegistry::instance()->colorSpace(LABAColorModelID.id(), Float32BitsColorDepthID.id(), QString()));
                    reinterpret_cast<float*>(c.data())[0] = l;
                    reinterpret_cast<float*>(c.data())[1] = a;
                    reinterpret_cast<float*>(c.data())[2] = b;
                    c.setOpacity(OPACITY_OPAQUE_F);
                    firstDefinition = true;
                    currentEntry.setColor(c);
                    break; // Immediately add this one
                }
                else if (model == "sRGB" && !firstDefinition) {
                    QStringList rgb = colorValueE.text().split(" ");
                    if (rgb.length() != 3) {
                        warnPigment << "Invalid sRGB color definition (line" << colorValueE.lineNumber() << ", column" << colorValueE.columnNumber() << ")";
                    }
                    float r = rgb.at(0).toFloat(&status);
                    float g = rgb.at(1).toFloat(&status);
                    float b = rgb.at(2).toFloat(&status);
                    if (!status) {
                        warnPigment << "Invalid float definition (line" << colorValueE.lineNumber() << ", column" << colorValueE.columnNumber() << ")";
                    }

                    KoColor c(KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), srgb));
                    reinterpret_cast<float*>(c.data())[0] = r;
                    reinterpret_cast<float*>(c.data())[1] = g;
                    reinterpret_cast<float*>(c.data())[2] = b;
                    c.setOpacity(OPACITY_OPAQUE_F);
                    currentEntry.setColor(c);
                    firstDefinition = true;
                }
                else if (model == "XYZ" && !firstDefinition) {
                    QStringList xyz = colorValueE.text().split(" ");
                    if (xyz.length() != 3) {
                        warnPigment << "Invalid XYZ color definition (line" << colorValueE.lineNumber() << ", column" << colorValueE.columnNumber() << ")";
                    }
                    float x = xyz.at(0).toFloat(&status);
                    float y = xyz.at(1).toFloat(&status);
                    float z = xyz.at(2).toFloat(&status);
                    if (!status) {
                        warnPigment << "Invalid float definition (line" << colorValueE.lineNumber() << ", column" << colorValueE.columnNumber() << ")";
                    }

                    KoColor c(KoColorSpaceRegistry::instance()->colorSpace(XYZAColorModelID.id(), Float32BitsColorDepthID.id(), QString()));
                    reinterpret_cast<float*>(c.data())[0] = x;
                    reinterpret_cast<float*>(c.data())[1] = y;
                    reinterpret_cast<float*>(c.data())[2] = z;
                    c.setOpacity(OPACITY_OPAQUE_F);
                    currentEntry.setColor(c);
                    firstDefinition = true;
                }
                // The following color spaces admit an ICC profile (in SwatchBooker)
                else if (model == "CMYK" && !firstDefinition) {
                    QStringList cmyk = colorValueE.text().split(" ");
                    if (cmyk.length() != 4) {
                        warnPigment << "Invalid CMYK color definition (line" << colorValueE.lineNumber() << ", column" << colorValueE.columnNumber() << ")";
                    }
                    float c = cmyk.at(0).toFloat(&status);
                    float m = cmyk.at(1).toFloat(&status);
                    float y = cmyk.at(2).toFloat(&status);
                    float k = cmyk.at(3).toFloat(&status);
                    if (!status) {
                        warnPigment << "Invalid float definition (line" << colorValueE.lineNumber() << ", column" << colorValueE.columnNumber() << ")";
                    }

                    QString space = colorValueE.attribute("space");
                    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->colorSpace(CMYKAColorModelID.id(), Float32BitsColorDepthID.id(), QString());

                    if (!space.isEmpty()) {
                        // Try loading the profile and add it to the registry
                        if (!fileColorSpaces.contains(space)) {
                            store->enterDirectory("profiles");
                            store->open(space);
                            QByteArray data;
                            data.resize(store->size());
                            data = store->read(store->size());
                            store->close();

                            const KoColorProfile *profile = KoColorSpaceRegistry::instance()->createColorProfile(CMYKAColorModelID.id(), Float32BitsColorDepthID.id(), data);
                            if (profile && profile->valid()) {
                                KoColorSpaceRegistry::instance()->addProfile(profile);
                                colorSpace = KoColorSpaceRegistry::instance()->colorSpace(CMYKAColorModelID.id(), Float32BitsColorDepthID.id(), profile);
                                fileColorSpaces.insert(space, colorSpace);
                            }
                        }
                        else {
                            colorSpace = fileColorSpaces.value(space);
                        }
                    }

                    KoColor color(colorSpace);
                    reinterpret_cast<float*>(color.data())[0] = c;
                    reinterpret_cast<float*>(color.data())[1] = m;
                    reinterpret_cast<float*>(color.data())[2] = y;
                    reinterpret_cast<float*>(color.data())[3] = k;
                    color.setOpacity(OPACITY_OPAQUE_F);
                    currentEntry.setColor(color);
                    firstDefinition = true;
                }
                else if (model == "GRAY" && !firstDefinition) {
                    QString gray = colorValueE.text();

                    float g = gray.toFloat(&status);
                    if (!status) {
                        warnPigment << "Invalid float definition (line" << colorValueE.lineNumber() << ", column" << colorValueE.columnNumber() << ")";
                    }

                    QString space = colorValueE.attribute("space");
                    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), Float32BitsColorDepthID.id(), QString());

                    if (!space.isEmpty()) {
                        // Try loading the profile and add it to the registry
                        if (!fileColorSpaces.contains(space)) {
                            store->enterDirectory("profiles");
                            store->open(space);
                            QByteArray data;
                            data.resize(store->size());
                            data = store->read(store->size());
                            store->close();

                            const KoColorProfile *profile = KoColorSpaceRegistry::instance()->createColorProfile(CMYKAColorModelID.id(), Float32BitsColorDepthID.id(), data);
                            if (profile && profile->valid()) {
                                KoColorSpaceRegistry::instance()->addProfile(profile);
                                colorSpace = KoColorSpaceRegistry::instance()->colorSpace(CMYKAColorModelID.id(), Float32BitsColorDepthID.id(), profile);
                                fileColorSpaces.insert(space, colorSpace);
                            }
                        }
                        else {
                            colorSpace = fileColorSpaces.value(space);
                        }
                    }

                    KoColor c(colorSpace);
                    reinterpret_cast<float*>(c.data())[0] = g;
                    c.setOpacity(OPACITY_OPAQUE_F);
                    currentEntry.setColor(c);
                    firstDefinition = true;
                }
                else if (model == "RGB" && !firstDefinition) {
                    QStringList rgb = colorValueE.text().split(" ");
                    if (rgb.length() != 3) {
                        warnPigment << "Invalid RGB color definition (line" << colorValueE.lineNumber() << ", column" << colorValueE.columnNumber() << ")";
                    }
                    float r = rgb.at(0).toFloat(&status);
                    float g = rgb.at(1).toFloat(&status);
                    float b = rgb.at(2).toFloat(&status);
                    if (!status) {
                        warnPigment << "Invalid float definition (line" << colorValueE.lineNumber() << ", column" << colorValueE.columnNumber() << ")";
                    }

                    QString space = colorValueE.attribute("space");
                    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), srgb);

                    if (!space.isEmpty()) {
                        // Try loading the profile and add it to the registry
                        if (!fileColorSpaces.contains(space)) {
                            store->enterDirectory("profiles");
                            store->open(space);
                            QByteArray data;
                            data.resize(store->size());
                            data = store->read(store->size());
                            store->close();

                            const KoColorProfile *profile = KoColorSpaceRegistry::instance()->createColorProfile(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), data);
                            if (profile && profile->valid()) {
                                KoColorSpaceRegistry::instance()->addProfile(profile);
                                colorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), profile);
                                fileColorSpaces.insert(space, colorSpace);
                            }
                        }
                        else {
                            colorSpace = fileColorSpaces.value(space);
                        }
                    }

                    KoColor c(colorSpace);
                    reinterpret_cast<float*>(c.data())[0] = r;
                    reinterpret_cast<float*>(c.data())[1] = g;
                    reinterpret_cast<float*>(c.data())[2] = b;
                    c.setOpacity(OPACITY_OPAQUE_F);
                    currentEntry.setColor(c);
                    firstDefinition = true;
                }
                else {
                    warnPigment << "Color space not implemented:" << model << "(line" << colorValueE.lineNumber() << ", column "<< colorValueE.columnNumber() << ")";
                }
            }
            if (firstDefinition) {
                materialsBook.insert(currentEntry.id(), currentEntry);
            }
            else {
                warnPigment << "No supported color  spaces for the current color (line" << colorElement.lineNumber() << ", column "<< colorElement.columnNumber() << ")";
                return false;
            }
        }
        // End colors
        // Now decide which ones will go into the palette

        for(;!swatch.isNull(); swatch = swatch.nextSiblingElement()) {
            QString type = swatch.tagName();
            if (type.isEmpty() || type.isNull()) {
                warnPigment << "Invalid swatch/group definition (no id) (line" << swatch.lineNumber() << ", column" << swatch.columnNumber() << ")";
                return false;
            }
            else if (type == "swatch") {
                QString id = swatch.attribute("material");
                if (id.isEmpty() || id.isNull()) {
                    warnPigment << "Invalid swatch definition (no material id) (line" << swatch.lineNumber() << ", column" << swatch.columnNumber() << ")";
                    return false;
                }
                if (materialsBook.contains(id)) {
                    groups[GLOBAL_GROUP_NAME].addEntry(materialsBook.value(id));
                }
                else {
                    warnPigment << "Invalid swatch definition (material not found) (line" << swatch.lineNumber() << ", column" << swatch.columnNumber() << ")";
                    return false;
                }
            }
            else if (type == "group") {
                QDomElement groupMetadata = swatch.firstChildElement("metadata");
                if (groupMetadata.isNull()) {
                    warnPigment << "Invalid group definition (missing metadata) (line" << groupMetadata.lineNumber() << ", column" << groupMetadata.columnNumber() << ")";
                    return false;
                }
                QDomElement groupTitle = metadata.firstChildElement("dc:title");
                if (groupTitle.isNull()) {
                    warnPigment << "Invalid group definition (missing title) (line" << groupTitle.lineNumber() << ", column" << groupTitle.columnNumber() << ")";
                    return false;
                }
                QString currentGroupName = groupTitle.text();

                QDomElement groupSwatch = swatch.firstChildElement("swatch");

                while(!groupSwatch.isNull()) {
                    QString id = groupSwatch.attribute("material");
                    if (id.isEmpty() || id.isNull()) {
                        warnPigment << "Invalid swatch definition (no material id) (line" << groupSwatch.lineNumber() << ", column" << groupSwatch.columnNumber() << ")";
                        return false;
                    }
                    if (materialsBook.contains(id)) {
                        groups[currentGroupName].addEntry(materialsBook.value(id));
                    }
                    else {
                        warnPigment << "Invalid swatch definition (material not found) (line" << groupSwatch.lineNumber() << ", column" << groupSwatch.columnNumber() << ")";
                        return false;
                    }
                    groupSwatch = groupSwatch.nextSiblingElement("swatch");
                }
            }
        }
        // End palette
    }

    buf.close();
    return true;
}

bool KoColorSet::Private::loadXml() {
    bool res = false;

    QXmlStreamReader *xml = new QXmlStreamReader(data);

    if (xml->readNextStartElement()) {
        QStringRef paletteId = xml->name();
        if (QStringRef::compare(paletteId, "SCRIBUSCOLORS", Qt::CaseInsensitive) == 0) { // Scribus
            dbgPigment << "XML palette: " << colorSet->filename() << ", Scribus format";
            res = loadScribusXmlPalette(colorSet, xml);
        }
        else {
            // Unknown XML format
            xml->raiseError("Unknown XML palette format. Expected SCRIBUSCOLORS, found " + paletteId);
        }
    }

    // If there is any error (it should be returned through the stream)
    if (xml->hasError() || !res) {
        warnPigment << "Illegal XML palette:" << colorSet->filename();
        warnPigment << "Error (line"<< xml->lineNumber() << ", column" << xml->columnNumber() << "):" << xml->errorString();
        return false;
    }
    else {
        dbgPigment << "XML palette parsed successfully:" << colorSet->filename();
        return true;
    }
}

bool KoColorSet::Private::saveKpl(QIODevice *dev) const
{
    QScopedPointer<KoStore> store(KoStore::createStore(dev, KoStore::Write, "application/x-krita-palette", KoStore::Zip));
    if (!store || store->bad()) return false;

    QSet<const KoColorProfile *> profiles;
    QMap<const KoColorProfile*, const KoColorSpace*> profileMap;

    {
        QDomDocument doc;
        QDomElement root = doc.createElement("Colorset");
        root.setAttribute("version", "1.0");
        root.setAttribute("name", colorSet->name());
        root.setAttribute("comment", comment);
        root.setAttribute("columns", groups[GLOBAL_GROUP_NAME].columnCount());
        for (int x = 0; x < groups[GLOBAL_GROUP_NAME].columnCount(); x++) {
            for (int y = 0; y < groups[GLOBAL_GROUP_NAME].rowCount(); y++) {
                // Only save non-builtin profiles.=
                /*
                const KoColorProfile *profile = entry.color().colorSpace()->profile();
                if (!profile->fileName().isEmpty()) {
                    profiles << profile;
                    profileMap[profile] = entry.color().colorSpace();
                }
                QDomElement el = doc.createElement("ColorSetEntry");
                el.setAttribute("name", entry.name());
                el.setAttribute("id", entry.id());
                el.setAttribute("spot", entry.spotColor() ? "true" : "false");
                el.setAttribute("bitdepth", entry.color().colorSpace()->colorDepthId().id());
                entry.color().toXML(doc, el);
                root.appendChild(el);
                */
            }
        }
        Q_FOREACH(const QString &groupName, groupNames) {
            QDomElement gl = doc.createElement("Group");
            gl.setAttribute("name", groupName);
            root.appendChild(gl);
            for (int x = 0; x < groups[groupName].columnCount(); x++) {
                for (int y = 0; y < groups[groupName].rowCount(); y++) {
                    // Only save non-builtin profiles.=
                    /*
                    const KoColorProfile *profile = entry.color().colorSpace()->profile();
                    if (!profile->fileName().isEmpty()) {
                        profiles << profile;
                        profileMap[profile] = entry.color().colorSpace();
                    }
                    QDomElement el = doc.createElement("ColorSetEntry");
                    el.setAttribute("name", entry.name());
                    el.setAttribute("id", entry.id());
                    el.setAttribute("spot", entry.spotColor() ? "true" : "false");
                    el.setAttribute("bitdepth", entry.color().colorSpace()->colorDepthId().id());
                    entry.color().toXML(doc, el);
                    gl.appendChild(el);
                    */
                }
            }
        }

        doc.appendChild(root);
        if (!store->open("colorset.xml")) { return false; }
        QByteArray ba = doc.toByteArray();
        if (store->write(ba) != ba.size()) { return false; }
        if (!store->close()) { return false; }
    }

    QDomDocument doc;
    QDomElement profileElement = doc.createElement("Profiles");

    Q_FOREACH(const KoColorProfile *profile, profiles) {
        QString fn = QFileInfo(profile->fileName()).fileName();
        if (!store->open(fn)) { return false; }
        QByteArray profileRawData = profile->rawData();
        if (!store->write(profileRawData)) { return false; }
        if (!store->close()) { return false; }
        QDomElement el = doc.createElement("Profile");
        el.setAttribute("filename", fn);
        el.setAttribute("name", profile->name());
        el.setAttribute("colorModelId", profileMap[profile]->colorModelId().id());
        el.setAttribute("colorDepthId", profileMap[profile]->colorDepthId().id());
        profileElement.appendChild(el);

    }
    doc.appendChild(profileElement);
    if (!store->open("profiles.xml")) { return false; }
    QByteArray ba = doc.toByteArray();
    if (store->write(ba) != ba.size()) { return false; }
    if (!store->close()) { return false; }

    return store->finalize();
}
