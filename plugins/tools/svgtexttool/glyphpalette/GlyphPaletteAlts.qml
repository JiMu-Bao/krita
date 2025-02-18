import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQml.Models 2.15
import org.krita.tools.text 1.0

Rectangle {
    id: root;
    property QtObject glyphModel;
    property int parentIndex: 0;
    property int columns: 8;
    property bool replace: false;

    property var fontFamilies: [];
    property double fontSize: 10.0;
    property double fontWeight: 400;
    property double fontWidth: 100;
    property int fontStyle: 0;
    property double fontSlant: 0.0;
    SystemPalette {
        id: sysPalette;
        colorGroup: SystemPalette.Active
    }

    color: sysPalette.base;

    GridView {
        anchors.fill: parent;

        model: DelegateModel {
            id: glyphAltModel
            model: root.glyphModel;
            property alias parentIndex: root.parentIndex;
            property var defaultIndex: modelIndex(-1);
            onParentIndexChanged: {
                // This first line is necessary to reset the model to the root of the glyph model.
                rootIndex = defaultIndex;
                rootIndex = modelIndex(root.parentIndex);
            }

            delegate: GlyphDelegate {
                textColor: hovered? sysPalette.highlightedText: sysPalette.text;
                fillColor: hovered? sysPalette.highlight: sysPalette.base;
                fontFamilies: root.fontFamilies;
                fontSize: root.fontSize;
                fontStyle: root.fontStyle;
                fontWeight: root.fontWeight;
                onGlyphClicked: (index, mouse)=> {
                                    if (!root.replace) {
                                        mainWindow.slotInsertRichText(root.parentIndex, index, root.replace)
                                    }
                                };
                onGlyphDoubleClicked: (index, mouse)=> {mainWindow.slotInsertRichText(root.parentIndex, index, root.replace)};
            }
        }
        focus: true;
        clip: true;

        cellWidth: width/root.columns;
        cellHeight: cellWidth;

        ScrollBar.vertical: ScrollBar {
            id: glyphAltScroll;
        }
    }
}
