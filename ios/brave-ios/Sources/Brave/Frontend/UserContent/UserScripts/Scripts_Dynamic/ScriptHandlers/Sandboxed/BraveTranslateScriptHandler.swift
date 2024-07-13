// Copyright 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

import Foundation
import NaturalLanguage
import Shared
import WebKit
import os.log

enum BraveTranslateError: Error {
  case invalidURL
  case invalidLanguage
  case sameLanguage
  case invalidPageSource
  case invalidTranslationResponseCode
  case invalidTranslationResponse
  case otherError
}

class BraveTranslateScriptHandler: NSObject, TabContentScript {
  private weak var tab: Tab?
  private let recognizer = NLLanguageRecognizer()
  private static let namespace = "translate_\(uniqueID)"

  init(tab: Tab) {
    self.tab = tab
    super.init()
  }

  static let scriptName = "BraveTranslateScript"
  static let scriptId = UUID().uuidString
  static let messageHandlerName = "TranslateMessage"
  static let scriptSandbox: WKContentWorld = .defaultClient
  static let userScript: WKUserScript? = {
    guard var script = loadUserScript(named: scriptName) else {
      return nil
    }

    // HACKS! Need a better way to do this.
    // Chromium Scripts do NOT have a secure message handler and cannot be sandboxed the same way!
    return WKUserScript(
      source: script
        .replacingOccurrences(of: "$<brave_translate_script>", with: namespace)
        .replacingOccurrences(of: "$<message_handler>", with: messageHandlerName),
      injectionTime: .atDocumentEnd,
      forMainFrameOnly: true,
      in: scriptSandbox
    )
  }()

  private func executePageFunction(name functionName: String) async -> String? {
    guard let webView = tab?.webView else { return nil }

    let (result, error) = await webView.evaluateSafeJavaScript(
      functionName: "window.__firefox__.\(BraveTranslateScriptHandler.namespace).\(functionName)",
      contentWorld: BraveTranslateScriptHandler.scriptSandbox,
      asFunction: true
    )

    if let error = error {
      Logger.module.error("Unable to execute page function \(functionName) error: \(error)")
      return nil
    }

    guard let result = result as? String else {
      Logger.module.error("Invalid Page Result")
      return nil
    }

    return result
  }

  private func getPageSource() async -> String? {
    return await executePageFunction(name: "getPageSource")
  }

  @MainActor
  func guessLanguage() async -> Locale.Language? {
    if let rawPageSource = await executePageFunction(name: "getRawPageSource") {
      recognizer.reset()
      recognizer.processString(rawPageSource)

      if let dominantLanguage = recognizer.dominantLanguage, dominantLanguage != .undetermined {
        return Locale.Language(identifier: dominantLanguage.rawValue)
      }
    }

    if let languageCode = await executePageFunction(name: "getPageLanguage") {
      return Locale.Language(identifier: languageCode)
    }

    return Locale.current.language
  }

  func getLanguageInfo() async throws -> (Locale.Language, Locale.Language) {
    let pageLanguage = await guessLanguage() ?? Locale.current.language
    return (Locale.current.language, pageLanguage)
  }

  func activateScript() {
    guard let webView = tab?.webView else {
      return
    }

    Task { @MainActor in
      await webView.evaluateSafeJavaScript(
        functionName: "window.__gCrWeb.translate.startTranslation",
        args: ["ja", "en"],
        contentWorld: BraveTranslateScriptHandler.scriptSandbox,
        asFunction: true
      )
    }
  }

  func userContentController(
    _ userContentController: WKUserContentController,
    didReceiveScriptMessage message: WKScriptMessage,
    replyHandler: @escaping (Any?, String?) -> Void
  ) {
    guard let body = message.body as? [String: Any] else {
      Logger.module.error("INVALID BRAVE TRANSLATE MESSAGE")
      return
    }
    
    if body["command"] as? String == "ready" {
      print("TRANSLATE IS READY!")
      
      Task { @MainActor in
        activateScript()
      }
    }
    
    replyHandler(nil, nil)
  }
}
