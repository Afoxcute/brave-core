/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_BROWSER_ADS_SERVICE_DELEGATE_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_BROWSER_ADS_SERVICE_DELEGATE_H_

#include "brave/components/services/bat_ads/public/interfaces/bat_ads.mojom.h"
#include "mojo/public/cpp/bindings/remote.h"

class GURL;

namespace brave_ads {

class AdsServiceDelegate {
 public:
  virtual ~AdsServiceDelegate() = default;

  virtual mojo::Remote<bat_ads::mojom::BatAdsService> LaunchBatAdsService()
      const = 0;

  virtual void OpenNewTabWithUrl(const GURL& url) = 0;
};

}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_BROWSER_ADS_SERVICE_DELEGATE_H_
