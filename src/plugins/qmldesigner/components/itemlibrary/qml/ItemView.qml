/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

import Qt 4.7

// view displaying an item library grid item

Item {
    id: itemView

    // public

    signal itemPressed()
    signal itemDragged()

    // internal

    ItemsViewStyle { id: style }

    // frame
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: style.gridLineLighter
    }
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: style.gridLineDarker
    }
    Rectangle {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: 1
        color: style.gridLineLighter
    }
    Rectangle {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: 1
        color: style.gridLineDarker
    }
    Rectangle {
        anchors.top:parent.top
        anchors.right:parent.right
        width:1
        height:1
        color: style.backgroundColor
    }
    Rectangle {
        anchors.bottom:parent.bottom
        anchors.left:parent.left
        width:1
        height:1
        color: style.backgroundColor
    }

    Image {
        id: itemIcon

        anchors.top: parent.top
        anchors.topMargin: style.cellVerticalMargin
        anchors.horizontalCenter: parent.horizontalCenter

        width: itemLibraryIconWidth  // to be set in Qml context
        height: itemLibraryIconHeight   // to be set in Qml context
        source: itemLibraryIconPath     // to be set by model
    }

    Text {
        id: text
        elide: Text.ElideMiddle
        anchors.top: itemIcon.bottom
        anchors.topMargin: style.cellVerticalSpacing
        anchors.left: parent.left
        anchors.leftMargin: style.cellHorizontalMargin
        anchors.right: parent.right
        anchors.rightMargin: style.cellHorizontalMargin
        width: style.textWidth
        height: style.textHeight

        verticalAlignment: "AlignVCenter"
        horizontalAlignment: "AlignHCenter"
        text: itemName  // to be set by model
        color: style.itemNameTextColor
    }

    MouseArea {
        id: mouseRegion
        anchors.fill: parent

        property bool reallyPressed: false
        property int pressedX
        property int pressedY

        onPressed: {
            reallyPressed = true
            pressedX = mouse.x
            pressedY = mouse.y
            itemPressed()
        }
        onPositionChanged: {
            if (reallyPressed &&
                (Math.abs(mouse.x - pressedX) +
                 Math.abs(mouse.y - pressedY)) > 3) {
                itemDragged()
                reallyPressed = false
            }
        }
        onReleased: reallyPressed = false
    }
}

