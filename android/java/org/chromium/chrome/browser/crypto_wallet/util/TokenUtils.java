/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.crypto_wallet.util;

import android.annotation.SuppressLint;

import androidx.annotation.NonNull;

import org.chromium.base.Callbacks;
import org.chromium.brave_wallet.mojom.BlockchainRegistry;
import org.chromium.brave_wallet.mojom.BlockchainToken;
import org.chromium.brave_wallet.mojom.BraveWalletService;
import org.chromium.brave_wallet.mojom.CoinType;
import org.chromium.brave_wallet.mojom.NetworkInfo;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.stream.Stream;

public class TokenUtils {
    /**
     * Type of token used for filtering an array of {@code BlockchainToken}.
     * See {@link #filterTokens(NetworkInfo, BlockchainToken[], TokenUtils.TokenType, boolean)}.
     */
    public enum TokenType {
        // Filter out all tokens that are not NFTs.
        // Note: native assets won't be included when using this token type.
        NFTS,

        // Filter out all tokens that don't belong to ERC20 standard.
        ERC20,

        // Filter out all tokens that don't belong to ERC721 standard.
        ERC721,

        // Filter out all tokens that are not solana coins.
        SOL,

        // Filter out all tokens that are NFTs.
        // Note: a custom asset may or may not be an NFT.
        NON_NFTS,

        // Don't apply any filter, the list will include all tokens.
        ALL
    }

    /**
     * Filter tokens by type and add native token (when needed).
     *
     * BlockchainRegistry.getAllTokens does not return the chain's native token;
     * BraveWalletService.getUserAssets contains the native asset on init, but:
     *   - the returned BlockchainToken object has no logo
     *   - the asset can be removed at some point by removeUserAsset
     * To make things consistent, here we do the following:
     *   - Decide whether a native asset exists in the input token array with isSameToken
     *   - If exists, replace the BlockchainToken object with the one generated by
     * Utils.makeNetworkAsset
     *   - If not exist, insert the BlockchainToken object generated by Utils.makeNetworkAsset to
     * front
     *
     * See `refreshVisibleTokenInfo` in components/brave_wallet_ui/common/async/lib.ts.
     */
    private static BlockchainToken[] filterTokens(NetworkInfo selectedNetwork,
            BlockchainToken[] tokens, TokenType tokenType, boolean keepVisibleOnly) {
        BlockchainToken nativeAsset = Utils.makeNetworkAsset(selectedNetwork);
        Stream<BlockchainToken> tokenStream = Arrays.stream(tokens).filter(token -> {
            final boolean typeFilter;
            switch (tokenType) {
                case NFTS:
                    typeFilter = token.isNft;
                    break;
                case ERC20:
                    typeFilter = token.isErc20;
                    break;
                case ERC721:
                    typeFilter = token.isErc721;
                    break;
                case SOL:
                    typeFilter = token.coin == CoinType.SOL;
                    break;
                case NON_NFTS:
                    typeFilter = !token.isNft;
                    break;
                case ALL:
                    typeFilter = true;
                    break;
                default:
                    throw new UnsupportedOperationException("Token type not supported.");
            }
            return typeFilter && !isSameToken(token, nativeAsset)
                    && (!keepVisibleOnly || token.visible);
        });

        // When token type is NFTS the native assets should not be added.
        // For all the other cases we add them.
        if (tokenType != TokenType.NFTS) {
            tokenStream = Stream.concat(Stream.of(nativeAsset), tokenStream);
        }
        return tokenStream.toArray(BlockchainToken[] ::new);
    }

    public static void getVisibleUserAssetsFiltered(BraveWalletService braveWalletService,
            NetworkInfo selectedNetwork, int coinType, TokenType tokenType,
            Callbacks.Callback1<BlockchainToken[]> callback) {
        braveWalletService.getUserAssets(
                selectedNetwork.chainId, coinType, (BlockchainToken[] tokens) -> {
                    BlockchainToken[] filteredTokens =
                            filterTokens(selectedNetwork, tokens, tokenType, true);
                    callback.call(filteredTokens);
                });
    }

