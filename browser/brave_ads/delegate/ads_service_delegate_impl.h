/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_BRAVE_ADS_DELEGATE_ADS_SERVICE_DELEGATE_IMPL_H_
#define BRAVE_BROWSER_BRAVE_ADS_DELEGATE_ADS_SERVICE_DELEGATE_IMPL_H_

#include "brave/components/brave_ads/browser/ads_service_delegate.h"

class Profile;

namespace brave_ads {

class AdsServiceDelegateImpl : public AdsServiceDelegate {
 public:
  explicit AdsServiceDelegateImpl(Profile* profile);
  ~AdsServiceDelegateImpl() override;

  AdsServiceDelegateImpl(const AdsServiceDelegateImpl&) = delete;
  AdsServiceDelegateImpl& operator=(const AdsServiceDelegateImpl&) = delete;
  AdsServiceDelegateImpl(AdsServiceDelegateImpl&&) noexcept = delete;
  AdsServiceDelegateImpl& operator=(AdsServiceDelegateImpl&&) noexcept = delete;

  // AdsServiceDelegate:
  mojo::Remote<bat_ads::mojom::BatAdsService> LaunchBatAdsService()
      const override;
  void OpenNewTabWithUrl(const GURL& url) override;

 private:
  const raw_ptr<Profile> profile_ = nullptr;  // NOT OWNED
};

}  // namespace brave_ads

#endif  // BRAVE_BROWSER_BRAVE_ADS_DELEGATE_ADS_SERVICE_DELEGATE_IMPL_H_
