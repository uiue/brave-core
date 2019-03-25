/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_shields/browser/tracking_protection_service.h"

#include <algorithm>
#include <utility>

#include "base/base_paths.h"
#include "base/bind.h"
#include "brave/browser/renderer_host/buildflags/buildflags.h"  // For STP
#include "base/command_line.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "brave/browser/brave_browser_process_impl.h"
#include "brave/components/brave_shields/browser/ad_block_service.h"
#include "brave/components/brave_shields/browser/local_data_files_service.h"
#include "brave/components/brave_shields/browser/dat_file_util.h"
#include "brave/vendor/tracking-protection/TPParser.h"

#if BUILDFLAG(BRAVE_STP_ENABLED)
#include "base/strings/string_split.h"
#include "base/task/post_task.h"
#include "brave/common/brave_switches.h"
#include "brave/components/brave_shields/browser/brave_shields_util.h"
#include "brave/components/brave_shields/browser/brave_shields_web_contents_observer.h"
#include "brave/components/brave_shields/common/brave_shield_constants.h"
#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"

#define STORAGE_TRACKERS_FILE "StorageTrackingProtection.dat"
#endif

#define NAVIGATION_TRACKERS_FILE "TrackingProtection.dat"
#define DAT_FILE_VERSION "1"
#define THIRD_PARTY_HOSTS_CACHE_SIZE 20

#if BUILDFLAG(BRAVE_STP_ENABLED)
using content::BrowserThread;
using content::RenderFrameHost;
#endif

