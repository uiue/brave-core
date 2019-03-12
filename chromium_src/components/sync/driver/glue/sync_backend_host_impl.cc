#include "../../../../../../components/sync/driver/glue/sync_backend_host_impl.cc"

namespace syncer {

void SyncBackendHostImpl::HandleNudgeSyncCycle() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(nudge_sync_cycle_delegate_function_);
  nudge_sync_cycle_delegate_function_.Run();
}

void SyncBackendHostImpl::HandlePollSyncCycle(
    brave_sync::GetRecordsCallback cb) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(poll_sync_cycle_delegate_function_);
  poll_sync_cycle_delegate_function_.Run(cb);
}

}  // namespace syncer
