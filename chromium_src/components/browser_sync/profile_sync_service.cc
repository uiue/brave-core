#include "../../../../components/browser_sync/profile_sync_service.cc"

#include "brave/components/brave_sync/brave_sync_prefs.h"
#include "content/public/browser/browser_thread.h"

namespace browser_sync {

using brave_sync::RecordsList;
using brave_sync::Uint8Array;

void ProfileSyncService::BackgroundSyncStarted(bool startup) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
#if 0
  if (startup)
    bookmark_change_processor_->Start();

  StartLoop();
#endif
}

void ProfileSyncService::BackgroundSyncStopped(bool shutdown) {
#if 0
  if (shutdown)
    Shutdown();
  else
    StopLoop();
#endif
}

void ProfileSyncService::OnSyncDebug(const std::string& message) {
#if 0
  NotifyLogMessage(message);
#endif
}

void ProfileSyncService::OnSyncSetupError(const std::string& error) {
#if 0
  if (initializing_) {
    sync_prefs_->Clear();
    initializing_ = false;
  }
  NotifySyncSetupError(error);
#endif
}

void ProfileSyncService::OnGetInitData(const std::string& sync_version) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

#if 0
  Uint8Array seed;
  if (!sync_words_.empty()) {
    VLOG(1) << "[Brave Sync] Init from sync words";
  } else if (!sync_prefs_->GetSeed().empty()) {
    seed = Uint8ArrayFromString(sync_prefs_->GetSeed());
    VLOG(1) << "[Brave Sync] Init from prefs";
  } else {
    VLOG(1) << "[Brave Sync] Init new chain";
  }

  Uint8Array device_id;
  if (!sync_prefs_->GetThisDeviceId().empty()) {
    device_id = Uint8ArrayFromString(sync_prefs_->GetThisDeviceId());
    VLOG(1) << "[Brave Sync] Init device id from prefs: " <<
        StrFromUint8Array(device_id);
  } else {
    VLOG(1) << "[Brave Sync] Init empty device id";
  }

  DCHECK(!sync_version.empty());
  // TODO(bridiver) - this seems broken because using the version we get back
  // from the server (currently v1.4.2) causes things to break. What is the
  // the point of having this value?
  sync_prefs_->SetApiVersion("0");

  brave_sync::client_data::Config config;
  config.api_version = sync_prefs_->GetApiVersion();
  config.server_url = "https://sync.brave.com";
  config.debug = true;
  sync_client_->SendGotInitData(seed, device_id, config, sync_words_);
#endif
}

void ProfileSyncService::OnSaveInitData(const Uint8Array& seed,
                                          const Uint8Array& device_id) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
#if 0
  DCHECK(!sync_initialized_);
  // If we are here and initializing_ is false, we have came
  // not from OnSetupSyncNewToSync or OnSetupSyncHaveCode.
  // One case is we put wrong code words and then restarted before cleared
  // kSyncEnabled pref. This should not happen.
  DCHECK(initializing_);

  std::string seed_str = StrFromUint8Array(seed);
  std::string device_id_str = StrFromUint8Array(device_id);

  std::string prev_seed_str = sync_prefs_->GetPrevSeed();

  sync_words_.clear();
  DCHECK(!seed_str.empty());

  if (prev_seed_str == seed_str) {  // reconnecting to previous sync chain
    sync_prefs_->SetPrevSeed(std::string());
  } else if (!prev_seed_str.empty()) {  // connect/create to new sync chain
    bookmark_change_processor_->Reset(true);
    sync_prefs_->SetPrevSeed(std::string());
  } else {
    // This is not required, because when there is no previous seed, bookmarks
    // should not have a metadata. However, this is done by intention, to be
    // a remedy for cases when sync had been reset and prev_seed_str had been
    // cleared when it shouldn't (brave-browser#3188).
    bookmark_change_processor_->Reset(true);
  }

  sync_prefs_->SetSeed(seed_str);
  sync_prefs_->SetThisDeviceId(device_id_str);

  sync_configured_ = true;

  sync_prefs_->SetSyncBookmarksEnabled(true);
  // TODO(bridiver) - re-enable these when we add history, site settings
  sync_prefs_->SetSyncSiteSettingsEnabled(false);
  sync_prefs_->SetSyncHistoryEnabled(false);

  initializing_ = false;
#endif
}

void ProfileSyncService::OnSyncReady() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
#if 0
  const std::string bookmarks_base_order = sync_prefs_->GetBookmarksBaseOrder();
  if (bookmarks_base_order.empty()) {
    std::string platform = tools::GetPlatformName();
    sync_client_->SendGetBookmarksBaseOrder(sync_prefs_->GetThisDeviceId(),
                                            platform);
    // OnSyncReady will be called by OnSaveBookmarksBaseOrder
    return;
  }

  DCHECK(false == sync_initialized_);
  sync_initialized_ = true;

  // fetch the records
  RequestSyncData();
#endif
}

void ProfileSyncService::OnGetExistingObjects(
    const std::string& category_name,
    std::unique_ptr<RecordsList> records,
    const base::Time &last_record_time_stamp,
    const bool is_truncated) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
#if 0
  // TODO(bridiver) - what do we do with is_truncated ?
  // It appears to be ignored in b-l
  if (!tools::IsTimeEmpty(last_record_time_stamp)) {
    sync_prefs_->SetLatestRecordTime(last_record_time_stamp);
  }

  if (category_name == jslib_const::kBookmarks) {
    auto records_and_existing_objects =
        std::make_unique<SyncRecordAndExistingList>();
    bookmark_change_processor_->GetAllSyncData(
        *records.get(), records_and_existing_objects.get());
    sync_client_->SendResolveSyncRecords(
        category_name, std::move(records_and_existing_objects));
  } else if (category_name == brave_sync::jslib_const::kPreferences) {
    auto existing_records = PrepareResolvedPreferences(*records.get());
    sync_client_->SendResolveSyncRecords(
        category_name, std::move(existing_records));
  }
#endif
}

void ProfileSyncService::OnResolvedSyncRecords(
    const std::string& category_name,
    std::unique_ptr<RecordsList> records) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
#if 0
  if (category_name == brave_sync::jslib_const::kPreferences) {
    OnResolvedPreferences(*records.get());
  } else if (category_name == brave_sync::jslib_const::kBookmarks) {
    bookmark_change_processor_->ApplyChangesFromSyncModel(*records.get());
    bookmark_change_processor_->SendUnsynced(unsynced_send_interval_);
  } else if (category_name == brave_sync::jslib_const::kHistorySites) {
    NOTIMPLEMENTED();
  }
#endif
}

void ProfileSyncService::OnDeletedSyncUser() {
  NOTIMPLEMENTED();
}

void ProfileSyncService::OnDeleteSyncSiteSettings()  {
  NOTIMPLEMENTED();
}

void ProfileSyncService::OnSaveBookmarksBaseOrder(const std::string& order)  {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
#if 0
  DCHECK(!order.empty());
  sync_prefs_->SetBookmarksBaseOrder(order);
  OnSyncReady();
#endif
}

void ProfileSyncService::OnSyncWordsPrepared(const std::string& words) {
#if 0
  NotifyHaveSyncWords(words);
#endif
}

syncer::SyncClient* ProfileSyncService::GetSyncClient() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return sync_client_.get();
}

void ProfileSyncService::BraveSyncSetup() {
  brave_sync_prefs_ =
    std::make_unique<brave_sync::prefs::Prefs>(sync_client_->GetPrefService());
  sync_client_->GetBraveSyncClient()->set_sync_message_handler(this);
}

}   // namespace browser_sync
