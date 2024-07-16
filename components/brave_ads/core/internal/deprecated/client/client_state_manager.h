/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_DEPRECATED_CLIENT_CLIENT_STATE_MANAGER_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_DEPRECATED_CLIENT_CLIENT_STATE_MANAGER_H_

#include <optional>
#include <string>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "brave/components/brave_ads/core/internal/deprecated/client/client_info.h"
#include "brave/components/brave_ads/core/internal/deprecated/client/preferences/filtered_advertiser_info.h"
#include "brave/components/brave_ads/core/internal/deprecated/client/preferences/filtered_category_info.h"
#include "brave/components/brave_ads/core/internal/deprecated/client/preferences/flagged_ad_info.h"
#include "brave/components/brave_ads/core/internal/targeting/behavioral/purchase_intent/resource/purchase_intent_signal_history_info.h"
#include "brave/components/brave_ads/core/internal/targeting/contextual/text_classification/model/text_classification_alias.h"
#include "brave/components/brave_ads/core/mojom/brave_ads.mojom-shared.h"
#include "brave/components/brave_ads/core/public/ads_callback.h"
#include "brave/components/brave_ads/core/public/client/ads_client_callback.h"
#include "brave/components/brave_ads/core/public/history/history_item_info.h"

namespace brave_ads {

struct AdContentInfo;
struct AdInfo;

class ClientStateManager final {
 public:
  ClientStateManager();

  ClientStateManager(const ClientStateManager&) = delete;
  ClientStateManager& operator=(const ClientStateManager&) = delete;

  ClientStateManager(ClientStateManager&&) noexcept = delete;
  ClientStateManager& operator=(ClientStateManager&&) noexcept = delete;

  ~ClientStateManager();

  static ClientStateManager& GetInstance();

  void LoadState(InitializeCallback callback);

  const FilteredAdvertiserList& GetFilteredAdvertisers() const;
  const FilteredCategoryList& GetFilteredCategories() const;
  const FlaggedAdList& GetFlaggedAds() const;

  void AppendHistory(const HistoryItemInfo& history_item);
  const HistoryItemList& GetHistory() const;

  void AppendToPurchaseIntentSignalHistoryForSegment(
      const std::string& segment,
      const PurchaseIntentSignalHistoryInfo& history);
  const PurchaseIntentSignalHistoryMap& GetPurchaseIntentSignalHistory() const;

  mojom::UserReactionType ToggleLikeAd(const AdContentInfo& ad_content);
  mojom::UserReactionType ToggleDislikeAd(const AdContentInfo& ad_content);
  mojom::UserReactionType GetUserReactionTypeForAdvertiser(
      const std::string& advertiser_id);

  mojom::UserReactionType ToggleLikeCategory(
      const CategoryContentInfo& category_content);
  mojom::UserReactionType ToggleDislikeCategory(
      const CategoryContentInfo& category_content);
  mojom::UserReactionType GetUserReactionTypeForSegment(
      const std::string& segment);

  bool ToggleSaveAd(const AdContentInfo& ad_content);

  bool ToggleMarkAdAsInappropriate(const AdContentInfo& ad_content);

  void AppendTextClassificationProbabilitiesToHistory(
      const TextClassificationProbabilityMap& probabilities);
  const TextClassificationProbabilityList&
  GetTextClassificationProbabilitiesHistory() const;

  void DeleteRewardsBrowsingDataBetween(base::Time from_time,
                                        base::Time to_time,
                                        ResultCallback callback);

 private:
  std::vector<HistoryItemInfo> DeleteHistoryItemsBetween(base::Time from_time,
                                                         base::Time to_time);
  void UpdateFlaggedAds(
      const std::vector<HistoryItemInfo>& deleted_history_items);
  void UpdateSavedAds(
      const std::vector<HistoryItemInfo>& deleted_history_items);
  void UpdateFilteredAdvertisers();
  void UpdateFilteredCategories();

  void DeletePurchaseIntentSignalHistoryBetween(base::Time from_time,
                                                base::Time to_time);
  void DeleteAllTextClassificationProbabilities();

  void SaveState();

  void LoadCallback(InitializeCallback callback,
                    const std::optional<std::string>& json);

  [[nodiscard]] bool FromJson(const std::string& json);

  ClientInfo client_;

  bool is_initialized_ = false;

  base::WeakPtrFactory<ClientStateManager> weak_factory_{this};
};

}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_DEPRECATED_CLIENT_CLIENT_STATE_MANAGER_H_
