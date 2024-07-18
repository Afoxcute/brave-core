/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/browser/brave_ads/delegate/ads_service_delegate_impl.h"

#include <memory>
#include <utility>

#include "base/check.h"
#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/task/single_thread_task_runner.h"
#include "base/task/single_thread_task_runner_thread_mode.h"
#include "base/task/thread_pool.h"
#include "brave/browser/brave_ads/delegate/service_sandbox_type.h"
#include "brave/components/brave_ads/core/public/ads_feature.h"
#include "brave/components/services/bat_ads/bat_ads_service_impl.h"
#include "brave/grit/brave_generated_resources.h"
#include "chrome/browser/browser_process.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/service_process_host.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/self_owned_receiver.h"
#include "url/gurl.h"

#if BUILDFLAG(IS_ANDROID)
#include "chrome/browser/android/service_tab_launcher.h"
#include "content/public/browser/page_navigator.h"
#else
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_navigator.h"
#include "chrome/browser/ui/browser_navigator_params.h"
#endif  // BUILDFLAG(IS_ANDROID)

namespace brave_ads {

namespace {

// Binds the `receiver` to a new provider on a background task runner.
void BindInProcessBatAdsService(
    mojo::PendingReceiver<bat_ads::mojom::BatAdsService> receiver) {
  mojo::MakeSelfOwnedReceiver(std::make_unique<bat_ads::BatAdsServiceImpl>(),
                              std::move(receiver));
}

// Launches an in process Bat Ads Service.
mojo::Remote<bat_ads::mojom::BatAdsService> LaunchInProcessBatAdsService() {
  mojo::Remote<bat_ads::mojom::BatAdsService> remote;
  base::ThreadPool::CreateSingleThreadTaskRunner(
      {base::MayBlock(), base::WithBaseSyncPrimitives()},
      base::SingleThreadTaskRunnerThreadMode::DEDICATED)
      ->PostTask(FROM_HERE,
                 base::BindOnce(&BindInProcessBatAdsService,
                                remote.BindNewPipeAndPassReceiver()));
  return remote;
}

// Launches a new Bat Ads Service utility process.
mojo::Remote<bat_ads::mojom::BatAdsService> LaunchOutOfProcessBatAdsService() {
  return content::ServiceProcessHost::Launch<bat_ads::mojom::BatAdsService>(
      content::ServiceProcessHost::Options()
          .WithDisplayName(IDS_SERVICE_BAT_ADS)
          .Pass());
}

}  // namespace

AdsServiceDelegateImpl::AdsServiceDelegateImpl(Profile* profile)
    : profile_(profile) {
  CHECK(profile_);
}

AdsServiceDelegateImpl::~AdsServiceDelegateImpl() = default;

mojo::Remote<bat_ads::mojom::BatAdsService>
AdsServiceDelegateImpl::LaunchBatAdsService() const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  return ShouldLaunchAsInProcessService() ? LaunchInProcessBatAdsService()
                                          : LaunchOutOfProcessBatAdsService();
}

void AdsServiceDelegateImpl::OpenNewTabWithUrl(const GURL& url) {
  if (g_browser_process->IsShuttingDown()) {
    return;
  }

  if (!url.is_valid()) {
    return VLOG(1) << "Failed to open new tab due to invalid URL: " << url;
  }

#if BUILDFLAG(IS_ANDROID)
  // ServiceTabLauncher can currently only launch new tabs
  const content::OpenURLParams params(url, content::Referrer(),
                                      WindowOpenDisposition::NEW_FOREGROUND_TAB,
                                      ui::PAGE_TRANSITION_LINK, true);
  ServiceTabLauncher::GetInstance()->LaunchTab(
      profile_, params, base::BindOnce([](content::WebContents*) {}));
#else
  Browser* browser = chrome::FindTabbedBrowser(profile_, false);
  if (!browser) {
    browser = Browser::Create(Browser::CreateParams(profile_, true));
  }

  NavigateParams nav_params(browser, url, ui::PAGE_TRANSITION_LINK);
  nav_params.disposition = WindowOpenDisposition::SINGLETON_TAB;
  nav_params.window_action = NavigateParams::SHOW_WINDOW;
  nav_params.path_behavior = NavigateParams::RESPECT;
  Navigate(&nav_params);
#endif
}

}  // namespace brave_ads