namespace brave_shields {

TrackingProtectionService::TrackingProtectionService()
  : tracking_protection_client_(new CTPParser()),
    weak_factory_(this) {
  DETACH_FROM_SEQUENCE(sequence_checker_);
}

TrackingProtectionService::~TrackingProtectionService() {
  tracking_protection_client_.reset();
}

#if BUILDFLAG(BRAVE_STP_ENABLED)
TrackingProtectionService::RenderFrameIdKey::RenderFrameIdKey()
    : render_process_id(content::ChildProcessHost::kInvalidUniqueID),
      frame_routing_id(MSG_ROUTING_NONE) {}

TrackingProtectionService::RenderFrameIdKey::RenderFrameIdKey(
    int render_process_id,
    int frame_routing_id)
    : render_process_id(render_process_id),
      frame_routing_id(frame_routing_id) {}

bool TrackingProtectionService::RenderFrameIdKey::operator<(
    const RenderFrameIdKey& other) const {
  return std::tie(render_process_id, frame_routing_id) <
         std::tie(other.render_process_id, other.frame_routing_id);
}

bool TrackingProtectionService::RenderFrameIdKey::operator==(
    const RenderFrameIdKey& other) const {
  return render_process_id == other.render_process_id &&
         frame_routing_id == other.frame_routing_id;
}

void TrackingProtectionService::SetStartingSiteForRenderFrame(
  GURL starting_site, int render_process_id, int render_frame_id) {
  base::AutoLock lock(frame_starting_site_map_lock_);
  const RenderFrameIdKey key(render_process_id, render_frame_id);
  auto iter = render_frame_key_to_starting_site_url.find(key);
  if (iter != render_frame_key_to_starting_site_url.end()) {
    render_frame_key_to_starting_site_url.erase(key);
  }
  render_frame_key_to_starting_site_url.insert(std::pair<RenderFrameIdKey,
    GURL>(key, starting_site));
  return;
}

GURL TrackingProtectionService::GetStartingSiteForRenderFrame(
  int render_process_id, int render_frame_id) {
  base::AutoLock lock(frame_starting_site_map_lock_);
  const RenderFrameIdKey key(render_process_id, render_frame_id);
  auto iter = render_frame_key_to_starting_site_url.find(key);
  if (iter != render_frame_key_to_starting_site_url.end()) {
    return iter->second;
  }
  return GURL();
}

void TrackingProtectionService::ModifyRenderFrameKey(
  int old_render_process_id, int old_render_frame_id,
  int new_render_process_id, int new_render_frame_id) {
  base::AutoLock lock(frame_starting_site_map_lock_);
  const RenderFrameIdKey old_key(old_render_process_id, old_render_frame_id);
  auto iter =
      render_frame_key_to_starting_site_url.find(old_key);
  if (iter != render_frame_key_to_starting_site_url.end()) {
    const RenderFrameIdKey new_key(new_render_process_id, new_render_frame_id);
    render_frame_key_to_starting_site_url.insert(
        std::pair<RenderFrameIdKey, GURL>(new_key, iter->second));
    render_frame_key_to_starting_site_url.erase(old_key);
  }
}

void TrackingProtectionService::DeleteRenderFrameKey(int render_process_id,
  int render_frame_id) {
  base::AutoLock lock(frame_starting_site_map_lock_);
  const RenderFrameIdKey key(render_process_id, render_frame_id);
  render_frame_key_to_starting_site_url.erase(key);
}

bool TrackingProtectionService::ShouldStartRequest(const GURL& url,
    content::ResourceType resource_type,
    const std::string &tab_host,
    bool* matching_exception_filter,
    bool* cancel_request_explicitly) {
  // There are no exceptions in the TP service, but exceptions are
  // combined with brave/ad-block.
  if (matching_exception_filter) {
    *matching_exception_filter = false;
  }
  // Intentionally don't set cancel_request_explicitly
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  std::string host = url.host();
  if (!tracking_protection_client_->matchesTracker(
        tab_host.c_str(), host.c_str())) {
    return true;
  }

  std::vector<std::string> hosts(GetThirdPartyHosts(tab_host));
  for (size_t i = 0; i < hosts.size(); i++) {
    if (host == hosts[i] ||
        host.find((std::string)"." + hosts[i]) != std::string::npos) {
      return true;
    }
    size_t iPos = host.find((std::string)"." + hosts[i]);
    if (iPos == std::string::npos) {
      continue;
    }
    if (hosts[i].length() + ((std::string)".").length() + iPos ==
        host.length()) {
      return true;
    }
  }
  return false;
}

bool TrackingProtectionService::ShouldStoreState(HostContentSettingsMap* map, 
  int render_process_id, int render_frame_id, const GURL& top_origin_url, 
  const GURL& origin_url) {
  const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();
  if (!command_line.HasSwitch(switches::kEnableSmartTrackingProtection)) {
    return true;
  }

  if (first_party_storage_trackers_.empty()) {
    LOG(INFO) << "First party storage trackers list is empty";
    return true;
  }

  std::string host = origin_url.host();

  const GURL starting_site = GetStartingSiteForRenderFrame(render_process_id,
    render_frame_id);

  // If starting host is the current host, user-interaction has happened
  // so we allow storage
  if (starting_site.host() == host) {
    return true;
  }

  bool allow_brave_shields = starting_site == GURL() ? false :
    IsAllowContentSetting(map, starting_site, GURL(),
      CONTENT_SETTINGS_TYPE_PLUGINS, brave_shields::kBraveShields);

  if (!allow_brave_shields) {
    return true;
  }

  bool allow_trackers = starting_site == GURL() ? true : IsAllowContentSetting(
      map, starting_site, GURL(), CONTENT_SETTINGS_TYPE_PLUGINS,
      brave_shields::kTrackers);

  if (allow_trackers) {
    return true;
  }

  // deny storage if host is found in the tracker list
  return !(std::find(first_party_storage_trackers_.begin(),
    first_party_storage_trackers_.end(), host)
    != first_party_storage_trackers_.end());
}

void TrackingProtectionService::ParseStorageTrackersData() {
  if (storage_trackers_buffer_.empty()) {
    LOG(ERROR) << "Could not obtain tracking protection data";
    return;
  }

  std::string trackers(storage_trackers_buffer_.begin(),
    storage_trackers_buffer_.end());
  std::vector<std::string> storage_trackers = base::SplitString(
    base::StringPiece(trackers.data(), trackers.size()), ",",
    base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);

  if (storage_trackers.empty()) {
    LOG(ERROR) << "No first party trackers found";
    return;
  }
  first_party_storage_trackers_ = base::flat_set<std::string>(
    std::move(storage_trackers));
}
#endif

void TrackingProtectionService::OnDATFileDataReady() {
  if (buffer_.empty()) {
    LOG(ERROR) << "Could not obtain tracking protection data";
    return;
  }
  tracking_protection_client_.reset(new CTPParser());
  if (!tracking_protection_client_->deserialize(
        reinterpret_cast<char*>(&buffer_.front()))) {
    tracking_protection_client_.reset();
    LOG(ERROR) << "Failed to deserialize tracking protection data";
    return;
  }
}

void TrackingProtectionService::OnComponentReady(
    const std::string& component_id,
    const base::FilePath& install_dir,
    const std::string& manifest) {
  base::FilePath navigation_tracking_protection_path =
      install_dir.AppendASCII(DAT_FILE_VERSION).AppendASCII(
        NAVIGATION_TRACKERS_FILE);

  GetTaskRunner()->PostTaskAndReply(
      FROM_HERE,
      base::Bind(&GetDATFileData, navigation_tracking_protection_path,
        &buffer_),
      base::Bind(&TrackingProtectionService::OnDATFileDataReady,
                 weak_factory_.GetWeakPtr()));

#if BUILDFLAG(BRAVE_STP_ENABLED)
  base::FilePath storage_tracking_protection_path =
      install_dir.AppendASCII(DAT_FILE_VERSION).AppendASCII(
        STORAGE_TRACKERS_FILE);

  GetTaskRunner()->PostTaskAndReply(
      FROM_HERE,
      base::Bind(&GetDATFileData, storage_tracking_protection_path,
        &storage_trackers_buffer_),
        base::Bind(&TrackingProtectionService::ParseStorageTrackersData,
          weak_factory_.GetWeakPtr()));
#endif
}

// Ported from Android: net/blockers/blockers_worker.cc
std::vector<std::string>
TrackingProtectionService::GetThirdPartyHosts(const std::string& base_host) {
  {
    std::lock_guard<std::mutex> guard(third_party_hosts_mutex_);
    std::map<std::string, std::vector<std::string>>::const_iterator iter =
      third_party_hosts_cache_.find(base_host);
    if (third_party_hosts_cache_.end() != iter) {
      if (third_party_base_hosts_.size() != 0
          && third_party_base_hosts_[third_party_hosts_cache_.size() - 1] !=
          base_host) {
        for (size_t i = 0; i < third_party_base_hosts_.size(); i++) {
          if (third_party_base_hosts_[i] == base_host) {
            third_party_base_hosts_.erase(third_party_base_hosts_.begin() + i);
            third_party_base_hosts_.push_back(base_host);
            break;
          }
        }
      }
      return iter->second;
    }
  }

  char* thirdPartyHosts =
    tracking_protection_client_->findFirstPartyHosts(base_host.c_str());
  std::vector<std::string> hosts;
  if (nullptr != thirdPartyHosts) {
    std::string strThirdPartyHosts = thirdPartyHosts;
    size_t iPos = strThirdPartyHosts.find(",");
    while (iPos != std::string::npos) {
      std::string thirdParty = strThirdPartyHosts.substr(0, iPos);
      strThirdPartyHosts = strThirdPartyHosts.substr(iPos + 1);
      iPos = strThirdPartyHosts.find(",");
      hosts.push_back(thirdParty);
    }
    if (0 != strThirdPartyHosts.length()) {
      hosts.push_back(strThirdPartyHosts);
    }
    delete []thirdPartyHosts;
  }

  {
    std::lock_guard<std::mutex> guard(third_party_hosts_mutex_);
    if (third_party_hosts_cache_.size() == THIRD_PARTY_HOSTS_CACHE_SIZE &&
        third_party_base_hosts_.size() == THIRD_PARTY_HOSTS_CACHE_SIZE) {
      third_party_hosts_cache_.erase(third_party_base_hosts_[0]);
      third_party_base_hosts_.erase(third_party_base_hosts_.begin());
    }
    third_party_base_hosts_.push_back(base_host);
    third_party_hosts_cache_.insert(
        std::pair<std::string, std::vector<std::string>>(base_host, hosts));
  }

  return hosts;
}

scoped_refptr<base::SequencedTaskRunner>
TrackingProtectionService::GetTaskRunner() {
  // We share the same task runner for all ad-block and TP code
  return g_brave_browser_process->ad_block_service()->GetTaskRunner();
}

///////////////////////////////////////////////////////////////////////////////

// The tracking protection factory. Using the Brave Shields as a singleton
// is the job of the browser process.
std::unique_ptr<TrackingProtectionService> TrackingProtectionServiceFactory() {
  std::unique_ptr<TrackingProtectionService> service =
    std::make_unique<TrackingProtectionService>();
  g_brave_browser_process->local_data_files_service()->AddObserver(
      service.get());
  return service;
}

}  // namespace brave_shields
