/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
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

import QtQuick 1.1
import org.krita.sketch 1.0

PageStack {
    id: base;
    clip: true;

    initialPage: Page {
        ListView {
            anchors.fill: parent;
            id: listView;

            delegate: ListItem {
                title: model.title;
                description: model.pubDate;

                onClicked: {
                    base.push( detailsPage,
                              { title: model.title,
                                description: model.description,
                                pubDate: model.pubDate});
                }

                Label {
                    anchors.right: parent.right;
                    anchors.rightMargin: Constants.GridWidth * 0.5;
                    anchors.verticalCenter: parent.verticalCenter;

                    text: "More >";
                    color: Constants.Theme.SecondaryTextColor;

                    font.italic: true;
                }
            }

            model: {
                if (aggregatedFeedsModel.articleCount > 0)
                    return aggregatedFeedsModel
                else {
                    return fallbackNewsModel
                }
            }

            ScrollDecorator { }
        }
    }

    ListModel {
        id: fallbackNewsModel;
        ListElement {
            title: "Welcome to Krita Sketch 1.0";
            blogName: "The Krita Team";
            description: "<div>Krita Sketch: Painting for Pro's on the Go</div> <p>With Krita Sketch you have all the power of Krita Desktop under your fingers. Paint with a stylus, rub with your fingers, zoom, pan, erase, select, filter and add layers. Sketch, speedpaint, polish and publish! Have fun and share!</p>";
            link: "";
            pubDate: "Today!";
        }
    }

    Component {
        id: detailsPage;

        Page {

            property string title;
            property string pubDate;
            property string description;

            Flickable {
                anchors.fill: parent;
                anchors.leftMargin: Constants.DefaultMargin;
                anchors.rightMargin: Constants.DefaultMargin;
                anchors.bottomMargin: Constants.DefaultMargin;

                contentWidth: width;
                contentHeight: contents.height;

                Column {
                    id: contents;
                    width: parent.width;

                    Item {
                        width: parent.width;
                        height: Constants.GridHeight;

                        Label {
                            anchors {
                                top: parent.top;
                                topMargin: Constants.DefaultMargin;
                            }

                            text: title
                            verticalAlignment: Text.AlignTop;
                        }

                        Label {
                            anchors {
                                bottom: parent.bottom;
                                bottomMargin: Constants.DefaultMargin;
                            }

                            text: pubDate;
                            font.pixelSize: Constants.SmallFontSize;
                            color: Constants.Theme.SecondaryTextColor;
                            verticalAlignment: Text.AlignBottom;
                        }


                    }

                    Label {
                        width: parent.width;
                        height: paintedHeight;

                        elide: Text.ElideNone;
                        wrapMode: Text.WordWrap;
                        horizontalAlignment: Text.AlignJustify;

                        text: description;
                    }

                    Label {
                        text: "< Back";
                        font.pixelSize: Constants.SmallFontSize;
                        color: Constants.Theme.SecondaryTextColor;
                    }
                }

                MouseArea {
                    anchors.fill: parent;
                    onClicked: pageStack.pop();
                }
            }
        }
    }

}
