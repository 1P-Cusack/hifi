//
//  Tree.qml
//
//  Created by David Rowe on 17 Feb 2016
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQml.Models 2.2
import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import "../styles-uit"

TreeView {
    id: treeView

    signal selectedFile(var curSelectionIndex, var prevSelectionIndex );

    property var treeModel: ListModel { }
    property bool centerHeaderText: false
    property int colorScheme: hifi.colorSchemes.light
    readonly property bool isLightColorScheme: colorScheme == hifi.colorSchemes.light

    property var modifyEl: function(index, data) { return false; }

    property var branchModelIndices: [ ]

    model: treeModel
    selection: ItemSelectionModel {
        id: selectionModel
        model: treeModel
    }

    anchors { left: parent.left; right: parent.right }
    
    headerVisible: false

    // Use rectangle to draw border with rounded corners.
    frameVisible: false
    Rectangle {
        color: "#00000000"
        anchors.fill: parent
        radius: hifi.dimensions.borderRadius
        border.color: isLightColorScheme ? hifi.colors.lightGrayText : hifi.colors.baseGrayHighlight
        border.width: 2
        anchors.margins: -2
    }
    anchors.margins: 2  // Shrink TreeView to lie within border.

    backgroundVisible: true

    horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
    verticalScrollBarPolicy: Qt.ScrollBarAsNeeded

    style: TreeViewStyle {
        // Needed in order for rows to keep displaying rows after end of table entries.
        backgroundColor: parent.isLightColorScheme ? hifi.colors.tableRowLightEven : hifi.colors.tableRowDarkEven
        alternateBackgroundColor: parent.isLightColorScheme ? hifi.colors.tableRowLightOdd : hifi.colors.tableRowDarkOdd
        
        headerDelegate: Rectangle {
            height: hifi.dimensions.tableHeaderHeight
            color: isLightColorScheme ? hifi.colors.tableBackgroundLight : hifi.colors.tableBackgroundDark

            RalewayRegular {
                id: titleText
                text: styleData.value
                size: hifi.fontSizes.tableHeading
                font.capitalization: Font.AllUppercase
                color: hifi.colors.baseGrayHighlight
                horizontalAlignment: (centerHeaderText ? Text.AlignHCenter : Text.AlignLeft)
                elide: Text.ElideRight
                anchors {
                    left: parent.left
                    leftMargin: hifi.dimensions.tablePadding
                    right: sortIndicatorVisible && sortIndicatorColumn === styleData.column ? titleSort.left : parent.right
                    rightMargin: hifi.dimensions.tablePadding
                    verticalCenter: parent.verticalCenter
                }
            }

            HiFiGlyphs {
                id: titleSort
                text:  sortIndicatorOrder == Qt.AscendingOrder ? hifi.glyphs.caratUp : hifi.glyphs.caratDn
                color: isLightColorScheme ? hifi.colors.darkGray : hifi.colors.baseGrayHighlight
                opacity: 0.6;
                size: hifi.fontSizes.tableHeadingIcon
                anchors {
                    right: parent.right
                    verticalCenter: titleText.verticalCenter
                }
                visible: sortIndicatorVisible && sortIndicatorColumn === styleData.column
            }

            Rectangle {
                width: 1
                anchors {
                    left: parent.left
                    top: parent.top
                    topMargin: 1
                    bottom: parent.bottom
                    bottomMargin: 2
                }
                color: isLightColorScheme ? hifi.colors.lightGrayText : hifi.colors.baseGrayHighlight
                visible: styleData.column > 0
            }

            Rectangle {
                height: 1
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                color: isLightColorScheme ? hifi.colors.lightGrayText : hifi.colors.baseGrayHighlight
            }
        }

        branchDelegate: HiFiGlyphs {
            text: styleData.isExpanded ? hifi.glyphs.caratDn : hifi.glyphs.caratR
            size: hifi.fontSizes.carat
            color: colorScheme == hifi.colorSchemes.light
                   ? (styleData.selected ? hifi.colors.black : hifi.colors.baseGrayHighlight)
                   : (styleData.selected ? hifi.colors.black : hifi.colors.lightGrayText)
            anchors {
                left: parent ? parent.left : undefined
                leftMargin: hifi.dimensions.tablePadding / 2
            }

            onTextChanged: {
                console.log("Tree.qml - branch::onTextChanged - " + styleData.value + " Selected: " + styleData.selected + " Expanded: " + styleData.isExpanded );
            }

            Component.onCompleted: {
                console.log("Tree.qml - branch::onCompleted - branches[" + treeView.branchModelIndices.length + "]: " + styleData.value + " Selected: " + styleData.selected + " Expanded: " + styleData.isExpanded );
                treeView.branchModelIndices[ treeView.branchModelIndices.length ] = styleData.index;
            }
        }

        handle: Item {
            id: scrollbarHandle
            implicitWidth: hifi.dimensions.scrollbarHandleWidth
            Rectangle {
                anchors {
                    fill: parent
                    topMargin: treeView.headerVisible ? hifi.dimensions.tableHeaderHeight + 3 : 3
                    bottomMargin: 3     // ""
                    leftMargin: 1       // Move it right
                    rightMargin: -1     // ""
                }
                radius: hifi.dimensions.scrollbarHandleWidth / 2
                color: treeView.isLightColorScheme ? hifi.colors.tableScrollHandleLight : hifi.colors.tableScrollHandleDark
            }
        }

        scrollBarBackground: Item {
            implicitWidth: hifi.dimensions.scrollbarBackgroundWidth
            Rectangle {
                anchors {
                    fill: parent
                    topMargin: treeView.headerVisible ? hifi.dimensions.tableHeaderHeight - 1 : -1
                    margins: -1     // Expand
                }
                color: treeView.isLightColorScheme ? hifi.colors.tableScrollBackgroundLight : hifi.colors.tableScrollBackgroundDark
                
                // Extend header color above scrollbar background
                Rectangle {
                    anchors {
                        top: parent.top
                        topMargin: -hifi.dimensions.tableHeaderHeight
                        left: parent.left
                        right: parent.right
                    }
                    height: hifi.dimensions.tableHeaderHeight
                    color: treeView.isLightColorScheme ? hifi.colors.tableBackgroundLight : hifi.colors.tableBackgroundDark
                    visible: treeView.headerVisible
                }
                Rectangle {
                    // Extend header bottom border
                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                    }
                    height: 1
                    color: treeView.isLightColorScheme ? hifi.colors.lightGrayText : hifi.colors.baseGrayHighlight
                    visible: treeView.headerVisible
                }
            }
        }

        incrementControl: Item {
            visible: false
        }

        decrementControl: Item {
            visible: false
        }
    }

    rowDelegate: Rectangle {
        height: hifi.dimensions.tableRowHeight
        color: styleData.selected
               ? hifi.colors.primaryHighlight
               : treeView.isLightColorScheme
                   ? (styleData.alternate ? hifi.colors.tableRowLightEven : hifi.colors.tableRowLightOdd)
                   : (styleData.alternate ? hifi.colors.tableRowDarkEven : hifi.colors.tableRowDarkOdd)
    }

    itemDelegate: FiraSansSemiBold {
        anchors {
            left: parent ? parent.left : undefined
            leftMargin: (2 + styleData.depth) * hifi.dimensions.tablePadding
            right: parent ? parent.right : undefined
            rightMargin: hifi.dimensions.tablePadding
            verticalCenter: parent ? parent.verticalCenter : undefined
        }

        text: styleData.value
        size: hifi.fontSizes.tableText
        color: colorScheme == hifi.colorSchemes.light
                ? (styleData.selected ? hifi.colors.black : hifi.colors.baseGrayHighlight)
                : (styleData.selected ? hifi.colors.black : hifi.colors.lightGrayText)
                       
        elide: Text.ElideRight
    }

    Item {
        id: unfocusHelper
        visible: false
    }

    onDoubleClicked: isExpanded(index) ? collapse(index) : expand(index)

    onClicked: {
        console.log("Tree.qml - onClicked - Triggered");
        var allowFileClickSignal = true;
        for (var branchIndex = 0; branchIndex < branchModelIndices.length; ++branchIndex) {
            if (index != branchModelIndices[ branchIndex ]) {
                continue;
            }

            console.log("Tree.qml - onClicked - Branch Screening: Match Found at arrIndex: " + branchIndex);
            allowFileClickSignal = false;
        }

        var prevSelectedModelIndex = selectionModel.currentIndex;
        selectionModel.setCurrentIndex(index, ItemSelectionModel.ClearAndSelect);
        if (allowFileClickSignal) {
            console.log("Tree.qml - onClicked - Emitting Signal: selectedFile.");
            selectedFile(selectionModel.currentIndex, prevSelectedModelIndex);
        }
    }

    onPressAndHold: {
        var isCurrentSelection = (index == selectionModel.currentIndex);
        console.log("Tree.qml - onPressAndHold - Triggered on currentIndex: " + isCurrentSelection);
    }

    onExpanded: {
        console.log("Tree.qml - onExpanded - Triggered");
    }

    onCollapsed: {
        console.log("Tree.qml - onCollapsed - Triggered");
    }
}
