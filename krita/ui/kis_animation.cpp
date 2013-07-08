/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_animation.h"
#include <krita_export.h>
#include <KoApplication.h>
#include <KoPart.h>
#include <QFile>
#include <QDomDocument>
#include <QByteArray>
#include <QString>

KisAnimation::KisAnimation(QObject *parent) : QStandardItemModel(parent)
{
}

void KisAnimation::setName(const QString &name){
    m_name = name;
}

QString KisAnimation::name() const{
    return m_name;
}

void KisAnimation::setAuthor(const QString &author){
    m_author = author;
}

QString KisAnimation::author() const{
    return m_author;
}

void KisAnimation::setDescription(const QString &description){
    m_description = description;
}

QString KisAnimation::description() const{
    return m_description;
}

void KisAnimation::setFps(int fps){
    m_fps = fps;
}

int KisAnimation::fps() const{
    return m_fps;
}

void KisAnimation::setTime(int time){
    m_time = time;
}

int KisAnimation::time() const{
    return m_time;
}

void KisAnimation::setColorSpace(const KoColorSpace *colorSpace){
    this->m_colorSpace = colorSpace;
}

const KoColorSpace *KisAnimation::colorSpace(){
    return m_colorSpace;
}

void KisAnimation::setWidth(qint32 w){
    this->m_width = w;
}

qint32 KisAnimation::width() const{
    return this->m_width;
}

void KisAnimation::setHeight(qint32 h){
    this->m_height = h;
}

qint32 KisAnimation::height() const{
    return this->m_height;
}

void KisAnimation::setResolution(double res){
    this->m_resolution = res;
}

double KisAnimation::resolution() const{
    return this->m_resolution;
}

void KisAnimation::setBgColor(KoColor bgColor){
    this->m_bgColor = bgColor;
}

KoColor KisAnimation::bgColor() const{
    return this->m_bgColor;
}

void KisAnimation::load(const QString &url){

}

void KisAnimation::save(const QString &url){

}
