/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/ai_chat/core/browser/text_embedder.h"

#include "base/hash/hash.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/sequence_checker.h"
#include "base/strings/string_split.h"
#include "base/task/bind_post_task.h"
#include "base/task/sequenced_task_runner.h"
#include "base/task/thread_pool.h"
#include "third_party/tflite/src/tensorflow/lite/core/api/op_resolver.h"
#include "third_party/tflite_support/src/tensorflow_lite_support/cc/task/text/utils/text_op_resolver.h"

using TFLiteTextEmbedder = tflite::task::text::TextEmbedder;
using TFLiteTextEmbedderOptions = tflite::task::text::TextEmbedderOptions;
using tflite::task::text::CreateTextOpResolver;

namespace ai_chat {

// static
std::unique_ptr<TextEmbedder> TextEmbedder::Create(
    const base::FilePath& model_path) {
  auto embedder = base::WrapUnique(new TextEmbedder());
  TFLiteTextEmbedderOptions options;
  options.mutable_base_options()->mutable_model_file()->set_file_name(
      model_path.value().c_str());
  auto maybe_text_embedder =
      TFLiteTextEmbedder::CreateFromOptions(options, CreateTextOpResolver());
  if (!maybe_text_embedder.ok()) {
    VLOG(1) << maybe_text_embedder.status().ToString();
    return nullptr;
  }

  embedder->tflite_text_embedder_ = std::move(maybe_text_embedder.value());
  return embedder;
}

TextEmbedder::TextEmbedder()
    : owner_task_runner_(base::SequencedTaskRunner::GetCurrentDefault()),
      embedder_task_runner_(
          base::ThreadPool::CreateSequencedTaskRunner({base::MayBlock()})) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(owner_sequence_checker_);
  DETACH_FROM_SEQUENCE(embedder_sequence_checker_);
}

TextEmbedder::~TextEmbedder() = default;

void TextEmbedder::GetTopSimilarityWithPromptTilContextLimit(
    const std::string& prompt,
    const std::string& text,
    uint32_t context_limit,
    TopSimilarityCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(owner_sequence_checker_);
  embedder_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(
          &TextEmbedder::GetTopSimilarityWithPromptTilContextLimitInternal,
          weak_ptr_factory_.GetWeakPtr(), prompt, text, context_limit,
          base::BindPostTaskToCurrentDefault(std::move(callback))));
}

void TextEmbedder::GetTopSimilarityWithPromptTilContextLimitInternal(
    const std::string& prompt,
    const std::string& text,
    uint32_t context_limit,
    TopSimilarityCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(embedder_sequence_checker_);
  auto text_hash = base::FastHash(base::as_bytes(base::make_span(text)));
  if (text_hash != text_hash_) {
    text_hash_ = text_hash;
    segments_ = SplitSegments(text);
    auto status = EmbedSegments();
    if (!status.ok()) {
      std::move(callback).Run(base::unexpected(status.ToString()));
      return;
    }
  }

  using ScoreType = std::pair<size_t, double>;
  std::vector<ScoreType> ranked_sentences;
  auto maybe_prompt_embed = tflite_text_embedder_->Embed(prompt);
  if (!maybe_prompt_embed.ok()) {
    std::move(callback).Run(
        base::unexpected(maybe_prompt_embed.status().ToString()));
    return;
  }
  for (size_t i = 0; i < embeddings_.size(); i++) {
    auto maybe_similarity = tflite_text_embedder_->CosineSimilarity(
        maybe_prompt_embed->embeddings(0).feature_vector(),
        embeddings_[i].embeddings(0).feature_vector());
    if (!maybe_similarity.ok()) {
      std::move(callback).Run(
          base::unexpected(maybe_similarity.status().ToString()));
      return;
    }
    ranked_sentences.push_back(std::make_pair(i, maybe_similarity.value()));
  }
  std::sort(ranked_sentences.begin(), ranked_sentences.end(),
            [](const ScoreType& lhs, const ScoreType& rhs) {
              return lhs.second > rhs.second;
            });
  std::vector<size_t> top_k_indices;
  size_t total_length = 0;
  std::string refined_page_content = "";
  for (const auto& ranked_sentence : ranked_sentences) {
    const auto& segment = segments_[ranked_sentence.first];
    if (total_length + segment.size() > context_limit) {
      break;
    }
    total_length += segment.size();
    top_k_indices.push_back(ranked_sentence.first);
  }
  std::sort(top_k_indices.begin(), top_k_indices.end());
  for (const auto& index : top_k_indices) {
    refined_page_content += segments_[index] + ". ";
  }
  VLOG(4) << "Refined page content: " << refined_page_content;
  std::move(callback).Run(base::ok(refined_page_content));
}

std::vector<std::string> TextEmbedder::SplitSegments(const std::string& text) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(embedder_sequence_checker_);
  auto segments = base::SplitStringUsingSubstr(
      text, ". ", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
  DVLOG(4) << "Segments: " << segments.size();
  if (segments.size() > 300) {
    std::vector<std::string> new_segments;
    size_t join_size =
        static_cast<size_t>(std::ceil(segments.size() / 300));
    std::string new_segment = "";
    for (size_t i = 0; i < segments.size(); ++i) {
      new_segment += segments[i];
      if ((i + 1) % join_size == 0) {
        new_segments.push_back(new_segment);
        new_segment = "";
      } else if (i != segments.size() - 1) {
        new_segment += " ";
      }
      if (i == segments.size() - 1 && !new_segment.empty()) {
        new_segments.push_back(new_segment);
        new_segment = "";
      }
    }
    segments = new_segments;
    DVLOG(4) << "New Segments: " << segments.size();
  }
  return segments;
}

absl::Status TextEmbedder::EmbedText(
    const std::string& text,
    tflite::task::processor::EmbeddingResult& embedding) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(embedder_sequence_checker_);
  auto maybe_embedding = tflite_text_embedder_->Embed(text);
  if (!maybe_embedding.ok()) {
    return maybe_embedding.status();
  }
  embedding = maybe_embedding.value();
  return absl::OkStatus();
}

absl::Status TextEmbedder::EmbedSegments() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(embedder_sequence_checker_);
  if (segments_.empty()) {
    return absl::InvalidArgumentError("No segments to embed.");
  }
  for (const auto& segment : segments_) {
    tflite::task::processor::EmbeddingResult embedding;
    auto status = EmbedText(segment, embedding);
    if (!status.ok()) {
      return status;
    }
    embeddings_.push_back(embedding);
  }
  return absl::OkStatus();
}

}  // namespace ai_chat
