#include "components/sync/base/model_type.h"
namespace sync_pb {
class GetUpdatesResponse;
}

namespace syncer {
namespace {
void InitFakeUpdateResponse(sync_pb::GetUpdatesResponse*, ModelTypeSet*);
}
}
#include "../../../../../components/sync/engine_impl/get_updates_processor.cc"

namespace syncer {
namespace {

void InitFakeUpdateResponse(sync_pb::GetUpdatesResponse* gu_response,
                            ModelTypeSet* request_types) {
  DCHECK(gu_response);
  DCHECK(request_types);
  for (ModelType type : *request_types) {
    sync_pb::DataTypeProgressMarker* marker =
      gu_response->add_new_progress_marker();
    marker->set_data_type_id(GetSpecificsFieldNumberFromModelType(type));
    marker->set_token("foobarbaz");
    sync_pb::DataTypeContext* context = gu_response->add_context_mutations();
    context->set_data_type_id(GetSpecificsFieldNumberFromModelType(type));
    context->set_version(1);
    context->set_context("context");
    }
  gu_response->set_changes_remaining(0);
}

}
}
