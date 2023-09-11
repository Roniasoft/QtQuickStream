pragma Singleton

import QtQuick
import QtQuick.Controls

import QtQuickStream

/*! ***********************************************************************************************
 * QSSerializer is resonsible for transforming object from their in-memory form to something that
 * can be saved/loaded to/from disk.
 *
 * ************************************************************************************************/

QtObject {
    id: serializer

    /* Enumerations
     * ****************************************************************************************/
    enum SerialType {
        STORAGE = 0,    // provides object type AS IS, but sets remote objects to unavailable
        NETWORK = 1     // provides objects interface type, but passes on availability AS IS
    }

    /* Property Declarations
     * ****************************************************************************************/
    readonly property var    blackListedPropNames: [
        "objectName",
        "selectionModel"
    ]

    //! Identified for QtQuickStream object references
    readonly property string protoString: "qqs:/"
    readonly property string protoStrLen: protoString.length

    /* Functions
     * ****************************************************************************************/
    //! Create object based on its type and imports
    function createQSObject(qsType: string, imports = [ "QtQuickStream" ],
                             parent = serializer) : object
    {
        let importString = imports.map(item => "import " + item + "; ").join("");
        let obj = Qt.createQmlObject(importString + qsType + "{}", parent);

        //! \todo Is this really necessary???
        if (parent === serializer) {
            obj._qsParent = null;
        }

        return obj;
    }

    //! Restores all QtQuickStream URLs by object references (using repo to resolve the object),
    //! and handles some other problematic data types
    function fromQSUrlProps(obj, props, repo: QSRepository) : object
    {
        // Go over all props
        for (const [propName, propVal] of Object.entries(props)) {
            //! \todo Skip readonly properties
            if ((typeof propVal) === "function")   { continue; }
            if (propName === "qsType")             { continue; }
            if (isPropertyBlackListed(propName))   { continue; }

            try {
                // Get temporary (will overwrite sub-properties of old prop value)
                let oldPropVal = obj[propName];
                let tmpPropVal = fromQSUrlProp(oldPropVal, propVal, repo);
                // Skip if equal, overwrite if different
                if (tmpPropVal !== oldPropVal) {
                    obj[propName] = tmpPropVal;
                }
            } catch (e) {
                console.log(e);
            }
        }

        return obj;
    }

    //! Restores QtQuickStream URLs by object references (using repo to resolve the object), and
    //! handles some other problematic data types
    //! \todo Handle Date types
    function fromQSUrlProp(objProp, propValue, repo: QSRepository)
    {
        // Immediately return null, undefined, 0, etc.
        if (propValue == null) {
            return propValue;
        }
        // Resolve QtQuickStream references
        else if (typeof propValue === "string" && propValue.startsWith(protoString)) {
            // Replace url by object
            return resolveQSUrl(propValue, repo);
        }
        // Handle objects (property maps)
        else if (typeof propValue === "object") {
            // Handle vector types
            //! \todo this should be based on the propValue, not the objProp
            //! \note hopefully this wont be needed in a future qt version
            if (isQVector(objProp)) {
                return Qt.vector2d(propValue["x"], propValue["y"]);
            }
            // Make sure arrays stay arrays
            else if (Array.isArray(objProp)) {
                return Object.values(propValue).map(
                    item => fromQSUrlProp(item, item, repo)
                );
            }
            // Handle Dates
            else if (objProp instanceof Date) {
                return Date.parse(propValue);
            }
            // Make sure QObjects say QObjects, recurse on subproperty
            else if (Qt.isQtObject(objProp)) {
                return fromQSUrlProps(objProp, propValue, repo);
            }
            // Instantiate encapsulated objects
            else if (isQSObject(propValue)) {
                let obj = QSSerializer.createQSObject(propValue.qsType, repo._allImports);
                return fromQSUrlProps(obj, propValue, repo);
            }
            // Otherwise replace object (= property map), recurse on subproperty
            // -- this is needed to allow deletion of properties!
            else {
                return fromQSUrlProps(propValue, propValue, repo);
            }
        }

        // Default: return the propValue
        return propValue;
    }

    //! Returns a map (object) in which other QSObjects are replaced by their QtQuickStream URL
    //! \todo This code needs a cleanup/rewrite
    function getQSProps(obj: object, serialType = QSSerializer.SerialType.STORAGE) : object
    {
        const handleAsInterface     = (serialType === QSSerializer.SerialType.NETWORK)
                                   && obj?._qsInterfaceType;
        const handleAsUnavailable   = (serialType === QSSerializer.SerialType.STORAGE);

        // Get list of interface properties if applicable
        const ifacePropNames        = handleAsInterface ? obj.getInterfacePropNames() : [];

        let objectSimpleProps = {};

        // Serialize all properties that are not blacklisted
        for (const [propName, propVal] of Object.entries(obj)) {
            // Skip blacklisted properties
            if (isPropertyBlackListed(propName))                            { continue; }
            // Skip non-interface propnames
            if (handleAsInterface && !ifacePropNames.includes(propName))    { continue; }

            objectSimpleProps[propName] = getQSProp(propVal, serialType);
        }

        // Overwrite type by interface if only interfaces requested
        if (handleAsInterface) {
            objectSimpleProps["qsType"] = obj._qsInterfaceType;
        }

        return objectSimpleProps;
    }

    //! Replaces QSObjects by their QtQuickStream
    function getQSProp(propValue, serialType = QSSerializer.SerialType.STORAGE)
    {
        // Nothing to do for null, undefined, etc.
        if (propValue == null) { return propValue; }

        // Write non-qt objects
        if (typeof propValue === "object") {
            // Replace QSObject by its UUID if it's registered
            if (isRegisteredQSObject(propValue)) {
                return getQSUrl(propValue);
            }
            // Handle dates
            else if (propValue instanceof Date) {
                try {
                    return propValue.toISOString();
                } catch (e) {
                    return null;
                }
            }
            // Handle arrays
            else if (Array.isArray(propValue)) {
                return propValue.map(elem => getQSProp(elem, serialType));
            }
            // Recurse
            else {
                return getQSProps(propValue, serialType);
            }
        }

        return propValue;
    }

    /* Helper Functions
     * ****************************************************************************************/
    //! Returns whether the property should be de/serialized
    function isPropertyBlackListed(propName: string) : bool
    {
        return propName.startsWith("_") || propName.endsWith("_")
            || blackListedPropNames.includes(propName);
    }

    //! Returns an object reference based on the url (qqs:/UUID)
    //! \todo Repo IDs should precede the object id so you get qqss:/RepoName/QSObjectUUID
    function getQSUrl(obj: QSObject) : string
    {
        return obj !== null
             ? protoString + obj._qsUuid
             : null;
    }

    //! Returns whether the object is a Qt vector.
    //! \todo Terrible performance - hopefully not needed in future Qt versions
    function isQVector(obj) : bool
    {
        if(obj === undefined)
            return false;

        if(obj.hasOwnProperty("x") && obj.hasOwnProperty("y"))
            return true;

        return obj !== null
            && obj.toString().startsWith("QVector");
    }

    //! Returns whether the object is QSObject.
    function isQSObject(obj) : bool
    {
        return (obj.qsType !== undefined);
    }

    //! Returns whether the object is a QSObject registered with Repo
    //! \todo This checks that object is not a REPO, but repo shouldn't be a qsobject
    function isRegisteredQSObject(obj) : bool
    {
        return isQSObject(obj)
            && !(obj._qsRepo === null && obj._isLoading === undefined);
    }

    //! Returns an object reference (proxy when repo is remote) based on the url (qqs:/UUID)
    //! \todo Repo IDs should precede the object id so you get qqs:/RepoName/QSObjectUUID
    function resolveQSUrl(qsUrl, repo) : QSObject
    {
        if (typeof qsUrl === "string" && qsUrl.startsWith(protoString)) {
            var uuid = qsUrl.substring(protoStrLen);
            return repo._qsUuid === uuid
                 ? repo
                 : repo._qsObjects[uuid];
        } else {
            return null;
        }
    }
}
