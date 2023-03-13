pragma Singleton

import QtQuick
import QtQuick.Controls

import QtQuickStream

QtObject {
    Component.onCompleted: {
        reigsterArrayEqualsFunc();
        reigsterArrayFlatFunc();
    }

    //! Add Array.equals(other) function.
    //! From: https://stackoverflow.com/questions/7837456/how-to-compare-arrays-in-javascript
    function reigsterArrayEqualsFunc()
    {
        // Warn if overriding existing method
        if (Array.prototype.equals) {
            console.warn("Array.prototype.equals already exists -- aborting!");
        }

        // attach the .equals method to Array's prototype to call it on any array
        Array.prototype.equals = function (array) {
            // if the other array is a falsy value, return
            if (!array) { return false; }

            // compare lengths - can save a lot of time
            if (this.length !== array.length) { return false; }

            for (var i = 0, l = this.length; i < l; i++) {
                // Check if we have nested arrays
                if (this[i] instanceof Array && array[i] instanceof Array) {
                    // recurse into the nested arrays
                    if (!this[i].equals(array[i])) { return false; }
                }
                else if (this[i] !== array[i]) {
                    // Warning - two different object instances will never be equal: {x:20} != {x:20}
                    return false;
                }
            }

            return true;
        }

        // Hide method from for-in loops
        Object.defineProperty(Array.prototype, "equals", {enumerable: false});
    }

    function reigsterArrayFlatFunc()
    {
        // Warn if overriding existing method
        if (Array.prototype.flat) {
            console.warn("Array.prototype.flat already exists -- aborting!");
        }

        // attach the .equals method to Array's prototype to call it on any array
        Array.prototype.flat = function () {
            return [].concat(...this)
        }

        // Hide method from for-in loops
        Object.defineProperty(Array.prototype, "flat", {enumerable: false});
    }
}
