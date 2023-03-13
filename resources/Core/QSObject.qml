import QtQuick

import QtQuickStream

/*! ***********************************************************************************************
 * The QSObjectQml provides additional JavaScript/QML helper functionality.
 * ************************************************************************************************/
QSObjectCpp {
    id: qsObjectQml

    /* Signals
     * ****************************************************************************************/
    signal loadedFromStorage()
    signal removedFromRepo()

    /* Signal Handlers
     * ****************************************************************************************/
    on_QsRepoChanged: {
        if (_qsRepo === null) { removedFromRepo(); }
    }

    /* Helper Functions
     * ****************************************************************************************/
    //! Adds element to container and emits containerChangedSignal
    //! \note this takes parentship of the element
    function addElement(container, element: QSObject, containerChangedSignal, emit = true) {
        // Sanity check
        if (container[element._qsUuid] === element) { return; }

        // Assign the repo
        element._qsRepo = qsObjectQml._qsRepo;
        element._qsParent = qsObjectQml;

        // Add to local administration
        container[element._qsUuid] = element;

        // Inform obersvers
        if (emit) { containerChangedSignal(); }
    }

    //! Adds/removes element from/to container to match elements and emits containerChangedSignal
    //! \note this is more efficient than adding/removing multiple times
    function setElements(container, elements, containerChangedSignal) {
        let oldElements = Object.values(container);
        let newElements = Object.values(elements);

        // Sanity check
        if (oldElements.equals(newElements)) { return; }

        let toRemove    = oldElements.filter(x => !newElements.includes(x));
        let toAdd       = newElements.filter(x => !oldElements.includes(x));

        for (let i in toRemove) {
            removeElement(container, toRemove[i], containerChangedSignal, false);
        }

        for (let j in toAdd) {
            addElement(container, toAdd[j], containerChangedSignal, false);
        }

        // Generate new container to preserve order
        let tempContainer = {};
        for (let element in newElements) {
            tempContainer[element._qsUuid] = element;
        }

        // Allocate and inform observers
        container = tempContainer;
        containerChangedSignal();
    }

    //! Removes element from container and emits containerChangedSignal
    //! \note this gives up parentship of the element
    function removeElement(container, element: QSObject, containerChangedSignal, emit = true) {
        // Sanity check
        if (container[element._qsUuid] === undefined) {
            console.warn("Attempted to remove unknown element: " + element?._qsUuid);
            return;
        }

        // Remove from local administration
        delete container[element._qsUuid];

        // Remove from repo
        element._qsRepo = null;
        element._qsParent = null;

        // Inform obersvers
        // Inform obersvers
        if (emit) { containerChangedSignal(); }
    }

    //! Copies all properties from propertiesMap to this object
    //! \note For remote usage, this is far more efficient than assign each value separately
    function setProperties(propertiesMap: object, qsRepo = _qsRepo) {
        QSSerializer.fromQSUrlProps(qsObjectQml,
                                      QSSerializer.getQSProps(propertiesMap),
                                      qsRepo);
    }
}
