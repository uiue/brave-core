// Copyright (c) 2019 The Brave Authors. All rights reserved.

#include "brave/browser/renderer_host/brave_render_message_filter.h"

#include "brave/browser/brave_browser_process_impl.h"
#include "brave/components/brave_shields/browser/tracking_protection_service.h"
#include "chrome/browser/content_settings/host_content_settings_map_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/render_messages.h"

using base::string16;

BraveRenderMessageFilter::BraveRenderMessageFilter(int render_process_id,
  Profile* profile)
  : ChromeRenderMessageFilter(render_process_id, profile),
    host_content_settings_map_(HostContentSettingsMapFactory::GetForProfile(
      profile)) {
}

BraveRenderMessageFilter::~BraveRenderMessageFilter() {}

bool BraveRenderMessageFilter::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(BraveRenderMessageFilter, message)
    IPC_MESSAGE_HANDLER(ChromeViewHostMsg_AllowDatabase, OnAllowDatabase);
    IPC_MESSAGE_HANDLER(ChromeViewHostMsg_AllowDOMStorage, OnAllowDOMStorage);
    IPC_MESSAGE_HANDLER(ChromeViewHostMsg_AllowIndexedDB, OnAllowIndexedDB);
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return ChromeRenderMessageFilter::OnMessageReceived(message);
}

void BraveRenderMessageFilter::OnAllowDatabase(int render_frame_id,
                                               const GURL& origin_url,
                                               const GURL& top_origin_url,
                                               const string16& name,
                                               const string16& display_name,
                                               bool* allowed) {
  *allowed = g_brave_browser_process->tracking_protection_service()->
    ShouldStoreState(host_content_settings_map_,
                     render_process_id_,
                     render_frame_id,
                     origin_url,
                     top_origin_url);

  if (*allowed) {
    ChromeRenderMessageFilter::OnAllowDatabase(render_frame_id,
                                               origin_url,
                                               top_origin_url,
                                               name,
                                               display_name,
                                               allowed);
  }
}

void BraveRenderMessageFilter::OnAllowDOMStorage(int render_frame_id,
                                                 const GURL& origin_url,
                                                 const GURL& top_origin_url,
                                                 bool local,
                                                 bool* allowed) {
  *allowed = g_brave_browser_process->tracking_protection_service()->
    ShouldStoreState(host_content_settings_map_,
                     render_process_id_,
                     render_frame_id,
                     origin_url,
                     top_origin_url);

  if (*allowed) {
    ChromeRenderMessageFilter::OnAllowDOMStorage(render_frame_id,
                                                 origin_url,
                                                 top_origin_url,
                                                 local,
                                                 allowed);
  }
}

void BraveRenderMessageFilter::OnAllowIndexedDB(int render_frame_id,
                                                const GURL& origin_url,
                                                const GURL& top_origin_url,
                                                bool* allowed) {
  *allowed = g_brave_browser_process->tracking_protection_service()->
    ShouldStoreState(host_content_settings_map_,
                     render_process_id_,
                     render_frame_id,
                     origin_url,
                     top_origin_url);

  if (*allowed) {
    ChromeRenderMessageFilter::OnAllowIndexedDB(render_frame_id,
                                                origin_url,
                                                top_origin_url,
                                                allowed);
  }
}
