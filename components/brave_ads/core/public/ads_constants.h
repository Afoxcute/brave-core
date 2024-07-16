/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_PUBLIC_ADS_CONSTANTS_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_PUBLIC_ADS_CONSTANTS_H_

namespace brave_ads::data::resource {

inline constexpr char kCatalogJsonSchemaFilename[] = "catalog-schema.json";

// Deprecated client state JSON file. State from this file will be moved to the
// database.
inline constexpr char kDeprecatedClientStateFilename[] = "client.json";

}  // namespace brave_ads::data::resource

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_PUBLIC_ADS_CONSTANTS_H_
