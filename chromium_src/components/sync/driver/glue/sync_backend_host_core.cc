#include "../../../../../../components/sync/driver/glue/sync_backend_host_core.cc"

#include "brave/components/brave_sync/jslib_messages.h"

namespace syncer {

void SyncBackendHostCore::OnNudgeSyncCycle() {
  host_.Call(FROM_HERE,
             &SyncBackendHostImpl::HandleNudgeSyncCycle);
}

void SyncBackendHostCore::OnPollSyncCycle(GetRecordsCallback cb,
                                          base::WaitableEvent* wevent) {
  host_.Call(FROM_HERE,
             &SyncBackendHostImpl::HandlePollSyncCycle, cb, wevent);
}

void SyncBackendHostCore::DoDispatchGetRecordsCallback(
    GetRecordsCallback cb, std::unique_ptr<RecordsList> records) {
  cb.Run(std::move(records));
}

}  // namespace syncer
