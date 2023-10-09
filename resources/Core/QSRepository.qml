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

    //! Application name
    property string             _applicationKey:     "Application"
    property string             _applicationName: ""

    //! Application Version and supported version
    property string _versionKey: "version"
    property string _version: ""
    property string _supported_minimum_version: ""

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

        // Add version
        jsonObjects[_versionKey] = _version;

        //! Hash the application name and the licensekey
        var hashedAppKey     = HashStringCPP.hexHashString(_applicationKey);
        var hashedAppName    = HashStringCPP.hexHashString(_applicationName);
        jsonObjects[hashedAppKey] = hashedAppName;

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

        /* 0. Validate the file
         * ********************************************************************************/

        //! Hash the application name and its key
        var hashedAppKey     = HashStringCPP.hexHashString(_applicationKey);
        var hashedAppName    = jsonObjects[hashedAppKey];
        var hashRealAppName  = HashStringCPP.hexHashString(_applicationName);

        if (hashedAppName.length !== 0 && !HashStringCPP.compareStringModels(hashedAppName, hashRealAppName)) {
            console.warn("[QSRepo] The file is unrelated to the application, failed.");
            _isLoading = false;
            return false;
        }

        delete jsonObjects[hashedAppKey];

        /* 1. Check version
         * ********************************************************************************/

        var versionString = jsonObjects[_versionKey];
        if (_supported_minimum_version.length > 0) {
            console.warn("[QSRepo] Loading Version ", versionString);
            if (!versionString || !checkApplicationVersion(versionString) || !checkSupportedVersion(versionString)) {
                console.warn("[QSRepo] Version not supported, failed. Minimum spported version is ", _supported_minimum_version);
                _isLoading = false;
                return false
            }
        }

        delete jsonObjects[_versionKey];

        /* 2. Validate Object Map
         * ********************************************************************************/
        if (jsonObjects[_rootkey] === undefined) {
            console.warn("[QSRepo] Could not find root, failed.");
            _isLoading = false;
            return false;
        }

        //! \todo remove this hack by serializing differently
        var rootUrl = jsonObjects[_rootkey];
        delete jsonObjects[_rootkey];

        /* 3. Create objects
         * ********************************************************************************/
        loadQSObjects(jsonObjects);

        /* 4. Delete unneeded objects
         * ********************************************************************************/
        if (deleteOldObjects) {
            Object.keys     (_qsObjects)
                  .filter   (objId => !(objId in jsonObjects))
                  .forEach  (objId =>
            {
                delObject(objId)
            });
        };

        /* 5. Set root object
         * ********************************************************************************/
        // Reload root
        let rootObj = QSSerializer.resolveQSUrl(rootUrl, repo);
        if (qsRootObject && rootObj && rootObj._qsUuid !== qsRootObject._qsUuid)
            qsRootObject.destroy();

        qsRootObject = rootObj;

        /* 6. Initialize local objects
         * ********************************************************************************/
        // Inform all new local objects that they were loaded (from storage)
        for (const [objId, qsObj] of Object.entries(_qsObjects)) {
            if (objId in jsonObjects) {
                qsObj?.loadedFromStorage();
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

    /*! ***************************************************************************************
     * Set application name
     * ****************************************************************************************/
    function setApplicationName(application: string) {
        _applicationName = application;
    }

    /*! ***************************************************************************************
     * Set application version
     * ****************************************************************************************/

    function setApplicationVersion(version: string) {
        _version = version;
    }

    function setApplicationMinimumVersionSupported(version: string) {
        _supported_minimum_version = version;
        if (!checkSupportedVersion(_version))
            console.warn("[QSRepo] Minimum Supported version : ", version, " can not be greater than current version: ", _version);
    }

    /*! ***************************************************************************************
     * converts version string to int
     * assumed that each part is not greater than 99
     * ****************************************************************************************/
    function getVersionNumber(versionString: string): int {
        if (versionString.length > 0) {
            var versionArray = versionString.split(".");
            var resultArray = versionArray.map(function(part, index) {
                if (part > 99)
                    console.warn("[QSRepo] version part should not be greater than 99")
                // Convert the part to a number and perform different operations based on the index
                switch (index) {
                    case 0:
                        return parseInt(part) * 10000;
                    case 1:
                        return parseInt(part) * 100;
                    default:
                        return parseInt(part);
                }
            });

            // Calculate the sum of the results
            var sum = resultArray.reduce(function(total, value) {
                return total + value;
            }, 0);

            return sum;
        }
    }

    /*! ***************************************************************************************
     * Check application version
     * The application should only load files with major version equal or less unless that not match the minimum version required.
     * ****************************************************************************************/
    function checkSupportedVersion(savedVersion: string) : bool {
        if (savedVersion.length > 0) {
            var saved = getVersionNumber(savedVersion)
            var minimum = getVersionNumber(_supported_minimum_version)

            var isValidVersion  = saved >= minimum;

            if (!isValidVersion)
                console.warn("[QSRepo] the save file is too old, not supported.");

            return isValidVersion;
        }

        return false;
    }


    function checkApplicationVersion(savedVersion: string) : bool {

        if (savedVersion.length > 0) {
            var versionArraySaved = savedVersion.split(".");
            if (versionArraySaved.length > 0) {
                var majorVersionSaved = versionArraySaved[0];

                var versionArrayApp = _version.split(".");
                var majorVersionApp = versionArrayApp[0];

                var isValidVersion  = majorVersionApp >= majorVersionSaved;

                if (!isValidVersion)
                    console.warn("[QSRepo] the save file is for Higher Major version, not supported.");

                return isValidVersion;
            }
        }

        return false;
    }
}
