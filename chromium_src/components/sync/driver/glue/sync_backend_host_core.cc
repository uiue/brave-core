#include "../../../../../../components/sync/driver/glue/sync_backend_host_core.cc"

namespace syncer {

void SyncBackendHostCore::OnNudgeSyncCycle() {
  host_.Call(FROM_HERE,
             &SyncBackendHostImpl::HandleNudgeSyncCycle);
}

void SyncBackendHostCore::OnPollSyncCycle() {
  host_.Call(FROM_HERE,
             &SyncBackendHostImpl::HandlePollSyncCycle);
}

}  // namespace syncer
