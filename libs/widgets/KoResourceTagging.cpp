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


KoResourceTagging::KoResourceTagging()
{

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

    m_tagRepo.insert( resource->filename(), tag );

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
