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

// the coloured selector of the items view

Item {
    id: selector

    ItemsViewStyle { id: style }

    Rectangle {
        anchors.fill: parent
        color: highlightColor
        clip:true
        Rectangle {
            width:parent.width-1
            x:1
            y:-parent.height/2
            height:parent.height
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.lighter(highlightColor) }
                GradientStop { position: 1.0; color: highlightColor }
            }
        }

        Rectangle {
            width:parent.width-1
            x:1
            y:parent.height/2
            height:parent.height
            gradient: Gradient {
                GradientStop { position: 0.0; color: highlightColor }
                GradientStop { position: 1.0; color: Qt.darker(highlightColor) }
            }
        }
    }
    Rectangle {
        anchors.right:parent.right
        anchors.left:parent.left
        anchors.top:parent.top
        anchors.topMargin:1
        anchors.rightMargin:2
        height:1
        color:Qt.lighter(highlightColor)
    }
    Rectangle {
        anchors.right:parent.right
        anchors.left:parent.left
        anchors.bottom:parent.bottom
        anchors.bottomMargin:1
        anchors.leftMargin:2
        height:1
        color:Qt.darker(highlightColor)
    }
    Rectangle {
        anchors.left:parent.left
        anchors.top:parent.top
        anchors.bottom:parent.bottom
        anchors.leftMargin:1
        anchors.bottomMargin:2
        width:1
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.lighter(highlightColor) }
            GradientStop { position: 1.0; color: highlightColor }
        }
    }
    Rectangle {
        anchors.right:parent.right
        anchors.top:parent.top
        anchors.bottom:parent.bottom
        anchors.rightMargin:1
        anchors.topMargin:2
        width:1
        gradient: Gradient {
            GradientStop { position: 0.0; color: highlightColor }
            GradientStop { position: 1.0; color: Qt.darker(highlightColor) }
        }
    }
}
