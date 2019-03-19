#include "components/sync/engine_impl/get_updates_processor.h"
namespace sync_pb {
class ClientToServerResponse;
}  // namespace sync_pb

namespace syncer {
class SyncerError;
namespace {
SyncerError ApplyBraveRecords(sync_pb::ClientToServerResponse*, ModelTypeSet*,
                              std::unique_ptr<brave_sync::RecordsList>);
}   // namespace
}   // namespace syncer
#include "../../../../../components/sync/engine_impl/get_updates_processor.cc"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "brave/components/brave_sync/jslib_messages.h"
#include "components/sync/syncable/syncable_proto_util.h"
#include "url/gurl.h"

namespace syncer {
namespace {

using brave_sync::jslib::SyncRecord;
using syncable::Id;
static const char kCacheGuid[] = "IrcjZ2jyzHDV9Io4+zKcXQ==";
const char kBookmarkBarTag[] = "bookmark_bar";
const char kOtherBookmarksTag[] = "other_bookmarks";

uint64_t GetIndexByOrder(const std::string& record_order) {
  uint64_t index = 0;
  char last_ch = record_order.back();
  bool result = base::StringToUint64(std::string(&last_ch), &index);
  DCHECK(result);
  return index;
}

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

void AddBookmarkNode(sync_pb::SyncEntity* entity, const SyncRecord* record) {
  DCHECK(entity);
  DCHECK(record);
  DCHECK(record->has_bookmark());
  DCHECK(!record->objectId.empty());

  auto bookmark_record = record->GetBookmark();

  sync_pb::EntitySpecifics specifics;
  AddDefaultFieldValue(BOOKMARKS, &specifics);
  if (record->action == SyncRecord::Action::A_UPDATE) {
    // TODO(darkdh): requires SyncEntity.version to be set correctly
  } else if (record->action == SyncRecord::Action::A_DELETE) {
    // TODO(darkdh): make bookmark specific empty
  } else if (record->action == SyncRecord::Action::A_CREATE) {
    entity->set_id_string(record->objectId);
    if (!bookmark_record.parentFolderObjectId.empty())
      entity->set_parent_id_string(bookmark_record.parentFolderObjectId);
    else
      entity->set_parent_id_string(std::string(kBookmarkBarTag));
    entity->set_non_unique_name(bookmark_record.site.title);
    entity->set_folder(bookmark_record.isFolder);
    // TODO(darkdh): set version, originator_cache_guid and
    // originator_client_item_id in meta info in CommitMessage
    entity->set_version(1000);
    entity->set_originator_cache_guid(
      std::string(kCacheGuid, base::size(kCacheGuid) - 1));
    Id client_id = Id::CreateFromClientString(record->objectId);
    entity->set_originator_client_item_id(client_id.GetServerId());
    entity->set_position_in_parent(GetIndexByOrder(bookmark_record.order));
    entity->set_ctime(TimeToProtoTime(base::Time::Now()));
    entity->set_mtime(TimeToProtoTime(base::Time::Now()));
    AddBookmarkSpecifics(&specifics, bookmark_record.site.title,
                         GURL(bookmark_record.site.location));
    entity->mutable_specifics()->CopyFrom(specifics);
  }
}

void ConstructUpdateResponse(sync_pb::GetUpdatesResponse* gu_response,
                             ModelTypeSet* request_types,
                             std::unique_ptr<RecordsList> records) {
  DCHECK(gu_response);
  DCHECK(request_types);
  for (ModelType type : *request_types) {
    sync_pb::DataTypeProgressMarker* marker =
      gu_response->add_new_progress_marker();
    marker->set_data_type_id(GetSpecificsFieldNumberFromModelType(type));
    marker->set_token("token");
    sync_pb::DataTypeContext* context = gu_response->add_context_mutations();
    context->set_data_type_id(GetSpecificsFieldNumberFromModelType(type));
    context->set_version(1);
    context->set_context("context");
    if (type == BOOKMARKS) {
      google::protobuf::RepeatedPtrField<sync_pb::SyncEntity> entities;
      AddRootForType(entities.Add(), BOOKMARKS);
      AddPermanentNode(entities.Add(), kBookmarkBarTag);
      AddPermanentNode(entities.Add(), kOtherBookmarksTag);
      if (records) {
        for (const auto& record : *records.get()) {
          AddBookmarkNode(entities.Add(), record.get());
        }
      }
      std::copy(entities.begin(), entities.end(),
                RepeatedPtrFieldBackInserter(gu_response->mutable_entries()));
    }
    gu_response->set_changes_remaining(0);
  }
}

SyncerError ApplyBraveRecords(sync_pb::ClientToServerResponse* update_response,
                              ModelTypeSet* request_types,
                              std::unique_ptr<RecordsList> records) {
  DCHECK(update_response);
  DCHECK(request_types);
  sync_pb::GetUpdatesResponse* gu_response = new sync_pb::GetUpdatesResponse();
  ConstructUpdateResponse(gu_response, request_types, std::move(records));
  update_response->set_allocated_get_updates(gu_response);
  return SyncerError(SyncerError::SYNCER_OK);
}

}   // namespace

void GetUpdatesProcessor::AddBraveRecords(
    std::unique_ptr<RecordsList> records) {
  brave_records_ = std::move(records);
}

}   // namespace syncer
