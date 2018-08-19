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
#include <QtEndian> // qFromLittleEndian

#include <QImage>
#include <QPoint>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QBuffer>
#include <QByteArray>
#include <QPainter>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>
#include <QXmlStreamReader>
#include <QTextCodec>
#include <QMap>
#include <QHash>
#include <QStringList>

#include <DebugPigment.h>
#include <klocalizedstring.h>

#include <KoStore.h>
#include "KoColor.h"
#include "KoColorSetEntry.h"
#include "KoColorProfile.h"
#include "KoColorSpaceRegistry.h"
#include "KoColorModelStandardIds.h"


struct KoColorSet::Private {
    KoColorSet::PaletteType paletteType;
    QByteArray data;
    QString comment;
    qint32 columns {0}; // Set the default value that the GIMP uses...
    QVector<KoColorSetEntry> colors; //ungrouped colors
    QStringList groupNames; //names of the groups, this is used to determine the order they are in.
    QMap<QString, QVector<KoColorSetEntry>> groups; //grouped colors.
};

KoColorSet::PaletteType detectFormat(const QString &fileName, const QByteArray &ba) {

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

KoColorSet::KoColorSet(const QString& filename)
    : KoResource(filename)
    , d(new Private())
{
}

KoColorSet::KoColorSet()
    : KoResource(QString())
    , d(new Private())
{

}

/// Create an copied palette
KoColorSet::KoColorSet(const KoColorSet& rhs)
    : QObject(0)
    , KoResource(QString())
    , d(new Private())
{
    setFilename(rhs.filename());
    d->comment = rhs.d->comment;
    d->columns = rhs.d->columns;
    d->colors = rhs.d->colors;
    d->groupNames = rhs.d->groupNames;
    d->groups = rhs.d->groups;
    setValid(true);
}

KoColorSet::~KoColorSet()
{
}

bool KoColorSet::load()
{
    QFile file(filename());
    if (file.size() == 0) return false;
    if (!file.open(QIODevice::ReadOnly)) {
        warnPigment << "Can't open file " << filename();
        return false;
    }
    bool res =  loadFromDevice(&file);
    file.close();
    return res;
}

bool KoColorSet::loadFromDevice(QIODevice *dev)
{
    if (!dev->isOpen()) dev->open(QIODevice::ReadOnly);

    d->data = dev->readAll();

    Q_ASSERT(d->data.size() != 0);

    return init();
}


bool KoColorSet::save()
{
    QFile file(filename());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    saveToDevice(&file);
    file.close();
    return true;
}

bool KoColorSet::saveToDevice(QIODevice *dev) const
{
    bool res;
    switch(d->paletteType) {
    case GPL:
        res = saveGpl(dev);
        break;
    default:
        res = saveKpl(dev);
    }
    if (res) {
        KoResource::saveToDevice(dev);
    }
    return res;
}

bool KoColorSet::init()
{
    d->colors.clear(); // just in case this is a reload (eg by KoEditColorSetDialog),
    d->groups.clear();
    d->groupNames.clear();

    if (filename().isNull()) {
        warnPigment << "Cannot load palette" << name() << "there is no filename set";
        return false;
    }
    if (d->data.isNull()) {
        QFile file(filename());
        if (file.size() == 0) {
            warnPigment << "Cannot load palette" << name() << "there is no data available";
            return false;
        }
        file.open(QIODevice::ReadOnly);
        d->data = file.readAll();
        file.close();
    }

    bool res = false;
    d->paletteType = detectFormat(filename(), d->data);
    switch(d->paletteType) {
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
    setValid(res);

    if (d->columns == 0) {
        d->columns = 10;
    }

    QImage img(d->columns * 4, (d->colors.size() / d->columns) * 4, QImage::Format_ARGB32);
    QPainter gc(&img);
    gc.fillRect(img.rect(), Qt::darkGray);
    int counter = 0;
    for(int i = 0; i < d->columns; ++i) {
        for (int j = 0; j < (d->colors.size() / d->columns); ++j) {
            if (counter < d->colors.size()) {
                QColor c = d->colors.at(counter).color().toQColor();
                gc.fillRect(i * 4, j * 4, 4, 4, c);
                counter++;
            }
            else {
                break;
            }
        }
    }
    setImage(img);

    // save some memory
    d->data.clear();
    return res;
}

bool KoColorSet::saveGpl(QIODevice *dev) const
{
    QTextStream stream(dev);
    stream << "GIMP Palette\nName: " << name() << "\nColumns: " << d->columns << "\n#\n";

    for (int i = 0; i < d->colors.size(); i++) {
        const KoColorSetEntry& entry = d->colors.at(i);
        QColor c = entry.color().toQColor();
        stream << c.red() << " " << c.green() << " " << c.blue() << "\t";
        if (entry.name().isEmpty())
            stream << "Untitled\n";
        else
            stream << entry.name() << "\n";
    }

    return true;
}

quint32 KoColorSet::nColors()
{
    if (d->colors.isEmpty()) return 0;

    quint32 total = d->colors.size();
    if (!d->groups.empty()) {
        Q_FOREACH (const QVector<KoColorSetEntry> &group, d->groups.values()) {
            total += group.size();
        }
    }
    return total;
}

quint32 KoColorSet::nColorsGroup(QString groupName) {
    if (d->groups.contains(groupName)) {
        return d->groups.value(groupName).size();
    } else if (groupName.isEmpty() && !d->colors.isEmpty()){
        return d->colors.size();
    } else {
        return 0;
    }
}

quint32 KoColorSet::getIndexClosestColor(const KoColor color, bool useGivenColorSpace)
{
    quint32 closestIndex = 0;
    quint8 highestPercentage = 0;
    quint8 testPercentage = 0;
    KoColor compare = color;
    for (quint32 i=0; i<nColors(); i++) {
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

QString KoColorSet::closestColorName(const KoColor color, bool useGivenColorSpace)
{
    int i = getIndexClosestColor(color, useGivenColorSpace);
    return getColorGlobal(i).name();
}

void KoColorSet::add(const KoColorSetEntry & c, QString groupName)
{
    if (d->groups.contains(groupName) || d->groupNames.contains(groupName)) {
        d->groups[groupName].push_back(c);
    } else {
        d->colors.push_back(c);
    }
}

quint32 KoColorSet::insertBefore(const KoColorSetEntry &c, qint32 index, const QString &groupName)
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
}

void KoColorSet::removeAt(quint32 index, QString groupName)
{
    if (d->groups.contains(groupName)){
        if ((quint32)d->groups.value(groupName).size()>index) {
            d->groups[groupName].remove(index);
        }
    } else {
        if ((quint32)d->colors.size()>index) {
            d->colors.remove(index);
        }
    }
}

void KoColorSet::clear()
{
    d->colors.clear();
    d->groups.clear();
}

KoColorSetEntry KoColorSet::getColorGlobal(quint32 index)
{
    KoColorSetEntry e;
    quint32 groupIndex = index;
    QString groupName = findGroupByGlobalIndex(index, &groupIndex);
    e = getColorGroup(groupIndex, groupName);
    return e;
}

KoColorSetEntry KoColorSet::getColorGroup(quint32 index, QString groupName)
{
    KoColorSetEntry e;
    if (d->groups.contains(groupName)) {
        if (nColorsGroup(groupName)>index) {
            e = d->groups.value(groupName).at(index);
        } else {
            warnPigment<<index<<"is above"<<nColorsGroup(groupName)<<"of"<<groupName;
        }
    } else if (groupName.isEmpty() || groupName == QString()) {
        if (nColorsGroup(groupName)>index) {
            e = d->colors.at(index);
        } else {
            warnPigment<<index<<"is above the size of the default group:"<<nColorsGroup(groupName);
        }
    } else {
        warnPigment << "Color group "<<groupName<<" not found";
    }
    return e;
}

QString KoColorSet::findGroupByGlobalIndex(quint32 globalIndex, quint32 *index)
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

QString KoColorSet::findGroupByColorName(const QString &name, quint32 *index)
{
    *index = 0;
    QString groupName = QString();
    for (int i = 0; i<d->colors.size(); i++) {
        if(d->colors.at(i).name() == name) {
            *index = (quint32)i;
            return groupName;
        }
    }
    QStringList groupNames = getGroupNames();
    Q_FOREACH (QString name, groupNames) {
        for (int i=0; i<d->groups[name].size(); i++) {
            if(d->groups[name].at(i).name() == name) {
                *index = (quint32)i;
                groupName = name;
                return groupName;
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
}

QStringList KoColorSet::getGroupNames()
{
    if (d->groupNames.size()<d->groups.size()) {
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
    QVector<KoColorSetEntry> dummyList = d->groups.value(oldGroupName);
    d->groups.remove(oldGroupName);
    d->groups[newGroupName] = dummyList;
    //rename the string in the stringlist;
    int index = d->groupNames.indexOf(oldGroupName);
    d->groupNames.replace(index, newGroupName);
    return true;
}

bool KoColorSet::changeColorSetEntry(KoColorSetEntry entry, QString groupName, quint32 index)
{
    if (index>=nColorsGroup(groupName) || (d->groupNames.contains(groupName)==false &&  groupName.size()>0)) {
        return false;
    }

    if (groupName==QString()) {
        d->colors[index] = entry;
    } else {
        d->groups[groupName][index] = entry;
    }
    return true;
}

void KoColorSet::setColumnCount(int columns)
{
    d->columns = columns;
}

int KoColorSet::columnCount()
{
    return d->columns;
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
    d->groups[groupName] = QVector<KoColorSetEntry>();
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
        for (int i = 0; i<d->groups.value(groupName).size(); i++) {
            d->colors.append(d->groups.value(groupName).at(i));
        }
    }
    for(int n = 0; n<d->groupNames.size(); n++) {
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


bool KoColorSet::loadGpl()
{
    QString s = QString::fromUtf8(d->data.data(), d->data.count());

    if (s.isEmpty() || s.isNull() || s.length() < 50) {
        warnPigment << "Illegal Gimp palette file: " << filename();
        return false;
    }

    quint32 index = 0;

    QStringList lines = s.split('\n', QString::SkipEmptyParts);

    if (lines.size() < 3) {
        warnPigment << "Not enough lines in palette file: " << filename();
        return false;
    }

    QString columns;
    qint32 r, g, b;
    KoColorSetEntry e;

    // Read name


    if (!lines[0].startsWith("GIMP") || !lines[1].toLower().contains("name")) {
        warnPigment << "Illegal Gimp palette file: " << filename();
        return false;
    }

    setName(i18n(lines[1].split(":")[1].trimmed().toLatin1()));

    index = 2;

    // Read columns
    if (lines[index].toLower().contains("columns")) {
        columns = lines[index].split(":")[1].trimmed();
        d->columns = columns.toInt();
        index = 3;
    }


    for (qint32 i = index; i < lines.size(); i++) {

        if (lines[i].startsWith('#')) {
            d->comment += lines[i].mid(1).trimmed() + ' ';
        } else if (!lines[i].isEmpty()) {
            QStringList a = lines[i].replace('\t', ' ').split(' ', QString::SkipEmptyParts);

            if (a.count() < 3) {
                break;
            }

            r = a[0].toInt();
            a.pop_front();
            g = a[0].toInt();
            a.pop_front();
            b = a[0].toInt();
            a.pop_front();

            r = qBound(0, r, 255);
            g = qBound(0, g, 255);
            b = qBound(0, b, 255);

            e.setColor(KoColor(QColor(r, g, b), KoColorSpaceRegistry::instance()->rgb8()));

            QString name = a.join(" ");
            e.setName(name.isEmpty() ? i18n("Untitled") : name);

            add(e);
        }
    }
    return true;
}

bool KoColorSet::loadAct()
{
    QFileInfo info(filename());
    setName(info.baseName());
    KoColorSetEntry e;
    for (int i = 0; i < d->data.size(); i += 3) {
        quint8 r = d->data[i];
        quint8 g = d->data[i+1];
        quint8 b = d->data[i+2];
        e.setColor(KoColor(QColor(r, g, b), KoColorSpaceRegistry::instance()->rgb8()));
        add(e);
    }
    return true;
}

struct RiffHeader {
    quint32 riff;
    quint32 size;
    quint32 signature;
    quint32 data;
    quint32 datasize;
    quint16 version;
    quint16 colorcount;
};


bool KoColorSet::loadRiff()
{
    // http://worms2d.info/Palette_file
    QFileInfo info(filename());
    setName(info.baseName());
    KoColorSetEntry e;

    RiffHeader header;
    memcpy(&header, d->data.constData(), sizeof(RiffHeader));
    header.colorcount = qFromBigEndian(header.colorcount);

    for (int i = sizeof(RiffHeader);
         (i < (int)(sizeof(RiffHeader) + header.colorcount) && i < d->data.size());
         i += 4) {
        quint8 r = d->data[i];
        quint8 g = d->data[i+1];
        quint8 b = d->data[i+2];
        e.setColor(KoColor(QColor(r, g, b), KoColorSpaceRegistry::instance()->rgb8()));
        add(e);
    }
    return true;
}


bool KoColorSet::loadPsp()
{
    QFileInfo info(filename());
    setName(info.baseName());
    KoColorSetEntry e;
    qint32 r, g, b;

    QString s = QString::fromUtf8(d->data.data(), d->data.count());
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

        r = a[0].toInt();
        a.pop_front();
        g = a[0].toInt();
        a.pop_front();
        b = a[0].toInt();
        a.pop_front();

        r = qBound(0, r, 255);
        g = qBound(0, g, 255);
        b = qBound(0, b, 255);

        e.setColor(KoColor(QColor(r, g, b),
                    KoColorSpaceRegistry::instance()->rgb8()));

        QString name = a.join(" ");
        e.setName(name.isEmpty() ? i18n("Untitled") : name);

        add(e);
    }
    return true;
}

void scribusParseColor(KoColorSet *set, QXmlStreamReader *xml)
{
    KoColorSetEntry colorEntry;
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

bool loadScribusXmlPalette(KoColorSet *set, QXmlStreamReader *xml)
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

bool KoColorSet::loadXml() {
    bool res = false;

    QXmlStreamReader *xml = new QXmlStreamReader(d->data);

    if (xml->readNextStartElement()) {
        QStringRef paletteId = xml->name();
        if (QStringRef::compare(paletteId, "SCRIBUSCOLORS", Qt::CaseInsensitive) == 0) { // Scribus
            dbgPigment << "XML palette: " << filename() << ", Scribus format";
            res = loadScribusXmlPalette(this, xml);
        }
        else {
            // Unknown XML format
            xml->raiseError("Unknown XML palette format. Expected SCRIBUSCOLORS, found " + paletteId);
        }
    }

    // If there is any error (it should be returned through the stream)
    if (xml->hasError() || !res) {
        warnPigment << "Illegal XML palette:" << filename();
        warnPigment << "Error (line"<< xml->lineNumber() << ", column" << xml->columnNumber() << "):" << xml->errorString();
        return false;
    }
    else {
        dbgPigment << "XML palette parsed successfully:" << filename();
        return true;
    }
}

bool KoColorSet::saveKpl(QIODevice *dev) const
{
    QScopedPointer<KoStore> store(KoStore::createStore(dev, KoStore::Write, "application/x-krita-palette", KoStore::Zip));
    if (!store || store->bad()) return false;

    QSet<const KoColorProfile *> profiles;
    QMap<const KoColorProfile*, const KoColorSpace*> profileMap;

    {
        QDomDocument doc;
        QDomElement root = doc.createElement("Colorset");
        root.setAttribute("version", "1.0");
        root.setAttribute("name", name());
        root.setAttribute("comment", d->comment);
        root.setAttribute("columns", d->columns);
        Q_FOREACH(const KoColorSetEntry &entry, d->colors) {

            // Only save non-builtin profiles.=
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
        }
        Q_FOREACH(const QString &groupName, d->groupNames) {
            QDomElement gl = doc.createElement("Group");
            gl.setAttribute("name", groupName);
            root.appendChild(gl);
            Q_FOREACH(const KoColorSetEntry &entry, d->groups.value(groupName)) {

                // Only save non-builtin profiles.=
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

bool KoColorSet::loadKpl()
{
    QBuffer buf(&d->data);
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
        setName(e.attribute("name"));
        d->comment = e.attribute("comment");
        d->columns = e.attribute("columns").toInt();

        QDomElement c = e.firstChildElement("ColorSetEntry");
        while (!c.isNull()) {
            QString colorDepthId = c.attribute("bitdepth", Integer8BitsColorDepthID.id());
            KoColorSetEntry entry;


            entry.setColor(KoColor::fromXML(c.firstChildElement(), colorDepthId));
            entry.setName(c.attribute("name"));
            entry.setId(c.attribute("id"));
            entry.setSpotColor(c.attribute("spot", "false") == "true" ? true : false);
            d->colors << entry;

            c = c.nextSiblingElement("ColorSetEntry");

        }
        QDomElement g = e.firstChildElement("Group");
        while (!g.isNull()) {
            QString groupName = g.attribute("name");
            addGroup(groupName);
            QDomElement cg = g.firstChildElement("ColorSetEntry");
            while (!cg.isNull()) {
                QString colorDepthId = cg.attribute("bitdepth", Integer8BitsColorDepthID.id());
                KoColorSetEntry entry;


                entry.setColor(KoColor::fromXML(cg.firstChildElement(), colorDepthId));
                entry.setName(cg.attribute("name"));
                entry.setId(cg.attribute("id"));
                entry.setSpotColor(cg.attribute("spot", "false") == "true" ? true : false);
                add(entry, groupName);

                cg = cg.nextSiblingElement("ColorSetEntry");

            }
            g = g.nextSiblingElement("Group");
        }

    }


    buf.close();
    return true;
}

quint16 readShort(QIODevice *io) {
    quint16 val;
    quint64 read = io->read((char*)&val, 2);
    if (read != 2) return false;
    return qFromBigEndian(val);
}

bool KoColorSet::loadAco()
{
    QFileInfo info(filename());
    setName(info.baseName());

    QBuffer buf(&d->data);
    buf.open(QBuffer::ReadOnly);

    quint16 version = readShort(&buf);
    quint16 numColors = readShort(&buf);
    KoColorSetEntry e;

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
            warnPigment << "Unsupported colorspace in palette" << filename() << "(" << colorSpace << ")";
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
                    warnPigment << "Version 2 name block is the wrong size" << filename();
                }
            }
            v2 = readShort(&buf); //end marker also needs to be skipped.
            Q_UNUSED(v2);
        }
        if (!skip) {
            add(e);
        }
    }
    return true;
}

bool KoColorSet::loadSbz() {
    QBuffer buf(&d->data);
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

        dbgPigment << "XML palette: " << filename() << ", SwatchBooker format";

        QDomDocument doc;
        int errorLine, errorColumn;
        QString errorMessage;
        bool status = doc.setContent(ba, &errorMessage, &errorLine, &errorColumn);
        if (!status) {
            warnPigment << "Illegal XML palette:" << filename();
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
        setName(colorName);
        dbgPigment << "Processed name of palette:" << name();
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
        QHash<QString, KoColorSetEntry> materialsBook;
        QHash<QString, const KoColorSpace*> fileColorSpaces;

        // Color processing
        for(; !colorElement.isNull(); colorElement = colorElement.nextSiblingElement("color"))
        {
            KoColorSetEntry currentEntry;
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
                    add(materialsBook.value(id));
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
                        add(materialsBook.value(id), currentGroupName);
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
