/*
 * This file is part of Krita
 *
 * Copyright (c) 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <FlakeDebug.h>
#include <KoStore.h>
#include <KoStoreDevice.h>
#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QTextDecoder>
#include <kis_assert.h>

#include "KisSeExprScript.h"

struct KisSeExprScript::Private {
    QString script;
    QByteArray data;
    bool dirtyPreset = false;
};

KisSeExprScript::KisSeExprScript(const QString &filename)
    : KoResource(filename)
    , d(new Private)
{
}

KisSeExprScript::KisSeExprScript(const QImage &image, const QString &script, const QString &name, const QString &folderName)
    : KoResource(QString())
    , d(new Private)
{
    setScript(script);
    setImage(image);
    setName(name);

    QFileInfo fileInfo(folderName + QDir::separator() + name + defaultFileExtension());

    int i = 1;
    while (fileInfo.exists()) {
        fileInfo.setFile(folderName + QDir::separator() + name + QString::number(i) + defaultFileExtension());
        i++;
    }

    setFilename(fileInfo.filePath());
}

KisSeExprScript::~KisSeExprScript()
{
    delete d;
}

bool KisSeExprScript::load()
{
    QFile file(filename());
    if (file.size() == 0)
        return false;

    if (!file.open(QIODevice::ReadOnly)) {
        warnFlake << "Can't open file " << filename();
        return false;
    }
    bool result = loadFromDevice(&file);
    file.close();

    return result;
}

bool KisSeExprScript::loadFromDevice(QIODevice *dev)
{
    if (!dev->isOpen())
        dev->open(QIODevice::ReadOnly);

    d->data = dev->readAll();

    // TODO: test
    KIS_ASSERT_RECOVER_RETURN_VALUE(d->data.size() != 0, false);

    if (filename().isNull()) {
        warnFlake << "Cannot load SeExpr script" << name() << ", there is no filename set";
        return false;
    }

    if (d->data.isNull()) {
        QFile file(filename());
        if (file.size() == 0) {
            warnFlake << "Cannot load SeExpr script" << name() << "there is no data available";
            return false;
        }

        file.open(QIODevice::ReadOnly);
        d->data = file.readAll();
        file.close();
    }

    QBuffer buf(&d->data);
    buf.open(QBuffer::ReadOnly);

    QScopedPointer<KoStore> store(KoStore::createStore(&buf, KoStore::Read, "application/x-krita-seexpr-script", KoStore::Zip));
    if (!store || store->bad())
        return false;

    bool storeOpened = store->open("script.se");
    if (!storeOpened) {
        return false;
    }

    d->script = QString(store->read(store->size()));
    store->close();

    if (store->open("preview.png")) {
        KoStoreDevice previewDev(store.data());
        previewDev.open(QIODevice::ReadOnly);

        QImage preview = QImage();
        preview.load(&previewDev, "PNG");
        setImage(preview);

        (void)store->close();
    }

    buf.close();

    QFileInfo fileinfo(filename());
    // The name of a SeExpr script is its basename
    // KoResourceServer uses its filename -- amyspark
    setName(fileinfo.baseName());
    setValid(true);
    setDirty(false);

    return true;
}

bool KisSeExprScript::save()
{
    QFile file(filename());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    saveToDevice(&file);
    file.close();

    return true;
}

bool KisSeExprScript::saveToDevice(QIODevice *dev) const
{
    KoStore *store(KoStore::createStore(dev, KoStore::Write, "application/x-krita-seexpr-script", KoStore::Zip));
    if (!store || store->bad())
        return false;

    if (!store->open("script.se")) {
        return false;
    }

    KoStoreDevice storeDev(store);
    storeDev.open(QIODevice::WriteOnly);

    storeDev.write(d->script.toUtf8());

    if (!store->close()) {
        return false;
    }

    if (!store->open("preview.png")) {
        return false;
    }

    KoStoreDevice previewDev(store);
    previewDev.open(QIODevice::WriteOnly);

    image().save(&previewDev, "PNG");
    if (!store->close()) {
        return false;
    }

    return store->finalize();
}

QString KisSeExprScript::defaultFileExtension() const
{
    return QString(".kse");
}

QString KisSeExprScript::script() const
{
    return d->script;
}

void KisSeExprScript::setScript(const QString &script)
{
    d->script = script;
}

KisSeExprScript *KisSeExprScript::clone() const
{
    KisSeExprScript *scr = new KisSeExprScript(filename());
    scr->setScript(script());
    scr->setImage(image());
    scr->setName(name());
    scr->setValid(valid());
    scr->setDirty(isDirty());

    return scr;
}

void KisSeExprScript::setDirty(bool value)
{
    d->dirtyPreset = value;
}

bool KisSeExprScript::isDirty() const
{
    return d->dirtyPreset;
}
