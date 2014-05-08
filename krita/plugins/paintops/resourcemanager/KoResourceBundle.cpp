/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoResourceBundle.h"
#include "KoResourceBundleManager.h"
#include "KoXmlResourceBundleManifest.h"
#include "KoXmlResourceBundleMeta.h"

#include <kglobal.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>

#include <QProcessEnvironment>
#include <QDate>
#include <QDir>

#include <iostream>
using namespace std;


//TODO Voir s'il ne vaut pas mieux faire un constructeur avec un xmlmeta plutot qu'un setmeta (cf control createPack)
KoResourceBundle::KoResourceBundle(QString const& bundlePath)
    : KoResource(bundlePath)
{
    m_manager = new KoResourceBundleManager(bundlePath.section('/', 0, bundlePath.count('/') - 2));
    setName(bundlePath.section('/', bundlePath.count('/')));
}

KoResourceBundle::~KoResourceBundle()
{
    delete m_manager;
    delete m_meta;
    delete m_manifest;
}

QString KoResourceBundle::defaultFileExtension() const
{
    return QString(".zip");
}

QImage KoResourceBundle::image() const
{
    return m_thumbnail;
}

bool KoResourceBundle::load()
{
    m_manager->setReadPack(filename());
    if (m_manager->bad()) {
        m_manifest = new KoXmlResourceBundleManifest();
        m_meta = new KoXmlResourceBundleMeta();
        m_installed = false;
    } else {
        //TODO Vérifier si on peut éviter de recréer manifest et meta à chaque load
        //A optimiser si possible
        m_manifest = new KoXmlResourceBundleManifest(m_manager->getFile("manifest.xml"));
        m_meta = new KoXmlResourceBundleMeta(m_manager->getFile("meta.xml"));
        m_thumbnail.load(m_manager->getFile("thumbnail.jpg"), "JPG");
        m_manager->close();
        m_installed = m_manifest->isInstalled();
        setValid(true);
    }
    return true;
}

bool KoResourceBundle::save()
{
    addMeta("updated", QDate::currentDate().toString("dd/MM/yyyy"));
    m_manifest->checkSort();
    m_meta->checkSort();

    if (m_manager->bad()) {
        //meta->addTags(manifest->getTagsList());
        m_manager->createPack(m_manifest, m_meta, m_thumbnail, true);
    } else {
        m_manager->createPack(m_manifest, m_meta, m_thumbnail);
    }

    if (!valid()) {
        setValid(true);
    }

    return load();
}

//TODO getFilesToExtract à vérifier
//TODO exportTags à vérifier
void KoResourceBundle::install()
{
    //load();
    if (!m_manager->bad()) {
        m_manager->extractKFiles(m_manifest->getFilesToExtract());
        m_manifest->exportTags();
        m_installed = true;
        m_manifest->install();
        save();
    }
}

void KoResourceBundle::uninstall()
{
    if (!m_installed)
        return;

    QString dirPath = this->m_manager->getKritaPath();
    QList<QString> directoryList = m_manifest->getDirList();
    QString shortPackName = m_meta->getPackName();

    for (int i = 0; i < directoryList.size(); i++) {
        if (!KoResourceBundleManager::removeDir(dirPath + directoryList.at(i) + QString("/") + shortPackName)) {
            cerr << "Error : Couldn't delete folder : " << qPrintable(dirPath) << endl;
        }
    }

    m_installed = false;
    m_manifest->uninstall();
    save();
}

void KoResourceBundle::addMeta(QString type, QString value)
{
    if (type == "created") {
        setValid(true);
    }
    m_meta->addTag(type, value);
}

void KoResourceBundle::setMeta(KoXmlResourceBundleMeta* newMeta)
{
    m_meta = newMeta;
}

//TODO Voir s'il faut aussi rajouter les tags dans le meta
void KoResourceBundle::addFile(QString fileType, QString filePath, QStringList fileTagList)
{
    m_manifest->addManiTag(fileType, filePath, fileTagList);
    m_meta->addTags(fileTagList);
}

//On rappelle que les tags d'un bundle ne sont stockés que dans le meta
//Les tags du manifest sont ajoutés au fur et à mesure de l'ajout des fichiers
QList<QString> KoResourceBundle::getTagsList()
{
    return m_meta->getTagsList();
}


void KoResourceBundle::removeFile(QString fileName)
{
    QList<QString> list = m_manifest->removeFile(fileName);

    for (int i = 0; i < list.size(); i++) {
        m_meta->removeFirstTag("tag", list.at(i));
    }
}

void KoResourceBundle::addResourceDirs()
{
    QList<QString> listeType = m_manifest->getDirList();
    for (int i = 0; i < listeType.size(); i++) {
        KGlobal::mainComponent().dirs()->addResourceDir(listeType.at(i).toLatin1().data(), this->m_manager->getKritaPath() + listeType.at(i) + "/" + this->name());
    }
}

bool KoResourceBundle::isInstalled()
{
    return m_installed;
}

void KoResourceBundle::rename(QString filename, QString name)
{
    QString oldName = m_meta->getPackName();
    QString shortName = name.section('.', 0, 0);

    addMeta("filename", filename);
    addMeta("name", shortName);
    m_manifest->rename(shortName);

    if (isInstalled()) {
        QList<QString> directoryList = m_manifest->getDirList();
        QString dirPath;
        QDir dir;
        for (int i = 0; i < directoryList.size(); i++) {
            dirPath = m_manager->getKritaPath();
            dirPath.append(directoryList.at(i)).append("/");
            dir.rename(dirPath + oldName, dirPath + shortName);
        }
    }
    save();
}

void KoResourceBundle::removeTag(QString tagName)
{
    m_meta->removeFirstTag("tag", tagName);
}

void KoResourceBundle::setThumbnail(QString filename)
{
    m_thumbnail = QImage(filename);
    save();
}

QByteArray KoResourceBundle::generateMD5() const
{
    Q_ASSERT("Implement this!");
    return QByteArray();
}

QString KoResourceBundle::getAuthor()
{
    return m_meta->getValue("author");
}

QString KoResourceBundle::getLicense()
{
    return m_meta->getValue("license");
}

QString KoResourceBundle::getWebSite()
{
    return m_meta->getValue("website");
}

QString KoResourceBundle::getCreated()
{
    return m_meta->getValue("created");
}

QString KoResourceBundle::getUpdated()
{
    return m_meta->getValue("updated");
}


