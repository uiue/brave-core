// Copyright (c) 2019 The Brave Authors. All rights reserved.

#ifndef BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_TRACKING_PROTECTION_HELPER_H_
#define BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_TRACKING_PROTECTION_HELPER_H_

#include "base/macros.h"
#include "base/synchronization/lock.h"
#include "base/strings/string16.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

namespace content {
class WebContents;
class NavigationHandle;
}

namespace brave_shields {

class TrackingProtectionHelper : public content::WebContentsObserver,
  public content::WebContentsUserData<TrackingProtectionHelper> {
 public:
  explicit TrackingProtectionHelper(content::WebContents*);
  ~TrackingProtectionHelper() override;
  void DidStartNavigation(content::NavigationHandle*
    navigation_handle) override;
  void RenderFrameHostChanged(content::RenderFrameHost* old_host,
                              content::RenderFrameHost* new_host) override;
  void RenderFrameDeleted(content::RenderFrameHost* render_frame_host)
    override;
  static bool IsSmartTrackingProtectionEnabled();

  DISALLOW_COPY_AND_ASSIGN(TrackingProtectionHelper);
};

}  // namespace brave_shields

#endif  // BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_TRACKING_PROTECTION_HELPER_H_
