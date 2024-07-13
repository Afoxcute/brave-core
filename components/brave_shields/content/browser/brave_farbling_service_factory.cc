// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "brave/components/brave_shields/content/browser/brave_farbling_service_factory.h"

#include "base/no_destructor.h"
#include "brave/components/brave_shields/content/browser/brave_farbling_service.h"
#include "chrome/browser/profiles/profile.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/browser_context.h"

namespace brave {

// static
BraveFarblingServiceFactory* BraveFarblingServiceFactory::GetInstance() {
  static base::NoDestructor<BraveFarblingServiceFactory> instance;
  return instance.get();
}

// static
BraveFarblingService* BraveFarblingServiceFactory::GetForProfile(
    Profile* profile) {
  return static_cast<BraveFarblingService*>(
      GetInstance()->GetServiceForBrowserContext(profile, true));
}

BraveFarblingServiceFactory::BraveFarblingServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "BraveFarblingServiceFactory",
          BrowserContextDependencyManager::GetInstance()) {}

BraveFarblingServiceFactory::~BraveFarblingServiceFactory() = default;

std::unique_ptr<KeyedService>
BraveFarblingServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  return std::make_unique<BraveFarblingService>();
}

}  // namespace brave
