/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/webui/brave_education/brave_education_ui.h"
#include "chrome/browser/ui/webui/new_tab_page/new_tab_page_ui.h"
#include "chrome/browser/ui/webui/whats_new/whats_new_ui.h"

#define CommandHandlerFactory \
  CommandHandlerFactory, brave_education::BraveEducationUI

#include "src/chrome/browser/chrome_browser_interface_binders.cc"

#undef CommandHandlerFactory
