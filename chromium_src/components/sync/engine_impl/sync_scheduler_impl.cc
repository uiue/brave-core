#include "../../../../../components/sync/engine_impl/sync_scheduler_impl.cc"

namespace syncer {

void SyncSchedulerImpl::TryBraveSyncCycleJob() {
  SyncCycle cycle(cycle_context_, this);
  syncer_->DownloadBraveRecords(&cycle);
}

}  // namespace syncer
