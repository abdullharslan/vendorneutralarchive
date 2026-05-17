#include "dicom/StoreScp.h"

#include "dicom/StoreScpError.h"
#include "domain/AeTitle.h"
#include "domain/DicomUid.h"
#include "storage/InstanceStorage.h"
#include "storage/StorageError.h"

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcuid.h>
#include <dcmtk/dcmdata/dcxfer.h>
#include <dcmtk/dcmnet/assoc.h>
#include <dcmtk/dcmnet/dimse.h>
#include <dcmtk/ofstd/ofcond.h>
#include <dcmtk/ofstd/ofstring.h>

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace vna::dicom {

namespace {

using LogFn = std::function<void(const std::string&)>;

const char* kSupportedAbstractSyntaxes[] = {
    UID_VerificationSOPClass,
    UID_SecondaryCaptureImageStorage,
    UID_CTImageStorage,
    UID_MRImageStorage,
    UID_ComputedRadiographyImageStorage,
    UID_DigitalXRayImageStorageForPresentation,
    UID_UltrasoundImageStorage,
};
constexpr int kSupportedAbstractSyntaxesCount =
    static_cast<int>(sizeof(kSupportedAbstractSyntaxes) /
                     sizeof(kSupportedAbstractSyntaxes[0]));

const char* kAcceptedTransferSyntaxes[] = {
    UID_LittleEndianExplicitTransferSyntax,
    UID_BigEndianExplicitTransferSyntax,
    UID_LittleEndianImplicitTransferSyntax,
};
constexpr int kAcceptedTransferSyntaxesCount =
    static_cast<int>(sizeof(kAcceptedTransferSyntaxes) /
                     sizeof(kAcceptedTransferSyntaxes[0]));

constexpr std::uint16_t kStatusSuccess              = 0x0000;
constexpr std::uint16_t kStatusProcessingFailure    = 0xC001;
constexpr std::uint16_t kStatusOutOfResources       = 0xA700;
constexpr std::uint16_t kStatusDataSetDoesNotMatch  = 0xA900;

constexpr int kAcceptPollSeconds = 1;

std::string conditionText(const OFCondition& cond) {
    return std::string{cond.text()};
}

std::string formatStatusCode(std::uint16_t code) {
    char buf[8];
    std::snprintf(buf, sizeof(buf), "0x%04X", static_cast<unsigned int>(code));
    return std::string{buf};
}

OFCondition sendCEchoResponse(T_ASC_Association* assoc,
                              T_ASC_PresentationContextID presID,
                              const T_DIMSE_C_EchoRQ& req) {
    T_DIMSE_Message rspMsg;
    std::memset(&rspMsg, 0, sizeof(rspMsg));
    rspMsg.CommandField = DIMSE_C_ECHO_RSP;
    T_DIMSE_C_EchoRSP& rsp = rspMsg.msg.CEchoRSP;
    rsp.MessageIDBeingRespondedTo = req.MessageID;
    rsp.DimseStatus = STATUS_Success;
    rsp.DataSetType = DIMSE_DATASET_NULL;
    std::strncpy(rsp.AffectedSOPClassUID,
                 req.AffectedSOPClassUID, DIC_UI_LEN);
    rsp.AffectedSOPClassUID[DIC_UI_LEN] = '\0';
    rsp.opts = O_ECHO_AFFECTEDSOPCLASSUID;
    return DIMSE_sendMessageUsingMemoryData(
        assoc, presID, &rspMsg, nullptr, nullptr, nullptr, nullptr);
}

OFCondition sendCStoreResponse(T_ASC_Association* assoc,
                               T_ASC_PresentationContextID presID,
                               const T_DIMSE_C_StoreRQ& req,
                               std::uint16_t status) {
    T_DIMSE_Message rspMsg;
    std::memset(&rspMsg, 0, sizeof(rspMsg));
    rspMsg.CommandField = DIMSE_C_STORE_RSP;
    T_DIMSE_C_StoreRSP& rsp = rspMsg.msg.CStoreRSP;
    rsp.MessageIDBeingRespondedTo = req.MessageID;
    rsp.DimseStatus = status;
    rsp.DataSetType = DIMSE_DATASET_NULL;
    std::strncpy(rsp.AffectedSOPClassUID,
                 req.AffectedSOPClassUID, DIC_UI_LEN);
    rsp.AffectedSOPClassUID[DIC_UI_LEN] = '\0';
    std::strncpy(rsp.AffectedSOPInstanceUID,
                 req.AffectedSOPInstanceUID, DIC_UI_LEN);
    rsp.AffectedSOPInstanceUID[DIC_UI_LEN] = '\0';
    rsp.opts = O_STORE_AFFECTEDSOPCLASSUID | O_STORE_AFFECTEDSOPINSTANCEUID;
    return DIMSE_sendMessageUsingMemoryData(
        assoc, presID, &rspMsg, nullptr, nullptr, nullptr, nullptr);
}

E_TransferSyntax negotiatedTransferSyntax(T_ASC_Association* assoc,
                                          T_ASC_PresentationContextID presID) {
    T_ASC_PresentationContext pc;
    std::memset(&pc, 0, sizeof(pc));
    const OFCondition cond = ASC_findAcceptedPresentationContext(
        assoc->params, presID, &pc);
    if (cond.bad()) {
        return EXS_LittleEndianExplicit;
    }
    DcmXfer xfer{pc.acceptedTransferSyntax};
    const E_TransferSyntax xs = xfer.getXfer();
    return (xs == EXS_Unknown) ? EXS_LittleEndianExplicit : xs;
}

std::uint16_t handleCStore(T_ASC_Association* assoc,
                           T_ASC_PresentationContextID presID,
                           const T_DIMSE_C_StoreRQ& req,
                           int timeoutInt,
                           storage::InstanceStorage& storage,
                           const LogFn& log) {
    DcmDataset* dataset = nullptr;
    OFCondition cond = DIMSE_receiveDataSetInMemory(
        assoc, DIMSE_BLOCKING, timeoutInt, &presID,
        &dataset, nullptr, nullptr);

    if (cond.bad() || dataset == nullptr) {
        log("C-STORE: failed to receive dataset: " + conditionText(cond));
        sendCStoreResponse(assoc, presID, req, kStatusProcessingFailure);
        delete dataset;
        return kStatusProcessingFailure;
    }

    OFString sopInstanceUidOf;
    cond = dataset->findAndGetOFString(DCM_SOPInstanceUID, sopInstanceUidOf);
    if (cond.bad() || sopInstanceUidOf.empty()) {
        log("C-STORE: received dataset is missing SOP Instance UID");
        sendCStoreResponse(assoc, presID, req, kStatusProcessingFailure);
        delete dataset;
        return kStatusProcessingFailure;
    }
    const std::string sopInstanceUid{sopInstanceUidOf.c_str()};

    OFString sopClassUidOf;
    cond = dataset->findAndGetOFString(DCM_SOPClassUID, sopClassUidOf);
    if (cond.bad() || sopClassUidOf.empty()) {
        log("C-STORE: received dataset is missing SOP Class UID");
        sendCStoreResponse(assoc, presID, req, kStatusProcessingFailure);
        delete dataset;
        return kStatusProcessingFailure;
    }
    const std::string sopClassUid{sopClassUidOf.c_str()};

    const std::string requestSopInstanceUid{req.AffectedSOPInstanceUID};
    if (requestSopInstanceUid != sopInstanceUid) {
        log("C-STORE: request Affected SOP Instance UID '" +
            requestSopInstanceUid +
            "' does not match dataset SOP Instance UID '" +
            sopInstanceUid + "'");
        sendCStoreResponse(assoc, presID, req, kStatusDataSetDoesNotMatch);
        delete dataset;
        return kStatusDataSetDoesNotMatch;
    }

    const std::string requestSopClassUid{req.AffectedSOPClassUID};
    if (requestSopClassUid != sopClassUid) {
        log("C-STORE: request Affected SOP Class UID '" +
            requestSopClassUid +
            "' does not match dataset SOP Class UID '" +
            sopClassUid + "'");
        sendCStoreResponse(assoc, presID, req, kStatusDataSetDoesNotMatch);
        delete dataset;
        return kStatusDataSetDoesNotMatch;
    }

    if (!domain::isValidDicomUid(sopInstanceUid)) {
        log("C-STORE: dataset SOP Instance UID is not a valid DICOM UID: \"" +
            sopInstanceUid + "\"");
        sendCStoreResponse(assoc, presID, req, kStatusProcessingFailure);
        delete dataset;
        return kStatusProcessingFailure;
    }

    std::string targetPath;
    try {
        targetPath = storage.prepareDestination(sopInstanceUid);
    } catch (const storage::StorageError& e) {
        log(std::string{"C-STORE: storage refused destination: "} + e.what());
        sendCStoreResponse(assoc, presID, req, kStatusOutOfResources);
        delete dataset;
        return kStatusOutOfResources;
    }

    const E_TransferSyntax writeXfer = negotiatedTransferSyntax(assoc, presID);

    // DcmFileFormat takes ownership of the dataset and frees it on destruction.
    DcmFileFormat fileFormat{dataset};
    const OFCondition saveCond = fileFormat.saveFile(
        targetPath.c_str(), writeXfer,
        EET_ExplicitLength, EGL_recalcGL, EPD_withoutPadding,
        0, 0, EWM_fileformat);
    if (saveCond.bad()) {
        log("C-STORE: failed to write '" + targetPath + "': " +
            conditionText(saveCond));
        sendCStoreResponse(assoc, presID, req, kStatusProcessingFailure);
        return kStatusProcessingFailure;
    }

    if (!storage.isPersisted(targetPath)) {
        log("C-STORE: post-write verification failed for '" + targetPath + "'");
        sendCStoreResponse(assoc, presID, req, kStatusProcessingFailure);
        return kStatusProcessingFailure;
    }

    log("C-STORE: persisted " + sopInstanceUid + " to " + targetPath);
    const OFCondition rspCond = sendCStoreResponse(
        assoc, presID, req, kStatusSuccess);
    if (rspCond.bad()) {
        log("C-STORE: persisted but failed to send response: " +
            conditionText(rspCond));
        return kStatusProcessingFailure;
    }
    return kStatusSuccess;
}

void handleAssociation(T_ASC_Association* assoc,
                       int timeoutInt,
                       const std::string& localAeTitle,
                       storage::InstanceStorage& storage,
                       const LogFn& log,
                       const std::atomic<bool>& shutdown) {
    char callingAE[DIC_AE_LEN + 2];
    char calledAE[DIC_AE_LEN + 2];
    char respondingAE[DIC_AE_LEN + 2];
    std::memset(callingAE, 0, sizeof(callingAE));
    std::memset(calledAE, 0, sizeof(calledAE));
    std::memset(respondingAE, 0, sizeof(respondingAE));
    OFCondition cond = ASC_getAPTitles(
        assoc->params,
        callingAE, sizeof(callingAE),
        calledAE, sizeof(calledAE),
        respondingAE, sizeof(respondingAE));
    if (cond.bad()) {
        log("Association: failed to read AE titles: " + conditionText(cond));
        ASC_abortAssociation(assoc);
        ASC_destroyAssociation(&assoc);
        return;
    }

    if (localAeTitle != std::string{calledAE}) {
        log("Association rejected: called AE '" + std::string{calledAE} +
            "' does not match local AE '" + localAeTitle + "'");
        T_ASC_RejectParameters reject;
        std::memset(&reject, 0, sizeof(reject));
        reject.result = ASC_RESULT_REJECTEDPERMANENT;
        reject.source = ASC_SOURCE_SERVICEUSER;
        reject.reason = ASC_REASON_SU_CALLEDAETITLENOTRECOGNIZED;
        ASC_rejectAssociation(assoc, &reject);
        ASC_destroyAssociation(&assoc);
        return;
    }

    cond = ASC_acceptContextsWithPreferredTransferSyntaxes(
        assoc->params,
        kSupportedAbstractSyntaxes, kSupportedAbstractSyntaxesCount,
        kAcceptedTransferSyntaxes, kAcceptedTransferSyntaxesCount);
    if (cond.bad()) {
        log("Association: failed to negotiate presentation contexts: " +
            conditionText(cond));
        ASC_abortAssociation(assoc);
        ASC_destroyAssociation(&assoc);
        return;
    }

    cond = ASC_acknowledgeAssociation(assoc);
    if (cond.bad()) {
        log("Association: failed to acknowledge: " + conditionText(cond));
        ASC_abortAssociation(assoc);
        ASC_destroyAssociation(&assoc);
        return;
    }

    log("Association accepted: calling='" + std::string{callingAE} +
        "', called='" + std::string{calledAE} + "'");

    bool releaseRequested = false;
    bool peerAborted = false;
    while (!shutdown.load(std::memory_order_relaxed)) {
        T_DIMSE_Message msg;
        std::memset(&msg, 0, sizeof(msg));
        T_ASC_PresentationContextID presID = 0;
        DcmDataset* statusDetail = nullptr;
        cond = DIMSE_receiveCommand(assoc, DIMSE_BLOCKING, timeoutInt,
                                    &presID, &msg, &statusDetail);
        if (statusDetail != nullptr) {
            delete statusDetail;
            statusDetail = nullptr;
        }

        if (cond == DUL_PEERREQUESTEDRELEASE) {
            releaseRequested = true;
            break;
        }
        if (cond == DUL_PEERABORTEDASSOCIATION) {
            peerAborted = true;
            log("Association: peer aborted");
            break;
        }
        if (cond.bad()) {
            log("Association: receive command failed: " + conditionText(cond));
            break;
        }

        if (msg.CommandField == DIMSE_C_ECHO_RQ) {
            log("Received C-ECHO request");
            const OFCondition sc = sendCEchoResponse(assoc, presID,
                                                    msg.msg.CEchoRQ);
            if (sc.bad()) {
                log("C-ECHO: failed to send response: " + conditionText(sc));
                break;
            }
        } else if (msg.CommandField == DIMSE_C_STORE_RQ) {
            const std::uint16_t status = handleCStore(
                assoc, presID, msg.msg.CStoreRQ, timeoutInt, storage, log);
            log("C-STORE: returned status " + formatStatusCode(status));
        } else {
            log("Association: unsupported DIMSE command; aborting");
            ASC_abortAssociation(assoc);
            ASC_destroyAssociation(&assoc);
            return;
        }
    }

    if (releaseRequested) {
        ASC_acknowledgeRelease(assoc);
    } else if (!peerAborted) {
        ASC_abortAssociation(assoc);
    }
    ASC_destroyAssociation(&assoc);
}

}  // namespace

StoreScp::StoreScp(StoreScpConfig config, storage::InstanceStorage& storage)
    : config_(std::move(config)), storage_(&storage),
      log_([](const std::string&){}) {
    try {
        domain::requireValidAeTitle(config_.aeTitle, "StoreScp AE title");
    } catch (const std::invalid_argument& e) {
        throw StoreScpError(e.what());
    }
    if (config_.port == 0) {
        throw StoreScpError("StoreScp port must not be zero");
    }
    if (config_.timeoutSeconds == 0) {
        throw StoreScpError("StoreScp timeout must be positive");
    }
    if (config_.timeoutSeconds >
        static_cast<std::uint32_t>(std::numeric_limits<int>::max())) {
        throw StoreScpError(
            "StoreScp timeout (" + std::to_string(config_.timeoutSeconds) +
            " seconds) exceeds the maximum supported value of " +
            std::to_string(std::numeric_limits<int>::max()) + " seconds");
    }
}

void StoreScp::setLogCallback(LogCallback callback) {
    log_ = callback ? std::move(callback) : LogCallback{[](const std::string&){}};
}

void StoreScp::requestShutdown() noexcept {
    shutdown_.store(true, std::memory_order_relaxed);
}

void StoreScp::run() {
    const int timeoutInt = static_cast<int>(config_.timeoutSeconds);

    T_ASC_Network* net = nullptr;
    OFCondition cond = ASC_initializeNetwork(
        NET_ACCEPTOR, static_cast<int>(config_.port),
        kAcceptPollSeconds, &net);
    if (cond.bad() || net == nullptr) {
        throw StoreScpError("failed to initialize DICOM network on port " +
                            std::to_string(config_.port) + ": " +
                            conditionText(cond));
    }

    // DCMTK 3.7.0's ASC_initializeNetwork has no API to bind to a specific
    // host; the listening socket always binds to all interfaces. We log the
    // configured host as the intended/advertised address only.
    log_("StoreScp: listening as AE '" + config_.aeTitle + "' on port " +
         std::to_string(config_.port) +
         " (all interfaces; advertised host '" + config_.host + "')");

    while (!shutdown_.load(std::memory_order_relaxed)) {
        T_ASC_Association* assoc = nullptr;
        cond = ASC_receiveAssociation(
            net, &assoc, ASC_DEFAULTMAXPDU,
            nullptr, nullptr, OFFalse,
            DUL_NOBLOCK, kAcceptPollSeconds);
        if (shutdown_.load(std::memory_order_relaxed)) {
            if (assoc != nullptr) {
                ASC_abortAssociation(assoc);
                ASC_destroyAssociation(&assoc);
            }
            break;
        }
        if (cond.bad()) {
            if (assoc != nullptr) {
                ASC_destroyAssociation(&assoc);
            }
            continue;
        }
        if (assoc == nullptr) {
            continue;
        }
        handleAssociation(assoc, timeoutInt, config_.aeTitle, *storage_,
                          log_, shutdown_);
    }

    ASC_dropNetwork(&net);
    log_("StoreScp: stopped");
}

}  // namespace vna::dicom
