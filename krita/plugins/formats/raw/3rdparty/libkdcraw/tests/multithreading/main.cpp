/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date  : 2011-12-28
 * @brief : test for implementation of threadWeaver api
 *
 * @author Copyright (C) 2011-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2014 by Veaceslav Munteanu
 *         <a href="mailto:veaceslav dot munteanu90 at gmail dot com">veaceslav dot munteanu90 at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Qt includes

#include <QString>
#include <QStringList>
#include <QApplication>
#include <QStandardPaths>
#include <QUrl>
#include <QDebug>
#include <QFileDialog>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "rawfiles.h"
#include "processordlg.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QList<QUrl> list;

    if (argc <= 1)
    {
        QString filter = i18n("Raw Files") + QString::fromLatin1(" (%1)").arg(QString::fromLatin1(raw_file_extentions));
        qDebug() << filter;

        QStringList files = QFileDialog::getOpenFileNames(0, i18n("Select RAW files to process"),
                                                         QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first(),
                                                         filter);

        foreach(QString f, files)
            list.append(QUrl::fromLocalFile(f));
    }
    else
    {
        for (int i = 1 ; i < argc ; i++)
            list.append(QUrl::fromLocalFile(QString::fromLocal8Bit(argv[i])));
    }

    if (!list.isEmpty())
    {

        ProcessorDlg* const dlg = new ProcessorDlg(list);
        dlg->show();
        app.exec();
    }

    return 0;
}
