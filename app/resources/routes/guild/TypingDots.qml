import QtQuick 2.15
import org.kde.kirigami 2.15 as Kirigami

Row {
    id: control

    spacing: 4
    readonly property int duration: Kirigami.Units.veryLongDuration

    FontMetrics {
        id: fontMetrics
        font: Kirigami.Theme.smallFont
    }

    Repeater {
        model: 3
        delegate: Rectangle {
            id: delegate

            required property int index

            implicitWidth: fontMetrics.xHeight
            implicitHeight: fontMetrics.xHeight
            radius: height / 2
            color: Kirigami.Theme.focusColor

            opacity: 0.4
            scale: 1

            SequentialAnimation {
                running: true
                // stagger
                PauseAnimation { duration: control.duration * delegate.index/3 }

                // the animation itself
                SequentialAnimation {
                    loops: Animation.Infinite

                    // pulse in
                    ParallelAnimation {
                        NumberAnimation {
                            target: delegate
                            property: "opacity"
                            from: 0.4
                            to: 1
                            duration: control.duration
                        }
                        NumberAnimation {
                            target: delegate
                            property: "scale"
                            from: 1
                            to: 1 + (1/3)
                            duration: control.duration
                        }
                    }

                    // pulse out
                    ParallelAnimation {
                        NumberAnimation {
                            target: delegate
                            property: "opacity"
                            to: 0.4
                            from: 1
                            duration: control.duration
                        }
                        NumberAnimation {
                            target: delegate
                            property: "scale"
                            to: 1
                            from: 1 + (1/3)
                            duration: control.duration
                        }
                    }

                    // stay pulsed out for a bit
                    PauseAnimation { duration: control.duration/2 }
                }
            }
        }
    }
}