/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com
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

#include <QDebug>
#include <QFile>
#include <qregexp.h>
#include <QStringList>
#include <QApplication>
#include <QTreeView>

#include "kis_gmic_parser.h"

#include <Parameter.h>
#include <Command.h>
#include <Filters.h>
#include <Category.h>
#include "kis_gmic_filter_model.h"


KisGmicParser::KisGmicParser(const QString& filePath):m_fileName(filePath)
{

}

KisGmicParser::~KisGmicParser()
{

}

bool KisGmicParser::matchesRegExp(const QRegExp& regExp,const QString& line)
{
    int indexOfMatch = regExp.indexIn(line);
    int matchedLength = regExp.matchedLength();
    return (indexOfMatch == 0) && (matchedLength > 0);
}

bool KisGmicParser::isCommand(const QString& line)
{
    return matchesRegExp(COMMAND_NAME_RX, line);
}

bool KisGmicParser::isParameter(const QString& line)
{
    return matchesRegExp(PARAMETER_RX,line);
}

bool KisGmicParser::isCategory(const QString& line)
{
    return matchesRegExp(CATEGORY_NAME_RX,line);
}

QString KisGmicParser::parseCategoryName(const QString& line)
{
    QString result = line;
    return result.remove(0, GIMP_COMMENT.size()).trimmed();
}

Component* KisGmicParser::createFilterTree()
{
    QFile file(m_fileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Can't open file " << m_fileName << file.errorString();
        return 0;
    }

    QTextStream in(&file);

    int folders = 0;
    m_status = PARSE_START;
    Category * rootCategory = new Category();
    rootCategory->setName("Filters");

    Command * command = 0;
    Category * category = rootCategory;
    int lineNum = 0;
    while(!in.atEnd()) {
        QString line = in.readLine();
        lineNum++;

        if (line.startsWith(GIMP_COMMENT))
        {
            if (isCategory(line))
            {
                QString categoryName = parseCategoryName(line);
                qDebug() << categoryName;

                if (categoryName.startsWith("_"))
                {
                    // root category
                    if (categoryName != "_")
                    {
                        category = new Category();
                        category->setName(categoryName.remove(0,1)); // remove _
                        rootCategory->add(category);
                    } else
                    {
                        // set current category to parent category
                        category = static_cast<Category*>(category->parent());
                    }
                }
                else
                {
                    // set current category as parent
                    Category * childCategory = new Category(category);
                    childCategory->setName(categoryName);
                    category->add(childCategory);
                    category = childCategory; // set current category to child
                }
            } else if (isCommand(line))
            {
                // qDebug() << "command" << line;
                command = new Command(category);
                command->processCommandName(line);
                category->add(command);


            } else if (isParameter(line))
            {
                // qDebug() << "Parameter" << line;
                if (command)
                {
                    command->processParameter(line);
                }else
                {
                    qDebug() << "No command for given parameter, invalid gmic definition file";
                }
            }else{
                // qDebug() << "IGNORING:" << line;
            }
        }
    }
    //command->debug();

    //rootCategory->print();
    file.close();

    return rootCategory;
}



int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KisGmicParser parser("gmic_def.gmic");
    Component * root = parser.createFilterTree();

    KisGmicFilterModel model(root);

    QTreeView view;
    view.setModel(&model);
    view.setWindowTitle(QObject::tr("Simple Tree Model"));
    view.show();

    return app.exec();
 }
