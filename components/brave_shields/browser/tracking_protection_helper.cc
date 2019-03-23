// Copyright (c) 2019 The Brave Authors. All rights reserved.

#include "brave/components/brave_shields/browser/tracking_protection_helper.h"

#include "brave/browser/brave_browser_process_impl.h"
#include "brave/components/brave_shields/browser/tracking_protection_service.h"
#include "chrome/browser/ui/browser_list.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_user_data.h"

using content::NavigationHandle;
using content::RenderFrameHost;
using content::WebContents;

namespace brave_shields {

TrackingProtectionHelper::TrackingProtectionHelper(WebContents*
  web_contents)
    : WebContentsObserver(web_contents) {
}

TrackingProtectionHelper::~TrackingProtectionHelper() {
}

void TrackingProtectionHelper::DidStartNavigation(NavigationHandle* handle) {
  RenderFrameHost* rfh = web_contents()->GetMainFrame();

  if (!ui::PageTransitionIsRedirect(handle->GetPageTransition())) {
    g_brave_browser_process->tracking_protection_service()->
      SetStartingSiteForRenderFrame(handle->GetURL(),
        rfh->GetProcess()->GetID(), rfh->GetRoutingID());
  }
  return;
}

void TrackingProtectionHelper::RenderFrameDeleted(RenderFrameHost*
  render_frame_host) {
  g_brave_browser_process->tracking_protection_service()->
    DeleteRenderFrameKey(render_frame_host->GetProcess()->GetID(),
      render_frame_host->GetRoutingID());
}

void TrackingProtectionHelper::RenderFrameHostChanged(
    RenderFrameHost* old_host, RenderFrameHost* new_host) {
  if (!old_host || old_host->GetParent() ||
      new_host->GetParent()) {
    return;
  }

  g_brave_browser_process->tracking_protection_service()->
    ModifyRenderFrameKey(old_host->GetProcess()->GetID(),
    old_host->GetRoutingID(), new_host->GetProcess()->GetID(),
    new_host->GetRoutingID());
}

}  // namespace brave_shields
