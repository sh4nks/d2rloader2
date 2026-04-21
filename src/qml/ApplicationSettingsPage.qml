pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardPage {
    id: root

    title: "Application Settings"

    FormCard.FormHeader {
        title: "General"
    }

    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            text: "Start D2RLoader on system boot"
            description: "Automatically launch the application when you log in."
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: "Minimize to system tray"
            description: "Keep the application running in the tray when the window is closed."
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormComboBoxDelegate {
            text: "Interface Theme"
            description: "Choose the visual appearance of the application."
            model: ["System Default", "Light", "Dark"]
        }
    }

    FormCard.FormHeader {
        title: "Advanced"
    }

    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            text: "Use hardware acceleration"
            description: "Enable GPU acceleration for a smoother interface."
        }
    }
}
