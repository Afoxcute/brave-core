/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_PRIVACY_TOKENS_UNBLINDED_PAYMENT_TOKENS_UNBLINDED_PAYMENT_TOKEN_UTIL_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_PRIVACY_TOKENS_UNBLINDED_PAYMENT_TOKENS_UNBLINDED_PAYMENT_TOKEN_UTIL_H_

#include "bat/ads/internal/privacy/tokens/unblinded_payment_tokens/unblinded_payment_token_info.h"

namespace ads::privacy {

UnblindedPaymentTokenList GetUnblindedPaymentTokens(int count);
const UnblindedPaymentTokenList& GetAllUnblindedPaymentTokens();

void SetUnblindedPaymentTokens(
    const UnblindedPaymentTokenList& unblinded_tokens);

void AddUnblindedPaymentTokens(
    const UnblindedPaymentTokenList& unblinded_tokens);

void RemoveUnblindedPaymentTokens(
    const UnblindedPaymentTokenList& unblinded_tokens);
void RemoveAllUnblindedPaymentTokens();

bool UnblindedPaymentTokenExists(
    const UnblindedPaymentTokenInfo& unblinded_token);

bool UnblindedPaymentTokensIsEmpty();

int UnblindedPaymentTokenCount();

}  // namespace ads::privacy

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_PRIVACY_TOKENS_UNBLINDED_PAYMENT_TOKENS_UNBLINDED_PAYMENT_TOKEN_UTIL_H_
