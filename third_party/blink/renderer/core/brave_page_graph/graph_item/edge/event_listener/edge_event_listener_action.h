/* Copyright (c) 2019 The Brave Software Team. Distributed under the MPL2
 * license. This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_GRAPH_ITEM_EDGE_EVENT_LISTENER_EDGE_EVENT_LISTENER_ACTION_H_
#define BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_GRAPH_ITEM_EDGE_EVENT_LISTENER_EDGE_EVENT_LISTENER_ACTION_H_

#include <libxml/tree.h>
#include <string>

#include "third_party/blink/renderer/platform/wtf/casting.h"

#include "brave/third_party/blink/renderer/core/brave_page_graph/types.h"

#include "brave/third_party/blink/renderer/core/brave_page_graph/graph_item/edge/edge.h"

namespace brave_page_graph {

class NodeActor;
class NodeHTMLElement;
class PageGraph;

class EdgeEventListenerAction : public Edge {
  friend class PageGraph;

 public:
  EdgeEventListenerAction() = delete;
  ~EdgeEventListenerAction() override;

  const std::string& GetEventType() const { return event_type_; }
  EventListenerId GetListenerId() const { return listener_id_; }
  NodeActor* GetListenerNode() const { return listener_script_; }
  ScriptId GetListenerScriptId() const;

  ItemDesc GetItemDesc() const override;

  void AddGraphMLAttributes(xmlDocPtr doc,
                            xmlNodePtr parent_node) const override;

  bool IsEdgeEventListenerAction() const override;

  virtual bool IsEdgeEventListenerAdd() const;
  virtual bool IsEdgeEventListenerRemove() const;

 protected:
  EdgeEventListenerAction(PageGraph* const graph,
                          NodeActor* const out_node,
                          NodeHTMLElement* const in_node,
                          const std::string& event_type,
                          const EventListenerId listener_id,
                          NodeActor* listener_script);

 private:
  const std::string event_type_;
  const EventListenerId listener_id_;
  NodeActor* listener_script_;
};

}  // namespace brave_page_graph

namespace blink {

template <>
struct DowncastTraits<brave_page_graph::EdgeEventListenerAction> {
  static bool AllowFrom(const brave_page_graph::Edge& edge) {
    return edge.IsEdgeEventListenerAction();
  }
  static bool AllowFrom(const brave_page_graph::GraphItem& graph_item) {
    return IsA<brave_page_graph::EdgeEventListenerAction>(
        DynamicTo<brave_page_graph::Edge>(graph_item));
  }
};

}  // namespace blink

#endif  // BRAVE_THIRD_PARTY_BLINK_RENDERER_CORE_BRAVE_PAGE_GRAPH_GRAPH_ITEM_EDGE_EVENT_LISTENER_EDGE_EVENT_LISTENER_ACTION_H_
