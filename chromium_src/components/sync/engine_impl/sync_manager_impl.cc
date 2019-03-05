#include "../../../../../components/sync/engine_impl/sync_manager_impl.cc"

namespace syncer {

void SyncManagerImpl::OnNudgeSyncCycle() {
  for (auto& observer : observers_) {
    observer.OnNudgeSyncCycle();
  }
}

void SyncManagerImpl::OnPollSyncCycle() {
  for (auto& observer : observers_) {
    observer.OnPollSyncCycle();
  }
}

}  // namespace syncer
