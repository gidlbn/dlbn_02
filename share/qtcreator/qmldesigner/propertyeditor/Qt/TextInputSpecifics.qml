import Qt 4.7
import Bauhaus 1.0

QWidget {
    id: textSpecifics;

    layout: QVBoxLayout {
        topMargin: 0
        bottomMargin: 0
        leftMargin: 0
        rightMargin: 0
        spacing: 0

        StandardTextGroupBox {
            finished: finishedNotify;
        }

        StandardTextColorGroupBox {
            finished: finishedNotify;
            showSelectionColor: true;
            showSelectedTextColor: true;
        }

        FontGroupBox {
            finished: finishedNotify;

        }

        TextInputGroupBox {
            finished: finishedNotify
            isTextInput: true
        }
        QScrollArea {
        }
    }
}
