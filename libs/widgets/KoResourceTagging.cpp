/*  This file is part of the KDE project

    Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
    Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "KoResourceTagging.h"

#include <QStringList>
#include <kstandarddirs.h>
#include <QFile>
#include <QDir>

#ifdef NEPOMUK
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Variant>
#include <Nepomuk/Tag>
#include <Nepomuk/File>

#include <Soprano/Model>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/NAO>

#include <kurl.h>
#endif

KoResourceTagging::KoResourceTagging(const QString& extensions)
{
    m_serverExtensions = extensions;
    m_tagsXMLFile = KStandardDirs::locateLocal("data", "krita/tags.xml");
    m_config = KConfigGroup( KGlobal::config(), "resource tagging" );
    m_nepomukOn = m_config.readEntry("nepomuk_usage_for_resource_tagging", false);

#ifndef NEPOMUK
    m_nepomukOn = false;
#endif

    if(m_nepomukOn) {
#ifdef NEPOMUK
      updateTagRepoFromNepomuk();
#endif
    }
    else {
        readXMLFile();
    }
}

KoResourceTagging::~KoResourceTagging()
{
    if(!m_nepomukOn) {
        writeXMLFile();
     }
}

QStringList KoResourceTagging::getAssignedTagsList( KoResource* resource )
{
    return m_tagRepo.values(getAdjustedFileName(resource->filename()));
}

QStringList KoResourceTagging::getTagNamesList()
{
    return m_tagList.uniqueKeys();
}

void KoResourceTagging::addTag( KoResource* resource,const QString& tag)
{
    QString fileName = getAdjustedFileName (resource->filename());

    addTag( fileName, tag );
}

void KoResourceTagging::addTag(const QString& fileName,const QString& tag)
{
    if( m_tagRepo.contains ( fileName, tag ) ) {
        return;
    }

    m_tagRepo.insert( fileName, tag );

#ifdef NEPOMUK
    if(m_nepomukOn) {
        addNepomukTag(fileName,tag);
    }
#endif
    if(m_tagList.contains(tag))
    {
        int val = m_tagList.value(tag);
        m_tagList.remove(tag);
        m_tagList.insert(tag, ++val);
    }
    else
    {
        m_tagList.insert(tag,1);
    }
}

void KoResourceTagging::delTag( KoResource* resource,const QString& tag)
{
    QString fileName = getAdjustedFileName(resource->filename());

    if( ! m_tagRepo.contains ( fileName, tag ) ) {
        return;
    }

    m_tagRepo.remove( fileName, tag);
#ifdef NEPOMUK
    if(m_nepomukOn) {
        delNepomukTag(fileName,tag);
    }
#endif
    int val = m_tagList.value(tag);

    m_tagList.remove(tag);

    if( val !=0 && val != 1){
        m_tagList.insert(tag, --val);
    }
}

QStringList KoResourceTagging::searchTag(const QString& lineEditText)
{
    QStringList tagsList = lineEditText.split(QRegExp("[,]\\s*"), QString::SkipEmptyParts);
    if (tagsList.isEmpty()) {
        return QStringList();
    }

    QStringList keysList = m_tagRepo.keys(tagsList.takeFirst());

    if(tagsList.count() >= 1) {
        QStringList resultKeysList;
        bool tagPresence;
        foreach(QString key, keysList) {
            tagPresence=true;
            foreach(QString tag, tagsList) {
                if(!m_tagRepo.contains(key,tag)) {
                    tagPresence=false;
                    break;
                }
            }
            if(tagPresence) {
                resultKeysList.append(key);
            }
        }
        return removeAdjustedFileNames(resultKeysList);
    }
    return removeAdjustedFileNames(keysList);
}

void KoResourceTagging::writeXMLFile(bool serverIdentity)
{
   QFile f(m_tagsXMLFile);
   bool fileExists = f.exists();

   if (!f.open(QIODevice::ReadWrite | QIODevice::Text)) {
        kWarning() << "Cannot write meta information to '" << m_tagsXMLFile << "'.";
        return;
   }
   QDomDocument doc;
   QDomElement root;

   if (!fileExists) {
       QDomDocument docTemp("tags");
       doc = docTemp;
       doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
       root = doc.createElement("tags");
       doc.appendChild(root);
   }
   else {
       if (!doc.setContent(&f)) {
           kWarning() << "The file could not be parsed.";
           return;
       }

       root = doc.documentElement();
       if (root.tagName() != "tags") {
           kWarning() << "The file doesn't seem to be of interest.";
           return;
       }
   }

   QStringList resourceNames = m_tagRepo.uniqueKeys();

   resourceNames.replaceInStrings(QDir::homePath(),QString("~"));

   if(fileExists) {
       QDomNodeList resourceNodesList = root.childNodes();
       /// resource are checked and added or removed according to need.
       for(int i = 0; i < resourceNodesList.count() ; i++) {
           QDomElement resourceEl = resourceNodesList.at(i).toElement();
           if(resourceEl.tagName() == "resource") {
               if (resourceNames.contains(resourceEl.attribute("identifier"))) {
                   resourceNames.removeAll(resourceEl.attribute("identifier"));
                   /// Tags are checked for a resource and added or removed according to need.
                   QDomNodeList tagNodesList = resourceNodesList.at(i).childNodes();
                   QStringList tags = m_tagRepo.values((resourceEl.attribute("identifier")).replace(QString("~"),QDir::homePath()));
                   for(int j = 0; j < tagNodesList.count() ; j++) {
                       QDomElement tagEl = tagNodesList.at(j).toElement();
                       if(tags.contains(tagEl.text())) {
                           tags.removeAll(tagEl.text());
                       }
                       else {
                           resourceNodesList.at(i).removeChild(tagNodesList.at(j--));
                       }
                   }
                   foreach(QString tag, tags) {
                       QDomElement newTagEl = doc.createElement("tag");
                       QDomText tagNameText = doc.createTextNode(tag);
                       newTagEl.appendChild(tagNameText);
                       resourceNodesList.at(i).appendChild(newTagEl);
                   }
               }
               else {
                    if( isServerResource((resourceEl.attribute("identifier")).replace(QString("~"),QDir::homePath())) || !serverIdentity) {
                       root.removeChild(resourceNodesList.at(i--));
                   }
               }
           }
      }
   }

   foreach(QString resourceName, resourceNames ) {

       QDomElement resourceEl = doc.createElement("resource");
       resourceEl.setAttribute("identifier",resourceName);

       QStringList tags = m_tagRepo.values(resourceName.replace(QString("~"),QDir::homePath()));
       foreach (QString tag, tags) {
           QDomElement tagEl = doc.createElement("tag");
           QDomText tagNameText = doc.createTextNode(tag);
           tagEl.appendChild(tagNameText);
           resourceEl.appendChild(tagEl);
       }
       root.appendChild(resourceEl);
   }

   f.remove();
   if(!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
       kWarning() << "Cannot write meta information to '" << m_tagsXMLFile << "'.";
   }
   QTextStream metastream(&f);
   metastream << doc.toByteArray();

   f.close();

}

void KoResourceTagging::readXMLFile(bool serverIdentity)
{
    QFile f(m_tagsXMLFile);
    if (!f.open(QIODevice::ReadOnly)) {
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&f)) {
        kWarning() << "The file could not be parsed.";
        return;
    }

    QDomElement root = doc.documentElement();
    if (root.tagName() != "tags") {
        kWarning() << "The file doesn't seem to be of interest.";
        return;
    }

    QDomNodeList resourceNodesList = root.childNodes();

    for(int i=0; i< resourceNodesList.count(); i++) {
        QDomElement resourceEl = resourceNodesList.at(i).toElement();
        if(resourceEl.tagName() == "resource") {
            QString resourceName = resourceEl.attribute("identifier");
            resourceName.replace(QString("~"),QDir::homePath());
            if (isServerResource(resourceName) || !serverIdentity) {
                QDomNodeList tagNodesList = resourceNodesList.at(i).childNodes();
                for(int j = 0; j < tagNodesList.count() ; j++) {
                    QDomElement tagEl = tagNodesList.at(j).toElement();
                    addTag(resourceName, tagEl.text());
                }
            }
        }
    }


}

bool KoResourceTagging::isServerResource(QString resourceName)
{
    bool removeChild = false;
    QStringList extensionsList = m_serverExtensions.split(':');
    foreach (QString extension, extensionsList) {
        if(resourceName.contains(extension.remove('*'))) {
            removeChild = true;
            break;
        }
    }
    return removeChild;
}

QString KoResourceTagging::getAdjustedFileName(QString fileName)
{
    if(!isServerResource(fileName)) {
        return fileName + "-krita" + m_serverExtensions.split(':').takeFirst().remove('*');
    }
    return fileName;
}

QStringList KoResourceTagging::removeAdjustedFileNames(QStringList fileNamesList)
{
    foreach(QString fileName, fileNamesList) {
        if(fileName.contains("-krita")) {
            fileNamesList.append(fileName.split("-krita").takeFirst());
            fileNamesList.removeAll(fileName);
        }
    }
    return fileNamesList;
}




/*
 * Nepomuk coding part
 *
 */

