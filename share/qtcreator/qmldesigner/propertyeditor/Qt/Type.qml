import Qt 4.7
import Bauhaus 1.0

GroupBox {
    id: type;
    finished: finishedNotify;
    caption: qsTr("Type")

    layout: VerticalLayout {
        spacing: 6
        QWidget {
            layout: HorizontalLayout {
                Label {
                    text: qsTr("Type")
                    windowTextColor: isBaseState ? "#000000" : "#FF0000";
                }

                QLabel {
                    text: backendValues.className.value;
                }

            }
        }
        QWidget {
            property variant isEnabled: isBaseState
            onIsEnabledChanged: idLineEdit.setStyleSheet("color: "+(isEnabled?scheme.defaultColor:scheme.disabledColor));
            ColorScheme{ id:scheme }

            layout: HorizontalLayout {
                Label {
                    text: qsTr("Id");
                }

                QLineEdit {
                    enabled: isBaseState
                    id: idLineEdit;
                    objectName: "idLineEdit";
                    readOnly: isBaseState != true;
                    text: backendValues.id.value;
                    onEditingFinished: {
                        backendValues.id.value = text;
                    }
                }
            }
        }
    }
}
