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

void KisAnimation::load(const QString &url){

}

void KisAnimation::save(const QString &url){

}
