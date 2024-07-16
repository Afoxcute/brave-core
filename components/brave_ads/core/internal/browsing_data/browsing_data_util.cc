/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/browsing_data/browsing_data_util.h"

#include <utility>

#include "brave/components/brave_ads/core/internal/account/confirmations/queue/confirmation_queue_database_table.h"
#include "brave/components/brave_ads/core/internal/common/database/database_transaction_util.h"
#include "brave/components/brave_ads/core/internal/common/logging_util.h"
#include "brave/components/brave_ads/core/internal/common/time/time_formatting_util.h"
#include "brave/components/brave_ads/core/internal/creatives/conversions/creative_set_conversion_database_table.h"
#include "brave/components/brave_ads/core/internal/user_engagement/ad_events/ad_event_cache_util.h"
#include "brave/components/brave_ads/core/internal/user_engagement/ad_events/ad_events_database_table.h"
#include "brave/components/brave_ads/core/mojom/brave_ads.mojom.h"

namespace brave_ads {

void DeleteNonRewardsBrowsingDataBetween(const base::Time from_time,
                                         const base::Time to_time,
                                         ResultCallback callback) {
  BLOG(1,
       "Start deleting non-rewards browsing data between "
           << LongFriendlyDateAndTime(from_time, /*use_sentence_style=*/false)
           << " and "
           << LongFriendlyDateAndTime(to_time, /*use_sentence_style=*/false));

  mojom::DBTransactionInfoPtr transaction = mojom::DBTransactionInfo::New();

  database::table::ConfirmationQueue confirmation_queue_database_table;
  confirmation_queue_database_table.PurgeAllBetween(transaction.get(),
                                                    from_time, to_time);

  database::table::CreativeSetConversions
      creative_set_conversions_database_table;
  creative_set_conversions_database_table.PurgeAllBetween(transaction.get(),
                                                          from_time, to_time);

  const database::table::AdEvents ad_events_database_table;
  ad_events_database_table.PurgeAllBetween(transaction.get(), from_time,
                                           to_time);

  database::RunTransaction(std::move(transaction),
                           base::BindOnce(
                               [](ResultCallback callback, const bool success) {
                                 if (success) {
                                   BLOG(1,
                                        "Successfully deleted non-rewards "
                                        "browsing data between the given "
                                        "time range.");

                                   RebuildAdEventCache();
                                 } else {
                                   BLOG(0,
                                        "Failed to delete non-rewards browsing "
                                        "data between the given time "
                                        "range.");
                                 }
                                 std::move(callback).Run(success);
                               },
                               std::move(callback)));
}

}  // namespace brave_ads
