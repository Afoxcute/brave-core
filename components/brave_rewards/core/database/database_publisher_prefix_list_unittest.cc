/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/big_endian.h"
#include "base/strings/string_piece.h"
#include "base/test/task_environment.h"
#include "brave/components/brave_rewards/core/database/database_publisher_prefix_list.h"
#include "brave/components/brave_rewards/core/ledger_client_mock.h"
#include "brave/components/brave_rewards/core/ledger_impl_mock.h"
#include "brave/components/brave_rewards/core/publisher/protos/publisher_prefix_list.pb.h"

// npm run test -- brave_unit_tests --filter=DatabasePublisherPrefixListTest.*

using ::testing::_;
using ::testing::Invoke;

namespace ledger {
namespace database {

class DatabasePublisherPrefixListTest : public ::testing::Test {
 protected:
  std::unique_ptr<publisher::PrefixListReader> CreateReader(
      uint32_t prefix_count) {
    auto reader = std::make_unique<publisher::PrefixListReader>();
    if (prefix_count == 0) {
      return reader;
    }

    std::string prefixes;
    prefixes.resize(prefix_count * 4);
    for (uint32_t i = 0; i < prefix_count; ++i) {
      base::WriteBigEndian(&prefixes[i * 4], i);
    }

    publishers_pb::PublisherPrefixList message;
    message.set_prefix_size(4);
    message.set_compression_type(
        publishers_pb::PublisherPrefixList::NO_COMPRESSION);
    message.set_uncompressed_size(prefixes.size());
    message.set_prefixes(std::move(prefixes));

    std::string out;
    message.SerializeToString(&out);
    reader->Parse(out);
    return reader;
  }

  void ExpectStartsWith(const std::string& subject, const std::string& prefix) {
    EXPECT_EQ(subject.size() > prefix.size() ? subject.substr(0, prefix.size())
                                             : subject,
              prefix);
  }

  base::test::TaskEnvironment task_environment_;
  MockLedgerImpl mock_ledger_impl_;
  DatabasePublisherPrefixList database_prefix_list_{&mock_ledger_impl_};
};

TEST_F(DatabasePublisherPrefixListTest, Reset) {
  std::vector<std::string> commands;

  auto on_run_db_transaction = [&](mojom::DBTransactionPtr transaction,
                                   ledger::RunDBTransactionCallback callback) {
    ASSERT_TRUE(transaction);
    if (transaction) {
      for (auto& command : transaction->commands) {
        commands.push_back(std::move(command->command));
      }
    }
    commands.push_back("---");
    auto response = mojom::DBCommandResponse::New();
    response->status = mojom::DBCommandResponse::Status::RESPONSE_OK;
    std::move(callback).Run(std::move(response));
  };

  ON_CALL(*mock_ledger_impl_.rewards_service(), RunDBTransaction(_, _))
      .WillByDefault(Invoke(on_run_db_transaction));

  database_prefix_list_.Reset(CreateReader(100'001),
                              [](const mojom::Result) {});

  ASSERT_EQ(commands.size(), 5u);
  EXPECT_EQ(commands[0], "DELETE FROM publisher_prefix_list");
  ExpectStartsWith(commands[1],
                   "INSERT OR REPLACE INTO publisher_prefix_list (hash_prefix) "
                   "VALUES (x'00000000'),(x'00000001'),(x'00000002'),");
  EXPECT_EQ(commands[2], "---");
  EXPECT_EQ(commands[3],
            "INSERT OR REPLACE INTO publisher_prefix_list (hash_prefix) "
            "VALUES (x'000186A0')");
  EXPECT_EQ(commands[4], "---");
}

}  // namespace database
}  // namespace ledger
