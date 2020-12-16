/*
   This file is part of the KDE project
   SPDX-FileCopyrightText: 2000 Werner Trobin <trobin@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIS_TEMPLATE_H
#define KIS_TEMPLATE_H

#include <QString>
#include <QPixmap>

/** @internal */
class KisTemplate
{

public:
    explicit KisTemplate(const QString &name,
                        const QString &description = QString(),
                        const QString &file = QString(),
                        const QString &picture = QString(),
                        const QString &fileName = QString(),
                        const QString &_measureSystem = QString(),
                        bool hidden = false, bool touched = false);
    ~KisTemplate() {}

    QString name() const {
        return m_name;
    }
    QString description() const {
        return m_descr;
    }
    QString file() const {
        return m_file;
    }
    QString picture() const {
        return m_picture;
    }
    QString fileName() const {
        return m_fileName;
    }
    const QPixmap &loadPicture();

    bool isHidden() const {
        return m_hidden;
    }
    void setHidden(bool hidden = true) {
        m_hidden = hidden; m_touched = true;
    }

    bool touched() const {
        return m_touched;
    }

    QString measureSystem() const {
        return m_measureSystem;
    }
    void setMeasureSystem(const QString& system) {
        m_measureSystem = system;
    }

private:
    QString m_name, m_descr, m_file, m_picture, m_fileName;
    bool m_hidden;
    mutable bool m_touched;
    bool m_cached;
    QPixmap m_pixmap;
    QString m_measureSystem;
};


#endif
