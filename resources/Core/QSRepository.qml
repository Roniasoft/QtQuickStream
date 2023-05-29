import QtQuick

import QtQuickStream

/*! ***********************************************************************************************
 * QSRepository is the container that stores and manages QSObjects. It can be de/serialized from/
 * to the disk, and will enable enable other mechanisms in the future.
 *
 * ************************************************************************************************/
QSRepositoryCpp {
    id: repo

    /* Property Declarations
     * ****************************************************************************************/
    readonly property string    _rootkey:        "root"
    property string             qsRootInterface: ""

    // 'Alias', used internally for creation of objects
    property var                _allImports:    [...imports, ..._localImports]
    // To be set by the concrete application, e.g., 'SystemGUI'
    property var                _localImports:  []

    // To be set by the system core, e.g., 'SystemCore'
    property var                imports:        [ "QtQuickStream" ]

    name: qsRootObject?.objectName ?? "Uninitialized Repo"


    /* ****************************************************************************************
     * OBJECT REMOTING STUFF
     *
     * ****************************************************************************************/

    /* Signal Handlers
     * ****************************************************************************************/

    // Local: Force push repo on root change, start pushing diffs at 5hz
    onQsRootObjectChanged: {
        console.log("[QSRepo] New root is: " + qsRootObject);

        // Sanity check: do nothing if remote
        qsRootInterface = qsRootObject?._qsInterfaceType ?? "";
    }

    onNameChanged: {
    }

    /* Functions
     * ****************************************************************************************/
    //! Initializes root object. Note that imports should be set correctly before doing this!
    function initRootObject(rootObjectType: string) : bool
    {
        if (qsRootObject != null) {
            console.warn("[QSRepo] Reinitializing root object can lead to unexpected behavior!");
        }

        // Create new root object
        let importString = _allImports.map(item => "import " + item + "; ").join("");
        let newRoot = Qt.createQmlObject(importString + rootObjectType + "{ _qsRepo: repo }", repo);

        // Set new root if creation was succesful
        if (newRoot !== null) {
            qsRootObject = newRoot;
        }

        return qsRootObject !== null;
    }
    /* ****************************************************************************************
     * SERIALIZATION
     * ****************************************************************************************/
    /*! ***************************************************************************************
     * Returns a dump of all objects' properties, in which qsobject references are URLs.
     * ****************************************************************************************/
    function dumpRepo(serialType = QSSerializer.SerialType.STORAGE) : object
    {
        var jsonObjects = {};

        // Build tree from all objects' attributes (replacing references by UUIDs)
        for (const [objId, qsObj] of Object.entries(_qsObjects)) {
            try {
                jsonObjects[objId] = QSSerializer.getQSProps(qsObj, serialType);
            } catch (e) {
                console.warn("[QSRepo] " + e.message);
            }
        }

        jsonObjects[_rootkey] = QSSerializer.getQSUrl(qsRootObject);

        return jsonObjects;
    }

    /*! ***************************************************************************************
     * Creates all objects' properties, in which qsobject references are URLs.
     * ****************************************************************************************/
    function loadRepo(jsonObjects: object, deleteOldObjects = true) : bool
    {
        //! Satrt the loading process
        _isLoading = true;

        /* 1. Validate Object Map
         * ********************************************************************************/
        if (jsonObjects[_rootkey] === undefined) {
            console.warn("[QSRepo] Could not find root, aborting");
            _isLoading = false;
            return false;
        }

        //! \todo remove this hack by serializing differently
        var rootUrl = jsonObjects[_rootkey];
        delete jsonObjects[_rootkey];

        /* 2. Create objects
         * ********************************************************************************/
        loadQSObjects(jsonObjects);

        /* 3. Delete unneeded objects
         * ********************************************************************************/
        if (deleteOldObjects) {
            Object.keys     (_qsObjects)
                  .filter   (objId => !(objId in jsonObjects))
                  .forEach  (objId =>
            {
                delObject(objId)
            });
        };

        /* 4. Set root object
         * ********************************************************************************/
        // Reload root
        qsRootObject = QSSerializer.resolveQSUrl(rootUrl, repo);

        /* 5. Initialize local objects
         * ********************************************************************************/
        // Inform all new local objects that they were loaded (from storage)
        for (const [objId, qsObj] of Object.entries(_qsObjects)) {
            if (objId in jsonObjects) {
                qsObj.loadedFromStorage();
            }
        }

        //! Finish the loading process
        _isLoading = false;

        return true;
    }

    /*! ***************************************************************************************
     * Loads objects from a json map of properties, in which qqsobject references are URLs.
     * ****************************************************************************************/
    function loadQSObjects(jsonObjects: object) : bool
    {

        /* 1. Create objects with default property values
         * ********************************************************************************/
        // Read baseline properties
        for (const [objId, jsonObj] of Object.entries(jsonObjects)) {
            if (objId in _qsObjects) {
                console.log("[QSRepo] Skipping creation of: " + objId + " " + jsonObj.qsType);
                continue;
            }

            try {
                var qsObj = QSSerializer.createQSObject(
                                jsonObj.qsType, _allImports, repo
                             );

                // Skip further processing if failed
                if (!qsObj) { continue; }

                qsObj._qsUuid = objId;
                qsObj._qsRepo = repo;

                // Store object in administration
                addObject(objId, qsObj);
            } catch (e) {
                console.warn("[QSRepo] " + e.message);
            }
        }

        /* 2. Update property values
         * ********************************************************************************/
        // Replace all qs://UUID properties by references
        for (const [objId, jsonObj] of Object.entries(jsonObjects)) {
            QSSerializer.fromQSUrlProps(_qsObjects[objId], jsonObj, repo);
        }


        return true;
    }

    /* ****************************************************************************************
     * FILE LOADING & SAVING
     * ****************************************************************************************/
    /*! ***************************************************************************************
     * Loads the repo and all its objects from a file
     * ****************************************************************************************/
    function loadFromFile(fileName: string) : bool
    {
        // Read file
        var jsonString = QSFileIO.read(fileName);

        // Sanity check: abort if file was empty
        if (jsonString == 0) {
            console.log("[QSRepo] File empty, aborting");
            return false;
        }

        var fileObjects = JSON.parse(jsonString);

        return loadRepo(fileObjects);
    }

    /*! ***************************************************************************************
     * Stores the repo and all its objects to a file
     * ****************************************************************************************/
    function saveToFile(fileName: string) : bool
    {
        console.log("[QSRepo] Saving Repo to File: " + fileName);
        console.log(QSSerializer.SerialType.STORAGE);

        // Get the objects for storage
        let repoDump = dumpRepo(QSSerializer.SerialType.STORAGE);

        // Store the tree to file
        return QSFileIO.write(fileName, JSON.stringify(repoDump, null, 4));
    }
}
