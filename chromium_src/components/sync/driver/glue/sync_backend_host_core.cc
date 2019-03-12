#include "../../../../../../components/sync/driver/glue/sync_backend_host_core.cc"

namespace syncer {

void SyncBackendHostCore::OnNudgeSyncCycle() {
  host_.Call(FROM_HERE,
             &SyncBackendHostImpl::HandleNudgeSyncCycle);
}

void SyncBackendHostCore::OnPollSyncCycle(brave_sync::GetRecordsCallback cb) {
  host_.Call(FROM_HERE,
             &SyncBackendHostImpl::HandlePollSyncCycle, cb);
}

}  // namespace syncer
