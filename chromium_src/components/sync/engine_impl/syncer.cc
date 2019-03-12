#include "brave/components/brave_sync/jslib_messages.h"
#include "../../../../../components/sync/engine_impl/syncer.cc"

namespace syncer {

using brave_sync::RecordsList;

void Syncer::OnGetRecords(std::unique_ptr<RecordsList> records) {
  LOG(ERROR) << __func__;
}

}  // namespace syncer

