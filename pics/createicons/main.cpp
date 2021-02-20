/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <QFile>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QList>
#include <QDebug>
#include <QCommandLineParser>
#include <QCoreApplication>

QStringList findIconPaths(const QString &baseName, const QDir startDir)
{
    QStringList filters;
    filters << baseName + ".svg" << baseName + ".png";

    QFileInfoList icons = startDir.entryInfoList(filters, QDir::Files);
    QStringList results;
    Q_FOREACH (const QFileInfo &icon, icons) {
        results << icon.absoluteFilePath();
    }

    QStringList subdirs = startDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    Q_FOREACH (const QString &subdir, subdirs) {
        results << findIconPaths(baseName, QDir(startDir.absolutePath() + "/" + subdir));
    }
    return results;

}

int main(int argc, char *argv[])
{

    QCoreApplication app(argc, argv);
    app.setApplicationName("CreateIcons");
    QCommandLineParser parser;
    parser.setApplicationDescription("Copy all specified icons from a given theme and create a qrc file.\n\n"
                                     "If the word 'dark' exists in the theme directory name, the icons will be"
                                     "classified as dark, otherwise as light."
                                     );
    parser.addHelpOption();
    QCommandLineOption targetDirectoryOption(QStringList() << "p" << "pics",
                                             "Copy all icons into <directory>.",
                                             "directory");
    parser.addOption(targetDirectoryOption);
    parser.addPositionalArgument("iconsdir", "location of the icon theme");

    parser.process(app);

    const QStringList themes = parser.positionalArguments();
    QString dir = parser.value(targetDirectoryOption);
    if (dir.isEmpty()) {
        dir = ".";
    }

    QFile f(":/icons");
    f.open(QFile::ReadOnly);
    QByteArray ba = f.readAll();
    QList<QByteArray> iconNames = ba.split('\n');
    qDebug() << iconNames.size() << "icons will be retrieved";

    QString qrc("<!DOCTYPE RCC>\n"
                "<RCC version=\"1.0\">\n"
                "\t<qresource>\n");

    Q_FOREACH (const QByteArray &iconName, iconNames) {
        if (!iconName.isEmpty()) {
            Q_FOREACH (const QString &theme, themes) {
                QStringList iconPaths = findIconPaths(QString::fromLatin1(iconName), QDir(theme));
                if (iconPaths.isEmpty()) {
                    qDebug() << "Could not find" << iconName << "in theme" << theme;
                }
                else {
                    bool dark = theme.contains("dark");
                    Q_FOREACH (const QString &iconPath, iconPaths) {
                        QFileInfo fi(iconPath);
                        // Check whether the path contains a size
                        QStringList parts = fi.absolutePath().split('/');
                        QString newIconPath = dir + "/";

                        bool ok;
                        parts.last().toInt(&ok);
                        if (ok) {
                            newIconPath = newIconPath + parts.last() + "_";
                        }
                        if (dark) {
                            newIconPath = newIconPath + "dark_";
                        }
                        else {
                            newIconPath = newIconPath + "light_";
                        }
                        newIconPath = newIconPath + fi.fileName();
                        QFile::copy(iconPath, newIconPath);

                        qrc += "\t\t<file>" + QFileInfo(newIconPath).fileName() + "</file>\n";
                    }
                }
            }
        }
    }

    qrc += "\t</qresource>\n"
           "</RCC>";

    QFile qrcf(dir + "/icons.qrc");
    qrcf.open(QFile::WriteOnly);
    qrcf.write(qrc.toLatin1());
    qrcf.close();
}
