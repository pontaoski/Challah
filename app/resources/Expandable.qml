import QtQuick 2.7

Item {
	default property Item contentItem: null

	id: root
	property bool childVisible: false
	implicitHeight: childVisible ? contentItem.height : 0
	implicitWidth: contentItem.width
	clip: true

	Behavior on implicitHeight {
		NumberAnimation {
			duration: 150
			easing.type: Easing.InOutQuart
		}
	}

	onContentItemChanged: {
		contentItem.parent = this
		contentItem.anchors.top = this.top
		contentItem.anchors.left = this.left
	}
}
