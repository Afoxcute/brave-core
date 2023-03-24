/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */
#include "brave/components/brave_rewards/core/endpoint/promotion/post_suggestions/post_suggestions.h"

#include <string>
#include <utility>
#include <vector>

#include "base/test/task_environment.h"
#include "brave/components/brave_rewards/core/ledger.h"
#include "brave/components/brave_rewards/core/ledger_client_mock.h"
#include "brave/components/brave_rewards/core/ledger_impl_mock.h"
#include "net/http/http_status_code.h"
#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=PostSuggestionsTest.*

using ::testing::_;
using ::testing::Invoke;

namespace ledger {
namespace endpoint {
namespace promotion {

class PostSuggestionsTest : public testing::Test {
 protected:
  base::test::TaskEnvironment task_environment_;
  MockLedgerImpl mock_ledger_impl_;
  PostSuggestions suggestions_{&mock_ledger_impl_};
};

TEST_F(PostSuggestionsTest, ServerOK) {
  ON_CALL(*mock_ledger_impl_.rewards_service(), LoadURL(_, _))
      .WillByDefault(
          Invoke([](mojom::UrlRequestPtr request, LoadURLCallback callback) {
            mojom::UrlResponse response;
            response.status_code = 200;
            response.url = request->url;
            response.body = "";
            std::move(callback).Run(response);
          }));

  mojom::UnblindedToken token;
  token.token_value =
      "s1OrSZUvo/33u3Y866mQaG/"
      "b6d94TqMThLal4+DSX4UrR4jT+GtTErim+"
      "FtEyZ7nebNGRoUDxObiUni9u8BB0DIT2aya6rYWko64IrXJWpbf0SVHnQFVYNyX64NjW9R6";  // NOLINT
  token.public_key = "dvpysTSiJdZUPihius7pvGOfngRWfDiIbrowykgMi1I=";

  credential::CredentialsRedeem redeem;
  redeem.publisher_key = "brave.com";
  redeem.type = mojom::RewardsType::ONE_TIME_TIP;
  redeem.processor = mojom::ContributionProcessor::BRAVE_TOKENS;
  redeem.token_list = {token};
  redeem.order_id = "c4645786-052f-402f-8593-56af2f7a21ce";
  redeem.contribution_id = "83b3b77b-e7c3-455b-adda-e476fa0656d2";

  suggestions_.Request(redeem, [](const mojom::Result result) {
    EXPECT_EQ(result, mojom::Result::LEDGER_OK);
  });
}

TEST_F(PostSuggestionsTest, ServerError400) {
  ON_CALL(*mock_ledger_impl_.rewards_service(), LoadURL(_, _))
      .WillByDefault(
          Invoke([](mojom::UrlRequestPtr request, LoadURLCallback callback) {
            mojom::UrlResponse response;
            response.status_code = 400;
            response.url = request->url;
            response.body = "";
            std::move(callback).Run(response);
          }));

  mojom::UnblindedToken token;
  token.token_value =
      "s1OrSZUvo/33u3Y866mQaG/"
      "b6d94TqMThLal4+DSX4UrR4jT+GtTErim+"
      "FtEyZ7nebNGRoUDxObiUni9u8BB0DIT2aya6rYWko64IrXJWpbf0SVHnQFVYNyX64NjW9R6";  // NOLINT
  token.public_key = "dvpysTSiJdZUPihius7pvGOfngRWfDiIbrowykgMi1I=";

  credential::CredentialsRedeem redeem;
  redeem.publisher_key = "brave.com";
  redeem.type = mojom::RewardsType::ONE_TIME_TIP;
  redeem.processor = mojom::ContributionProcessor::BRAVE_TOKENS;
  redeem.token_list = {token};
  redeem.order_id = "c4645786-052f-402f-8593-56af2f7a21ce";
  redeem.contribution_id = "83b3b77b-e7c3-455b-adda-e476fa0656d2";

  suggestions_.Request(redeem, [](const mojom::Result result) {
    EXPECT_EQ(result, mojom::Result::LEDGER_ERROR);
  });
}

TEST_F(PostSuggestionsTest, ServerError500) {
  ON_CALL(*mock_ledger_impl_.rewards_service(), LoadURL(_, _))
      .WillByDefault(
          Invoke([](mojom::UrlRequestPtr request, LoadURLCallback callback) {
            mojom::UrlResponse response;
            response.status_code = 500;
            response.url = request->url;
            response.body = "";
            std::move(callback).Run(response);
          }));

  mojom::UnblindedToken token;
  token.token_value =
      "s1OrSZUvo/33u3Y866mQaG/"
      "b6d94TqMThLal4+DSX4UrR4jT+GtTErim+"
      "FtEyZ7nebNGRoUDxObiUni9u8BB0DIT2aya6rYWko64IrXJWpbf0SVHnQFVYNyX64NjW9R6";  // NOLINT
  token.public_key = "dvpysTSiJdZUPihius7pvGOfngRWfDiIbrowykgMi1I=";

  credential::CredentialsRedeem redeem;
  redeem.publisher_key = "brave.com";
  redeem.type = mojom::RewardsType::ONE_TIME_TIP;
  redeem.processor = mojom::ContributionProcessor::BRAVE_TOKENS;
  redeem.token_list = {token};
  redeem.order_id = "c4645786-052f-402f-8593-56af2f7a21ce";
  redeem.contribution_id = "83b3b77b-e7c3-455b-adda-e476fa0656d2";

  suggestions_.Request(redeem, [](const mojom::Result result) {
    EXPECT_EQ(result, mojom::Result::LEDGER_ERROR);
  });
}

}  // namespace promotion
}  // namespace endpoint
}  // namespace ledger
