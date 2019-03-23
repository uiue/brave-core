// Copyright (c) 2019 The Brave Authors. All rights reserved.

#include "brave/browser/brave_tab_helpers.h"

#include "brave/browser/brave_drm_tab_helper.h"
#include "brave/browser/renderer_host/buildflags/buildflags.h"  // For STP
#include "brave/components/brave_ads/browser/ads_tab_helper.h"
#include "brave/components/brave_rewards/browser/buildflags/buildflags.h"
#include "content/public/browser/web_contents.h"

#if !defined(OS_ANDROID)
#include "brave/components/brave_shields/browser/brave_shields_web_contents_observer.h"
#if BUILDFLAG(BRAVE_REWARDS_ENABLED)
#include "brave/components/brave_rewards/browser/rewards_helper.h"
#endif
#if BUILDFLAG(BRAVE_STP_ENABLED)
#include "brave/components/brave_shields/browser/tracking_protection_helper.h"
#endif
// Add tab helpers here unless they are intended for android too
#endif

namespace brave {

void AttachTabHelpers(content::WebContents* web_contents) {
#if !defined(OS_ANDROID)
  brave_shields::BraveShieldsWebContentsObserver::CreateForWebContents(
      web_contents);
#if BUILDFLAG(BRAVE_REWARDS_ENABLED)
  brave_rewards::RewardsHelper::CreateForWebContents(web_contents);
#endif
  // Add tab helpers here unless they are intended for android too
  BraveDrmTabHelper::CreateForWebContents(web_contents);
#if BUILDFLAG(BRAVE_STP_ENABLED)
  brave_shields::TrackingProtectionHelper::CreateForWebContents(
      web_contents);
#endif
#endif

  brave_ads::AdsTabHelper::CreateForWebContents(web_contents);
}

}  // namespace brave
