import QtQuick 2.5

Item {

    id: entity

    default property alias contents: container.data

    property real halfWidth: width/2
    property real halfHeight: height/2

    property bool draggable: false
    property bool rotatable: false

    property bool useAdaptiveSource: true
    property string adaptiveSource: ""
    property alias source: adaptive.source

    AdaptiveSource {
        id: adaptive
        enabled: useAdaptiveSource
        target: entity
        targetSourceProperty: "adaptiveSource"
    }

    Item {
        id: container
        anchors.fill: parent

    }

    // Debug visuals
    DebugVisual {}

    // Drag'n'Drop functionality
    property bool dragReturnOnReject: true
    property alias dragArea: drag

    property int dragDisplaceX: 0 //main.grow()
    property int dragDisplaceY: 0 //main.grow()

    signal dragAccepted (variant mouse)
    signal dragRejected (variant mouse)
    signal dragStarted (variant mouse)
    signal dragReturn
    signal dragEnded (variant mouse)
    signal dragReturned

    function goBack() {
        dragMoveBackAnimation.running = true
    }

    MouseArea {
        id: drag
        property int ox: draggable ? entity.x : 0
        property int oy: draggable ? entity.y : 0

        enabled: parent.draggable
        visible: enabled

        anchors.fill: entity
        anchors.margins: 0 //main.grow()

        drag.target: entity
        onPressed: {
            if(!dragMoveBackAnimation.running) { // Panic click safety
                ox = entity.x
                oy = entity.y
            }

            var map = mapToItem(entity.parent,mouse.x,mouse.y)
            entity.x = map.x-(entity.width/2)+entity.dragDisplaceX
            entity.y = map.y-(entity.height/2)+entity.dragDisplaceY
            db('drag started',entity)
            dragStarted(mouse)
        }
        onReleased: {
            if(entity.Drag.drop() !== Qt.IgnoreAction) {
                db('drag accepted',entity)
                dragAccepted(mouse)
            } else {
                db('drag rejected',entity)
                dragRejected(mouse)
                goBack()
            }
            db('drag ended',entity)
            dragEnded(mouse)
        }

        function goBack() {
            if(dragReturnOnReject) {
                dragMoveBackAnimation.running = true
                db('drag return',entity)
                dragReturn()
            }
        }

        SequentialAnimation {
            id: dragMoveBackAnimation
            ParallelAnimation {
                PropertyAnimation { target: entity; property: "x"; to: drag.ox; easing.type: Easing.InOutQuad }
                PropertyAnimation { target: entity; property: "y"; to: drag.oy; easing.type: Easing.InOutQuad }
            }
            ScriptAction { script: {
                db('drag returned',entity)
                entity.dragReturned()
            }}
        }
    }

    Drag.active: drag.drag.active
    Drag.hotSpot.x: width / 2
    Drag.hotSpot.y: height / 2

    // Mouse rotate functionality
    MouseArea {
        id: rotator
        enabled: parent.rotatable

        property var container: parent.parent
        property var target: parent
        //property variant handle: parent

        property bool stop: false

        anchors.fill: parent
        anchors.margins: 0 //main.grow()

        property real startRotation: 0

        signal rotate(real degree)

        onPressed: {
            startRotation = target.rotation
        }

        onPositionChanged: {
            if(!enabled)
                return

            var point =  mapToItem(rotator.container, mouse.x, mouse.y)

            var rx = 0
            var ry = 0

            var pto = target.transformOrigin
            if(pto == Item.Center) {
                rx = rotator.container.width / 2; ry = rotator.container.height / 2
            } else if(pto == Item.TopLeft) {
                rx = 0; ry = 0;
            } else if(pto == Item.Left) {
                rx = 0; ry = rotator.container.height / 2
            } else if(pto == Item.BottomLeft) { // Untested
                rx = 0; ry = rotator.container.height
            } else if(pto == Item.Bottom) {
                rx = rotator.container.width / 2; ry = rotator.container.height
            } else if(pto == Item.BottomRight) {  // Untested
                rx = rotator.container.width; ry = rotator.container.height
            } else if(pto == Item.Right) {  // Untested
                rx = rotator.container.width; ry = rotator.container.height / 2
            } else if(pto == Item.TopRight) {  // Untested
                rx = rotator.container.width; ry = 0
            } else if(pto == Item.Top) {  // Untested
                rx = rotator.container.width / 2; ry = 0
            }

            var diffX = (point.x - rx)
            var diffY = -1 * (point.y - ry)
            var rad = Math.atan (diffY / diffX)
            var deg = (rad * 180 / Math.PI)

            var rotation = 0

            if (diffX > 0 && diffY > 0) {
                rotation += 90 - Math.abs (deg)
            }
            else if (diffX > 0 && diffY < 0) {
                rotation += 90 + Math.abs (deg)
            }
            else if (diffX < 0 && diffY > 0) {
                rotation += 270 + Math.abs (deg)
            }
            else if (diffX < 0 && diffY < 0) {
                rotation += 270 - Math.abs (deg)
            }

            db(rotation,rotation+startRotation)

            if(!stop)
                target.rotation = rotation
            //log('point',point.x,point.y,'r',rx,ry,'diff',diffX,diffY,'deg',deg,'rotation',rotation,'mouse',mouse.x,mouse.y)
            rotate(rotation)
        }
    }

    // Movement
    property var moveQueue: []

    function moveTo(x,y) {
        pushMove(x,y)
        startMoving()
    }

    function pushMove(x,y) {
        moveQueue.push(Qt.point(x,y))
        /* Use this for binding to moveQueue changes
        var t = moveQueue
        t.push(Qt.point(x,y))
        moveQueue = t
        */
    }

    function popMove() {
        return moveQueue.shift()
        /* Use this for binding to moveQueue changes
        var t = moveQueue
        var o = t.shift()
        moveQueue = t
        return o
        */
    }

    function startMoving() {
        pathAnim.stop()

        var list = []
        var d = 0
        var pp = Qt.point(entity.x,entity.y)
        while(moveQueue.length > 0) {
            var p = popMove()
            d += distance(pp,p)
            var temp = component.createObject(path, {"x":p.x, "y":p.y})
            list.push(temp)
            pp = p
        }
        db('Travel distance',d)
        pathAnim.duration = d*2
        if(list.length > 0) {
            path.pathElements = list
            pathAnim.start()
        }
    }

    function distance(p1,p2) {
        return Math.sqrt( (p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y)*(p1.y-p2.y) )
    }

    Component
    {
        id: component
        PathLine
        {

        }
    }

    PathAnimation {
        id: pathAnim

        duration: 2000

        target: entity

        anchorPoint: Qt.point(entity.width/2, entity.height/2)
        path: Path {
            id: path
        }
    }
}
