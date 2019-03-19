#include "../../../../../components/sync/engine_impl/sync_scheduler_impl.cc"

namespace syncer {

void SyncSchedulerImpl::TryBraveSyncCycleJob() {
  SyncCycle cycle(cycle_context_, this);
  if (mode_ != CONFIGURATION_MODE && mode_ != CLEAR_SERVER_DATA_MODE) {
      syncer_->DownloadBraveRecords(&cycle);
  }
}

}  // namespace syncer
