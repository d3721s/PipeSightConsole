pragma Singleton

import QtQuick

QtObject {
    signal notificationRequested(string message, bool isError)

    function show(message, isError) {
        notificationRequested(String(message), isError === true)
    }

    function info(message) {
        show(message, false)
    }

    function error(message) {
        show(message, true)
    }

    function nativeInfo(title, message) {
        NativeNotifier.notify(title, message)
    }

    function nativeError(title, message) {
        NativeNotifier.notify(title, message)
    }
}
