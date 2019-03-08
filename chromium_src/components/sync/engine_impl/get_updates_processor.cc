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
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "components/sync/syncable/syncable_proto_util.h"
#include "url/gurl.h"

namespace syncer {
namespace {

using syncable::Id;
static const char kCacheGuid[] = "IrcjZ2jyzHDV9Io4+zKcXQ==";
const char kBookmarkBarTag[] = "bookmark_bar";
const char kOtherBookmarksTag[] = "other_bookmarks";
#if 0
void AddBookmarkSpecifics(sync_pb::EntitySpecifics* specifics,
                          const std::string& title,
                          const GURL& url) {
  DCHECK(specifics);
  sync_pb::BookmarkSpecifics* bm_specifics = specifics->mutable_bookmark();
  bm_specifics->set_url(url.spec());
  bm_specifics->set_title(title);
  bm_specifics->set_creation_time_us(
      base::Time::Now().ToDeltaSinceWindowsEpoch().InMicroseconds());
}
#endif

void AddPermanentBookmarkSpecifics(sync_pb::EntitySpecifics* specifics,
                                   const char* tag) {
  DCHECK(specifics);
  sync_pb::BookmarkSpecifics* bm_specifics = specifics->mutable_bookmark();
  bm_specifics->set_title(tag);
  bm_specifics->set_creation_time_us(
      base::Time::Now().ToDeltaSinceWindowsEpoch().InMicroseconds());
}

void AddRootForType(sync_pb::SyncEntity* entity, ModelType type) {
  DCHECK(entity);
  sync_pb::EntitySpecifics specifics;
  AddDefaultFieldValue(type, &specifics);
  std::string root = Id::GetRoot().GetServerId();
  std::string id = ModelTypeToRootTag(type);
  entity->set_server_defined_unique_tag(id);
  entity->set_deleted(false);
  entity->set_id_string(id);
  entity->set_parent_id_string(root);
  entity->set_non_unique_name(id);
  entity->set_name(id);
  entity->set_version(1000);
  entity->set_folder(true);
  entity->mutable_specifics()->CopyFrom(specifics);
  entity->set_originator_cache_guid(
    std::string(kCacheGuid, base::size(kCacheGuid) - 1));
  Id client_id = Id::CreateFromClientString("-2");
  entity->set_originator_client_item_id(client_id.GetServerId());
  entity->set_position_in_parent(0);
}

void AddPermanentNode(sync_pb::SyncEntity* entity, const char* tag) {
  DCHECK(entity);
  sync_pb::EntitySpecifics specifics;
  AddDefaultFieldValue(BOOKMARKS, &specifics);
  std::string root = ModelTypeToRootTag(BOOKMARKS);
  std::string id = std::string(tag);
  entity->set_server_defined_unique_tag(id);
  entity->set_id_string(id);
  entity->set_parent_id_string(root);
  entity->set_non_unique_name(id);
  entity->set_name(id);
  entity->set_folder(true);
  entity->set_version(1000);
  entity->set_originator_cache_guid(
    std::string(kCacheGuid, base::size(kCacheGuid) - 1));
  Id client_id = Id::CreateFromClientString("-2");
  entity->set_originator_client_item_id(client_id.GetServerId());
  entity->set_position_in_parent(0);
  entity->set_ctime(TimeToProtoTime(base::Time::Now()));
  entity->set_mtime(TimeToProtoTime(base::Time::Now()));
  AddPermanentBookmarkSpecifics(&specifics, tag);
  entity->mutable_specifics()->CopyFrom(specifics);
}
#if 0
void AddBookmarkNode(sync_pb::SyncEntity* entity) {
  DCHECK(entity);
  sync_pb::EntitySpecifics specifics;
  AddDefaultFieldValue(BOOKMARKS, &specifics);
  std::string root = std::string(kBookmarkBarTag);
  std::string id ("BRAVE!!!");
  entity->set_id_string(id);
  entity->set_parent_id_string(root);
  entity->set_non_unique_name(id);
  entity->set_name(id);
  entity->set_folder(false);
  entity->set_version(1000);
  entity->set_originator_cache_guid(
    std::string(kCacheGuid, base::size(kCacheGuid) - 1));
  Id client_id = Id::CreateFromClientString("-3");
  entity->set_originator_client_item_id(client_id.GetServerId());
  entity->set_position_in_parent(0);
  entity->set_ctime(TimeToProtoTime(base::Time::Now()));
  entity->set_mtime(TimeToProtoTime(base::Time::Now()));
  AddBookmarkSpecifics(&specifics, std::string("BRAVE"),
                      GURL("https://brave.com"));
  entity->mutable_specifics()->CopyFrom(specifics);
}
#endif

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
    if (type == BOOKMARKS) {
      google::protobuf::RepeatedPtrField<sync_pb::SyncEntity> entities;
      AddRootForType(entities.Add(), BOOKMARKS);
      AddPermanentNode(entities.Add(), kBookmarkBarTag);
      AddPermanentNode(entities.Add(), kOtherBookmarksTag);
      // AddBookmarkNode(entities.Add());
      std::copy(entities.begin(), entities.end(),
                RepeatedPtrFieldBackInserter(gu_response->mutable_entries()));
    }
  }
  gu_response->set_changes_remaining(0);
}

}
}
