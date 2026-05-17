#include "dicom/EchoScu.h"

#include "dicom/EchoScuError.h"
#include "domain/AeTitle.h"
#include "domain/DicomNode.h"
#include "domain/DicomStatus.h"
#include "domain/EchoResult.h"

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcuid.h>
#include <dcmtk/dcmnet/assoc.h>
#include <dcmtk/dcmnet/dimse.h>
#include <dcmtk/ofstd/ofcond.h>

#include <cstdint>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace vna::dicom {

namespace {

std::string conditionText(const OFCondition& cond) {
    return std::string{cond.text()};
}

}  // namespace

domain::EchoResult EchoScu::echo(const std::string& callingAe,
                                 const domain::DicomNode& peer,
                                 std::uint32_t timeoutSeconds) const {
    try {
        domain::requireValidAeTitle(callingAe, "calling AE title");
    } catch (const std::invalid_argument& e) {
        throw EchoScuError(e.what());
    }
    if (timeoutSeconds == 0) {
        throw EchoScuError("timeout must be positive");
    }
    if (timeoutSeconds >
        static_cast<std::uint32_t>(std::numeric_limits<int>::max())) {
        throw EchoScuError(
            "timeout (" + std::to_string(timeoutSeconds) +
            " seconds) exceeds the maximum supported value of " +
            std::to_string(std::numeric_limits<int>::max()) + " seconds");
    }
    const int timeoutInt = static_cast<int>(timeoutSeconds);

    T_ASC_Network* net = nullptr;
    OFCondition cond = ASC_initializeNetwork(NET_REQUESTOR, 0, timeoutInt, &net);
    if (cond.bad() || net == nullptr) {
        throw EchoScuError("failed to initialize DICOM network: " + conditionText(cond));
    }

    T_ASC_Parameters* params = nullptr;
    cond = ASC_createAssociationParameters(&params, ASC_DEFAULTMAXPDU,
                                           static_cast<Sint32>(timeoutInt));
    if (cond.bad() || params == nullptr) {
        ASC_dropNetwork(&net);
        throw EchoScuError("failed to create association parameters: " + conditionText(cond));
    }

    ASC_setAPTitles(params, callingAe.c_str(), peer.aeTitle().c_str(), nullptr);

    char peerAddress[256];
    std::snprintf(peerAddress, sizeof(peerAddress), "%s:%u",
                  peer.host().c_str(),
                  static_cast<unsigned int>(peer.port()));
    cond = ASC_setPresentationAddresses(params, "localhost", peerAddress);
    if (cond.bad()) {
        ASC_destroyAssociationParameters(&params);
        ASC_dropNetwork(&net);
        throw EchoScuError("failed to set presentation addresses: " + conditionText(cond));
    }

    const char* transferSyntaxes[3] = {
        UID_LittleEndianExplicitTransferSyntax,
        UID_BigEndianExplicitTransferSyntax,
        UID_LittleEndianImplicitTransferSyntax,
    };
    cond = ASC_addPresentationContext(params, 1, UID_VerificationSOPClass,
                                      transferSyntaxes, 3);
    if (cond.bad()) {
        ASC_destroyAssociationParameters(&params);
        ASC_dropNetwork(&net);
        throw EchoScuError(
            "failed to add Verification presentation context: " + conditionText(cond));
    }

    T_ASC_Association* assoc = nullptr;
    cond = ASC_requestAssociation(net, params, &assoc);
    if (cond.bad()) {
        const std::string msg = "association request failed: " + conditionText(cond);
        if (assoc != nullptr) {
            ASC_destroyAssociation(&assoc);
        } else {
            ASC_destroyAssociationParameters(&params);
        }
        ASC_dropNetwork(&net);
        throw EchoScuError(msg);
    }

    if (ASC_countAcceptedPresentationContexts(params) == 0) {
        ASC_abortAssociation(assoc);
        ASC_destroyAssociation(&assoc);
        ASC_dropNetwork(&net);
        throw EchoScuError("peer did not accept the Verification SOP Class");
    }

    DIC_US msgId = assoc->nextMsgID++;
    DIC_US dicomStatusCode = 0;
    DcmDataset* statusDetail = nullptr;
    cond = DIMSE_echoUser(assoc, msgId, DIMSE_BLOCKING, timeoutInt,
                          &dicomStatusCode, &statusDetail);
    if (statusDetail != nullptr) {
        delete statusDetail;
        statusDetail = nullptr;
    }
    if (cond.bad()) {
        const std::string msg = "C-ECHO request failed: " + conditionText(cond);
        ASC_abortAssociation(assoc);
        ASC_destroyAssociation(&assoc);
        ASC_dropNetwork(&net);
        throw EchoScuError(msg);
    }

    const OFCondition releaseCond = ASC_releaseAssociation(assoc);
    ASC_destroyAssociation(&assoc);
    ASC_dropNetwork(&net);
    if (releaseCond.bad()) {
        throw EchoScuError("association release failed: " + conditionText(releaseCond));
    }

    const domain::DicomStatus status{static_cast<std::uint16_t>(dicomStatusCode)};
    std::string message;
    if (status.isSuccess()) {
        message = "C-ECHO successful";
    } else {
        char codeBuf[16];
        std::snprintf(codeBuf, sizeof(codeBuf), "0x%04X",
                      static_cast<unsigned int>(dicomStatusCode));
        message = std::string{"C-ECHO returned non-success status "} + codeBuf;
    }
    return domain::EchoResult{status, std::move(message)};
}

}  // namespace vna::dicom
