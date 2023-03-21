/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <string>
#include <utility>
#include <vector>

#include "base/test/task_environment.h"
#include "brave/components/brave_rewards/core/endpoint/bitflyer/get_balance/get_balance_bitflyer.h"
#include "brave/components/brave_rewards/core/ledger_callbacks.h"
#include "brave/components/brave_rewards/core/ledger_client_mock.h"
#include "brave/components/brave_rewards/core/ledger_impl_mock.h"
#include "net/http/http_status_code.h"
#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=GetBalanceTest.*

using ::testing::_;

namespace ledger {
namespace endpoint {
namespace bitflyer {

class GetBalanceTest : public testing::Test {
 protected:
  base::test::TaskEnvironment task_environment_;
  MockLedgerImpl mock_ledger_impl_;
  GetBalance balance_{&mock_ledger_impl_};
};

TEST_F(GetBalanceTest, ServerOK) {
  ON_CALL(*mock_ledger_impl_.mock_client(), LoadURL(_, _))
      .WillByDefault(
          [](mojom::UrlRequestPtr request, LoadURLCallback callback) {
            auto response = mojom::UrlResponse::New();
            response->status_code = 200;
            response->url = request->url;
            response->body = R"({
              "account_hash": "ad0fd9160be16790893ff021b2f9ccf7f14b5a9f",
              "inventory": [
                {
                  "currency_code": "JPY",
                  "amount": 0.0,
                  "available": 0.0
                },
                {
                  "currency_code": "BTC",
                  "amount": 0.0,
                  "available": 0.0
                },
                {
                  "currency_code": "BCH",
                  "amount": 0.0,
                  "available": 0.0
                },
                {
                  "currency_code": "ETH",
                  "amount": 0.0,
                  "available": 0.0
                },
                {
                  "currency_code": "ETC",
                  "amount": 0.0,
                  "available": 0.0
                },
                {
                  "currency_code": "LTC",
                  "amount": 0.0,
                  "available": 0.0
                },
                {
                  "currency_code": "MONA",
                  "amount": 0.0,
                  "available": 0.0
                },
                {
                  "currency_code": "LSK",
                  "amount": 0.0,
                  "available": 0.0
                },
                {
                  "currency_code": "XRP",
                  "amount": 0.0,
                  "available": 0.0
                },
                {
                  "currency_code": "BAT",
                  "amount": 4.0,
                  "available": 4.0
                },
                {
                  "currency_code": "XLM",
                  "amount": 0.0,
                  "available": 0.0
                },
                {
                  "currency_code": "XEM",
                  "amount": 0.0,
                  "available": 0.0
                },
                {
                  "currency_code": "XTZ",
                  "amount": 0.0,
                  "available": 0.0
                }
              ]
            })";
            std::move(callback).Run(std::move(response));
          });

  balance_.Request(
      "4c2b665ca060d912fec5c735c734859a06118cc8",
      base::BindOnce([](const mojom::Result result, const double available) {
        EXPECT_EQ(result, mojom::Result::LEDGER_OK);
        EXPECT_EQ(available, 4.0);
      }));
}

TEST_F(GetBalanceTest, ServerError401) {
  ON_CALL(*mock_ledger_impl_.mock_client(), LoadURL(_, _))
      .WillByDefault(
          [](mojom::UrlRequestPtr request, LoadURLCallback callback) {
            auto response = mojom::UrlResponse::New();
            response->status_code = 401;
            response->url = request->url;
            response->body = "";
            std::move(callback).Run(std::move(response));
          });

  balance_.Request(
      "4c2b665ca060d912fec5c735c734859a06118cc8",
      base::BindOnce([](const mojom::Result result, const double available) {
        EXPECT_EQ(result, mojom::Result::EXPIRED_TOKEN);
        EXPECT_EQ(available, 0.0);
      }));
}

TEST_F(GetBalanceTest, ServerErrorRandom) {
  ON_CALL(*mock_ledger_impl_.mock_client(), LoadURL(_, _))
      .WillByDefault(
          [](mojom::UrlRequestPtr request, LoadURLCallback callback) {
            auto response = mojom::UrlResponse::New();
            response->status_code = 453;
            response->url = request->url;
            response->body = "";
            std::move(callback).Run(std::move(response));
          });

  balance_.Request(
      "4c2b665ca060d912fec5c735c734859a06118cc8",
      base::BindOnce([](const mojom::Result result, const double available) {
        EXPECT_EQ(result, mojom::Result::LEDGER_ERROR);
        EXPECT_EQ(available, 0.0);
      }));
}

}  // namespace bitflyer
}  // namespace endpoint
}  // namespace ledger
