import QtQuick
import QtQuickStream

QSCoreCpp {
    id: core

    /* Signal Handlers
     * ****************************************************************************************/
    Component.onCompleted: {
        // Init QSUtil singleton by simply calling it.
        QSUtil;
    }

    // Handle C++ backend signal to create a Repository
    onSigCreateRepo: (repoId, isRemote) => createRepo(repoId, isRemote);

    /* Functions
     * ****************************************************************************************/
    function createDefaultRepo(imports: object) {
        // Create repo
        var newRepo = createRepo(core.coreId, false);
        newRepo.imports = imports;

        return newRepo;
    }

    function createRepo(repoId: string, isRemote: bool) {
        // Create repo
        var newRepo = QSSerializer.createQSObject("QSRepository", ["QtQuickStream"], core);

        newRepo._qsUuid = repoId;
        newRepo.qsIsAvailable = !isRemote;

        // Add repo
        addRepo(newRepo);

        return newRepo;
    }
}
