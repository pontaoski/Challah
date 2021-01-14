import QtQuick 2.7

Item {
	default property Item contentItem: null

	id: root
	property bool childVisible: false
	implicitHeight: childVisible ? contentItem.height : 0
	implicitWidth: contentItem.width
	clip: true

	enum Direction {
		FromBottom,
		FromTop
	}
	property int direction: Direction.FromBottom

	Behavior on implicitHeight {
		NumberAnimation {
			duration: 150
			easing.type: Easing.InOutQuart
		}
	}

	function reanchor() {
		contentItem.parent = this
		if (this.direction == Direction.FromBottom) {
			contentItem.anchors.top = this.top
			contentItem.anchors.bottom = null
		} else {
			contentItem.anchors.top = this.top
			contentItem.anchors.bottom = this.bottom
		}
		contentItem.anchors.left = this.left
	}

	onContentItemChanged: reanchor()
	onDirectionChanged: reanchor()
}