    /**
     * Gets all tokens from a given single network, includes user assets and filters out tokens
     * different from a given type.
     *
     * @param braveWalletService BraveWalletService to retrieve user asset from core.
     * @param blockchainRegistry BraveChainRegistry to retrieve all tokens from core.
     * @param selectedNetwork Selected network whose tokens will be retrieved.
     * @param tokenType Token type used for filtering.
     * @param callback Callback containing a filtered array of tokens for the given network.
     */
    public static void getAllTokensFiltered(
            BraveWalletService braveWalletService,
            BlockchainRegistry blockchainRegistry,
            NetworkInfo selectedNetwork,
            TokenType tokenType,
            Callbacks.Callback1<BlockchainToken[]> callback) {
        blockchainRegistry.getAllTokens(
                selectedNetwork.chainId,
                selectedNetwork.coin,
                tokens ->
                        braveWalletService.getUserAssets(
                                selectedNetwork.chainId,
                                selectedNetwork.coin,
                                userTokens -> {
                                    BlockchainToken[] filteredTokens =
                                            filterTokens(
                                                    selectedNetwork,
                                                    distinctiveConcatenatedArrays(
                                                            tokens, userTokens),
                                                    tokenType,
                                                    false);
                                    callback.call(filteredTokens);
                                }));
    }

    public static void getUserOrAllTokensFiltered(
            BraveWalletService braveWalletService,
            BlockchainRegistry blockchainRegistry,
            NetworkInfo selectedNetwork,
            int coinType,
            TokenType tokenType,
            boolean userAssetsOnly,
            Callbacks.Callback1<BlockchainToken[]> callback) {
        if (JavaUtils.anyNull(braveWalletService, blockchainRegistry)) return;
        if (userAssetsOnly)
            getVisibleUserAssetsFiltered(
                    braveWalletService, selectedNetwork, coinType, tokenType, callback);
        else
            getAllTokensFiltered(
                    braveWalletService, blockchainRegistry, selectedNetwork, tokenType, callback);
    }

    /**
     * Concatenates arrays, add only elements of arraySecond that are not present in the arrayFirst
     * @param arrayFirst first array to be added in the result
     * @param arraySecond second array, only distinctive elements are added in result by comparing
     *         with the items of arrayFirst
     * @return concatenated array
     */
    public static BlockchainToken[] distinctiveConcatenatedArrays(
            BlockchainToken[] arrayFirst, BlockchainToken[] arraySecond) {
        List<BlockchainToken> both = new ArrayList<>();

        Collections.addAll(both, arrayFirst);
        for (BlockchainToken tokenSecond : arraySecond) {
            boolean add = true;
            for (BlockchainToken tokenFirst : arrayFirst) {
                if (isSameToken(tokenFirst, tokenSecond)) {
                    add = false;
                    break;
                }
            }
            if (add) {
                both.add(tokenSecond);
            }
        }

        return both.toArray(new BlockchainToken[0]);
    }

    /**
     * Checks if two tokens are equal. Two tokens are equal if:
     * - Chain ID matches.
     * - Symbol matches.
     * - Token ID matches.
     * - Contract address (ignore case) matches.
     *
     * @param token1 First token to compare.
     * @param token2 Second token to compare.
     * @return {@code true} if two tokens are equal, {@code false} otherwise.
     */
    public static boolean isSameToken(
            @NonNull BlockchainToken token1, @NonNull BlockchainToken token2) {
        return token1.chainId.equals(token2.chainId) && token1.symbol.equals(token2.symbol)
                && token1.tokenId.equals(token2.tokenId)
                && token1.contractAddress.equalsIgnoreCase(token2.contractAddress);
    }

}
