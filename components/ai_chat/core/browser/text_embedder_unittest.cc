/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/ai_chat/core/browser/text_embedder.h"

#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/test/task_environment.h"
#include "brave/components/ai_chat/core/browser/local_models_updater.h"
#include "brave/components/constants/brave_paths.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace ai_chat {

class TextEmbedderUnitTest : public testing::Test {
 public:
  TextEmbedderUnitTest() = default;
  ~TextEmbedderUnitTest() override = default;

  void SetUp() override {
    base::FilePath test_dir;
    base::PathService::Get(brave::DIR_TEST_DATA, &test_dir);
    model_dir_ =
        test_dir.AppendASCII("leo").AppendASCII("leo-local-models-updater");
  }

 protected:
  base::test::TaskEnvironment task_environment_;
  base::FilePath model_dir_;
};

TEST_F(TextEmbedderUnitTest, Create) {
  EXPECT_FALSE(TextEmbedder::Create(base::FilePath()));
  EXPECT_FALSE(TextEmbedder::Create(
      base::FilePath(model_dir_.AppendASCII("model.tflite"))));
  EXPECT_TRUE(TextEmbedder::Create(
      base::FilePath(model_dir_.AppendASCII(kUniversalQAModelName))));
}

TEST_F(TextEmbedderUnitTest, SplitSegments) {
  auto embedder = TextEmbedder::Create(
      base::FilePath(model_dir_.AppendASCII(kUniversalQAModelName)));
  ASSERT_TRUE(embedder);

  struct {
    std::string input;
    std::vector<std::string> expected;
  } test_cases[] = {{"", {}},
                    {"Hello", {"Hello"}},
                    {"Hello.", {"Hello."}},
                    {"Hello. World!", {"Hello", "World!"}},
                    {"Hello, World. Brave!", {"Hello, World", "Brave!"}},
                    {"Hello.World. This. is. the way.",
                     {"Hello.World", "This", "is", "the way."}},
                    {"IP address. 127.0.0.1", {"IP address", "127.0.0.1"}}};
  for (const auto& test_case : test_cases) {
    SCOPED_TRACE(test_case.input);
    EXPECT_EQ(test_case.expected, embedder->SplitSegments(test_case.input));
  }

  constexpr char kSegmentedText[] =
      "A. B. C. D. E. F. G. H. I. J. K. L. M. N. "
      "O. P. Q. R. S. T. U. V. W. X. Y. Z";
  struct {
    size_t segments_size_limit;
    std::vector<std::string> expected;
  } segments_size_test_cases[] = {
      {2, {"A B C D E F G H I J K L M", "N O P Q R S T U V W X Y Z"}},
      {3, {"A B C D E F G H", "I J K L M N O P", "Q R S T U V W X", "Y Z"}},
      {5,
       {"A B C D E", "F G H I J", "K L M N O", "P Q R S T", "U V W X Y", "Z"}},
      {7,
       {"A B C", "D E F", "G H I", "J K L", "M N O", "P Q R", "S T U", "V W X",
        "Y Z"}},
      {11,
       {"A B", "C D", "E F", "G H", "I J", "K L", "M N", "O P", "Q R", "S T",
        "U V", "W X", "Y Z"}},
      {13,
       {"A B", "C D", "E F", "G H", "I J", "K L", "M N", "O P", "Q R", "S T",
        "U V", "W X", "Y Z"}},
      {17, {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
            "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"}},
      {19, {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
            "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"}},
      {23, {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
            "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"}},
  };
  for (const auto& test_case : segments_size_test_cases) {
    SCOPED_TRACE(test_case.segments_size_limit);
    TextEmbedder::SetSegmentSizeLimitForTesting(test_case.segments_size_limit);
    EXPECT_EQ(test_case.expected, embedder->SplitSegments(kSegmentedText));
  }
}

}  // namespace ai_chat
