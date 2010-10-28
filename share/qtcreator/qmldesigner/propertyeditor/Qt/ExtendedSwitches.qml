import Qt 4.7
import Bauhaus 1.0

QFrame {
    visible: false;
    focusPolicy: "Qt::NoFocus"
    id: extendedSwitches;
    property bool active: false;
    property variant backendValue;
    styleSheetFile: "switch.css";
    property variant specialModeIcon;
    specialModeIcon: "images/standard.png";

    opacity: 0;

    Behavior on opacity {
        NumberAnimation {
            easing.type: "InSine"
            duration: 200
        }
    }

    onActiveChanged: {
        if (!nameLineEdit.focus)
            if (active) {
                opacity = 1;
            } else {
                opacity = 0;
            }
    }

    layout: QHBoxLayout {
        topMargin: 6;
        bottomMargin: 4;
        leftMargin: 10;
        rightMargin: 8;
        spacing: 4;

        QPushButton {
            focusPolicy: "Qt::NoFocus"
            onReleased:  backendValue.resetValue();
            iconFromFile: "images/reset-button.png";
            toolTip: "reset property";
        }

        QLineEdit {
            id: nameLineEdit;
            readOnly: false;
            //focusPolicy: "Qt::NoFocus"
            text: backendValue == null ? "" : backendValue.expression;
            onEditingFinished: {
                if (backendValue != null)
                    backendValue.expression = text;
            }
            onFocusChanged: {
                extendedSwitches.active = focus;
            }
        }
     }
}
