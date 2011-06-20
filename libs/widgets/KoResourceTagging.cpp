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

#include "KoResourceServerAdapter.h"

KoResourceTagging::KoResourceTagging(KoResourceModel* model)
{
    m_model = model;
    m_tagsXMLFile = KStandardDirs::locateLocal("data", "krita/tags.xml");
    readXMLFile();
}

KoResourceTagging::~KoResourceTagging()
{
    writeXMLFile();
}

QStringList KoResourceTagging::getAssignedTagsList( KoResource* resource )
{
    return m_tagRepo.values(resource->filename());
}

QStringList KoResourceTagging::getTagNamesList()
{
    return m_tagList.uniqueKeys();
}

void KoResourceTagging::addTag( KoResource* resource,const QString& tag)
{
    if( m_tagRepo.contains ( resource->filename(), tag ) ) {
        return;
    }

    addTag( resource->filename(), tag );
}

void KoResourceTagging::addTag(const QString& fileName,const QString& tag)
{
    m_tagRepo.insert( fileName, tag );

    if(m_tagList.contains(tag))
    {
        int val = m_tagList.value(tag);
        m_tagList.insert(tag, ++val);
    }
    else
    {
        m_tagList.insert(tag,1);
    }
}

void KoResourceTagging::delTag( KoResource* resource,const QString& tag)
{
    if( ! m_tagRepo.contains ( resource->filename(), tag ) ) {
        return;
    }

    m_tagRepo.remove( resource->filename(), tag);

    int val = m_tagList.value(tag);

    m_tagList.remove(tag);

    if( val !=0 ){
        m_tagList.insert(tag, --val);
    }
}

QStringList KoResourceTagging::searchTag(const QString& lineEditText)
{
    QStringList tagsList = lineEditText.split(", ");
    if(tagsList.contains("")) {
       tagsList.removeAll("");
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
        return resultKeysList;
    }
    return keysList;
}

void KoResourceTagging::writeXMLFile()
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
                   QStringList tags = m_tagRepo.values(resourceEl.attribute("identifier"));
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
                    if( isServerResource(resourceEl.attribute("identifier"))) {
                       root.removeChild(resourceNodesList.at(i--));
                   }
               }
           }
      }
   }

   foreach(QString resourceName, resourceNames ) {

       QDomElement resourceEl = doc.createElement("resource");
       resourceEl.setAttribute("identifier",resourceName);

       QStringList tags = m_tagRepo.values(resourceName);
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

void KoResourceTagging::readXMLFile()
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
            if (isServerResource(resourceEl.attribute("identifier"))) {
                QDomNodeList tagNodesList = resourceNodesList.at(i).childNodes();
                for(int j = 0; j < tagNodesList.count() ; j++) {
                    QDomElement tagEl = tagNodesList.at(j).toElement();
                    addTag(resourceEl.attribute("identifier"), tagEl.text());
                }
            }
        }
    }


}

bool KoResourceTagging::isServerResource(QString resourceName)
{
    bool removeChild = false;
    QStringList extensionsList = m_model->resourceServerAdapter()->extensions().split(":");
    foreach (QString extension, extensionsList) {
        if(resourceName.contains(extension.remove("*"))) {
            removeChild = true;
            break;
        }
    }

    if(!removeChild && !resourceName.contains(".")) {
        removeChild = true;
    }

    return removeChild;
}
