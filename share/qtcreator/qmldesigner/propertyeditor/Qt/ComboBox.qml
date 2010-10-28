import Qt 4.7
import Bauhaus 1.0

QWidget {

    id: comboBox

    property variant backendValue;
    property variant baseStateFlag;
    property alias enabled: box.enabled;

    property alias items: box.items;
    property alias currentText: box.currentText;


    onBaseStateFlagChanged: {
        evaluate();
    }

    property variant isEnabled: comboBox.enabled
    onIsEnabledChanged: {
        evaluate();
    }

    function evaluate() {
        if (backendValue === undefined)
            return;
        if (!enabled) {
            box.setStyleSheet("color: "+scheme.disabledColor);
        } else {
            if (baseStateFlag) {
                if (backendValue.isInModel)
                    box.setStyleSheet("QComboBox,QComboBox:on{color: "+scheme.changedBaseColor+"}QComboBox:off{color:"+scheme.optionsColor+"}");
                else
                    box.setStyleSheet("QComboBox,QComboBox:on{color: "+scheme.defaultColor+"}QComboBox:off{color:"+scheme.optionsColor+"}");
            } else {
                if (backendValue.isInSubState)
                    box.setStyleSheet("QComboBox,QComboBox:on{color: "+scheme.changedStateColor+"}QComboBox:off{color:"+scheme.optionsColor+"}");
                else
                    box.setStyleSheet("QComboBox,QComboBox:on{color: "+scheme.defaultColor+"}QComboBox:off{color:"+scheme.optionsColor+"}");
            }
        }
    }

    ColorScheme { id:scheme; }

    layout: HorizontalLayout {
        QComboBox {
            id: box
            property variant backendValue: comboBox.backendValue
            onCurrentTextChanged: { backendValue.value = currentText; evaluate(); }
			onItemsChanged: {				
				if (comboBox.backendValue.value == curentText)
				    return;
				box.setCurrentTextSilent(comboBox.backendValue.value);
            }			
			
			property variant backendValueValue: comboBox.backendValue.value
			onBackendValueValueChanged: {			 
			    if (comboBox.backendValue.value == curentText)
				    return;					
				box.setCurrentTextSilent(comboBox.backendValue.value);
			}
            ExtendedFunctionButton {
                backendValue: comboBox.backendValue;
                y: 3
                x: 3
            }
        }
    }
}
