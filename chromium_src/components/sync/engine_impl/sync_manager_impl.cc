#include "../../../../../components/sync/engine_impl/sync_manager_impl.cc"
#include "brave/components/brave_sync/jslib_messages.h"

namespace syncer {

void SyncManagerImpl::OnNudgeSyncCycle() {
  for (auto& observer : observers_) {
    observer.OnNudgeSyncCycle();
  }
}

void SyncManagerImpl::OnPollSyncCycle(brave_sync::GetRecordsCallback cb,
                                      base::WaitableEvent* wevent) {
  for (auto& observer : observers_) {
    observer.OnPollSyncCycle(cb, wevent);
  }
}

}  // namespace syncer
