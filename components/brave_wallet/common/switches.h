// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef BRAVE_COMPONENTS_BRAVE_WALLET_COMMON_SWITCHES_H_
#define BRAVE_COMPONENTS_BRAVE_WALLET_COMMON_SWITCHES_H_

namespace brave_wallet {
namespace switches {

// Allows auto unlocking wallet password with command line.
inline constexpr char kDevWalletPassword[] = "dev-wallet-password";

// Ratios service dev URL
inline constexpr char kAssetRatioDevUrl[] = "asset-ratio-dev-url";

// Bitcoin ordinals rpc mainnet endpoint switch.
inline constexpr char kBitcoinOrdinalsMainnetRpcUrl[] =
    "bitcoin-ordinals-mainnet-rpc-url";

// Bitcoin ordinals rpc testnet endpoint switch.
inline constexpr char kBitcoinOrdinalsTestnetRpcUrl[] =
    "bitcoin-ordinals-testnet-rpc-url";

// ZCash rpc mainnet endpoint switch.
inline constexpr char kZCashMainnetRpcUrl[] = "zcash-mainnet-rpc-url";

// ZCash rpc testnet endpoint switch.
inline constexpr char kZCashTestnetRpcUrl[] = "zcash-testnet-rpc-url";

}  // namespace switches
}  // namespace brave_wallet

#endif  // BRAVE_COMPONENTS_BRAVE_WALLET_COMMON_SWITCHES_H_