#ifdef NEPOMUK
void KoResourceTagging::writeNepomukRepo(bool serverIdentity)
{
    QStringList resourceFileNames = m_tagRepo.uniqueKeys();

    QList<Nepomuk::Resource> resourceList = readNepomukRepo();

    foreach(Nepomuk::Resource resource, resourceList) {
        QString resourceFileOld = resource.genericLabel();
        if(resourceFileNames.contains(resourceFileOld)) {
            resourceFileNames.removeAll(resourceFileOld);

            QStringList tagNameListNew = m_tagRepo.values(resourceFileOld);

            QList<Nepomuk::Tag> tagListOld = resource.tags();

            foreach(Nepomuk::Tag tag, tagListOld) {
                QString tagName =  tag.genericLabel();

                 if(tagNameListNew.contains(tagName)) {
                     tagNameListNew.removeAll(tagName);
                 }
                 else {
                     delNepomukTag(resource.genericLabel(),tagName);
                 }
            }
            foreach(QString tagName, tagNameListNew) {
                addNepomukTag(resourceFileOld,tagName);
            }
        }
        else {
            if(isServerResource(resourceFileOld) || !serverIdentity) {
                resource.remove();
            }
        }
    }

    foreach(QString resourceFileName, resourceFileNames) {

        QStringList tagNameListNew = m_tagRepo.values(resourceFileName);

        foreach(QString tagName, tagNameListNew) {
            addNepomukTag(resourceFileName,tagName);
        }
    }
}

