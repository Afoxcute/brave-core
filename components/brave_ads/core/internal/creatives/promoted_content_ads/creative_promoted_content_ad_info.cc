/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/creatives/promoted_content_ads/creative_promoted_content_ad_info.h"

#include <tuple>

namespace brave_ads {

CreativePromotedContentAdInfo::CreativePromotedContentAdInfo() = default;

CreativePromotedContentAdInfo::CreativePromotedContentAdInfo(
    const CreativeAdInfo& creative_ad)
    : CreativeAdInfo(creative_ad) {}

bool CreativePromotedContentAdInfo::operator==(
    const CreativePromotedContentAdInfo& other) const {
  const auto tie = [](const CreativePromotedContentAdInfo& ad) {
    return std::tie(ad.title, ad.description);
  };

  return CreativeAdInfo::operator==(other) && tie(*this) == tie(other);
}

bool CreativePromotedContentAdInfo::operator!=(
    const CreativePromotedContentAdInfo& other) const {
  return !(*this == other);
}

}  // namespace brave_ads
