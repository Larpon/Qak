import QtQuick 2.5

import Qak 1.0

pragma Singleton

QtObject {
    id : component

    property bool debug: false
    property bool pause: false
    onPauseChanged: info(pause ? 'Paused' : 'Continued')

    property string logPrefix: "QAK"

    property int assetMultiplier: 1

    property QtObject platform: Qt.platform
    property alias resource: res

    Component.onCompleted: {
        var os = Qt.platform.os
        platform.isDesktop = (
            os !== 'android' &&
            os !== 'ios' &&
            os !== 'blackberry' &&
            os !== 'winphone'
        )

        platform.isMobile = !platform.isDesktop
    }

    Resource {
        id: res
    }

    function log() {
        //log.history=log.history||[]
        //log.history.push(arguments)
        if(console) {
            // 1. Convert args to a normal array
            var args = Array.prototype.slice.call(arguments);

            // 2. Prepend log prefix log string
            if(logPrefix != "")
                args.unshift(logPrefix);

            // 3. Pass along arguments to console.log
            console.log.apply(console, args);
            //console.log('QAK',Array.prototype.slice.call(arguments))
        }
    }

    function error() {
        //log.history=log.history||[]
        //log.history.push(arguments)
        if(console) {
            var args = Array.prototype.slice.call(arguments)
            args.unshift("QAK ERROR")
            console.error.apply(console, args)
        }
    }

    function db() {
        //db.history=db.history||[]
        //db.history.push(arguments)
        if(console && debug) {
            var args = Array.prototype.slice.call(arguments)
            args.unshift("QAK DEBUG")
            console.debug.apply(console, args)
        }
    }

    function warn() {
        //db.history=db.history||[]
        //db.history.push(arguments)
        if(console) {
            var args = Array.prototype.slice.call(arguments)
            args.unshift("QAK WARNING")
            console.warn.apply(console, args)
        }
    }

    function info() {
        //log.history=log.history||[]
        //log.history.push(arguments)
        if(console) {
            var args = Array.prototype.slice.call(arguments)
            args.unshift("QAK INFO")
            console.info.apply(console, args)
        }
    }

    function asset(path) {
        var assetPath = "assets/"+path
        var filename = path.replace(/^.*[\\\/]/, '')

        if(platform.isDesktop && debug) {
            path = 'file:///home/lmp/Projects/HammerBees/HammerBeesApp/assets/'+path
        } else {
            if(platform.os === 'android')
                path = "assets:/"+path
            else if(platform.os === 'osx' && endsWith(path,'.mp3'))
                path = 'file://'+Qak.resource.appPath()+'/'+path
            else
                path = 'qrc:///'+assetPath
        }

        if(!Qak.resource.exists(path)) {
            if(filename.indexOf("*") > -1)
                info('Wildcard asset',path)
            else
                error('Invalid asset',path)
        }
        //else
        //    console.info('Asset',path)

        return path
    }

    function assetExists(path) {
        return Qak.resource.exists(asset(path))
    }

}