QList<Nepomuk::Resource> KoResourceTagging::readNepomukRepo()
{
    Soprano::Model* model = Nepomuk::ResourceManager::instance()->mainModel();

    QList<Nepomuk::Resource> resourceList;

    QString query
       = QString("select distinct ?r where { ?r %1 ?p . } ")
         .arg( Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::hasTag()) );

   Soprano::QueryResultIterator it
       = model->executeQuery( query,Soprano::Query::QueryLanguageSparql );
    while( it.next() ) {
       resourceList << Nepomuk::Resource( it.binding("r").uri() );
    }
    return resourceList;
}

void KoResourceTagging::updateTagRepoFromNepomuk(bool serverIdentity)
{
    QList<Nepomuk::Resource> resourceList = readNepomukRepo();
    foreach(Nepomuk::Resource resource, resourceList) {

        QString resourceFileName = correctedNepomukFileName(resource.toFile().url().path());

        if(isServerResource(resourceFileName) || !serverIdentity) {
            QList<Nepomuk::Tag> tagList = resource.tags();
            foreach(Nepomuk::Tag tagName, tagList) {
                 addTag(resourceFileName, tagName.genericLabel());
            }
        }
    }
 }

void KoResourceTagging::addNepomukTag(const QString &fileName, const QString &tagNew)
{
    KUrl qurl(adjustedNepomukFileName(fileName));
    Nepomuk::File resource(qurl);
    Nepomuk::Tag nepomukTag( tagNew );
    resource.addTag(nepomukTag);
}

void KoResourceTagging::delNepomukTag(const QString &fileName, const QString &tagNew)
{
    QUrl qurl(adjustedNepomukFileName(fileName));
    Nepomuk::Resource res(qurl);
    Nepomuk::Tag nepomukTag( tagNew );
    Nepomuk::Variant tagValue(nepomukTag);
    res.removeProperty(res.tagUri(),tagValue);

    if(res.tags().count() == 0) {
        res.remove();
    }

    if(nepomukTag.tagOf().count() == 0) {
        nepomukTag.remove();
    }
}

QString KoResourceTagging::adjustedNepomukFileName(QString fileName)
{
    if(fileName.contains(" ")) {
        fileName.replace(" ","_k_");
    }
    return fileName;
}

QString KoResourceTagging::correctedNepomukFileName(QString fileName)
{
    if(fileName.contains("_k_")) {
        fileName.replace("_k_"," ");
    }
    return fileName;
}

void KoResourceTagging::updateNepomukXML(bool nepomukOn)
{
    if(nepomukOn) {
        m_tagRepo.clear();
        m_tagList.clear();
        readXMLFile(false);
        writeNepomukRepo(false);
    }
    else {
        m_tagRepo.clear();
        m_tagList.clear();
        updateTagRepoFromNepomuk(false);
        writeXMLFile(false);
    }

    m_config.writeEntry("nepomuk_usage_for_resource_tagging", QVariant(m_nepomukOn));
    m_config.sync();
}
#endif

void KoResourceTagging::setNepomukBool(bool nepomukOn)
{
    m_nepomukOn = nepomukOn;
}
