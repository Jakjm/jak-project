#include <cstring>
#include <map>
#include <vector>
#include <algorithm>
#include "third-party/fmt/core.h"
#include "game_text.h"
#include "decompiler/ObjectFile/ObjectFileDB.h"
#include "common/goos/Reader.h"
#include "common/util/BitUtils.h"

namespace decompiler {
namespace {
template <typename T>
T get_word(const LinkedWord& word) {
  T result;
  ASSERT(word.kind() == LinkedWord::PLAIN_DATA);
  static_assert(sizeof(T) == 4, "bad get_word size");
  memcpy(&result, &word.data, 4);
  return result;
}

DecompilerLabel get_label(ObjectFileData& data, const LinkedWord& word) {
  ASSERT(word.kind() == LinkedWord::PTR);
  return data.linked_data.labels.at(word.label_id());
}

}  // namespace

/*
(deftype game-text (structure)
  ((id   uint32  :offset-assert 0)
   (text basic   :offset-assert 4)
   )
  )

(deftype game-text-info (basic)
  ((length      int32            :offset-assert 4)
   (language-id int32            :offset-assert 8)
   (group-name  basic            :offset-assert 12)
   (data        game-text :dynamic :offset-assert 16)
   )
  )
 */

GameTextResult process_game_text(ObjectFileData& data, GameTextVersion version) {
  GameTextResult result;
  auto& words = data.linked_data.words_by_seg.at(0);
  std::vector<int> read_words(words.size(), false);

  int offset = 0;

  // type tag for game-text-info
  if (words.at(offset).kind() != LinkedWord::TYPE_PTR ||
      words.front().symbol_name() != "game-text-info") {
    ASSERT(false);
  }
  read_words.at(offset)++;
  offset++;

  // length field
  read_words.at(offset)++;
  u32 text_count = get_word<u32>(words.at(offset++));

  // language-id field
  read_words.at(offset)++;
  u32 language = get_word<u32>(words.at(offset++));
  result.language = language;

  // group-name field
  read_words.at(offset)++;
  auto group_label = get_label(data, words.at(offset++));
  auto group_name = data.linked_data.get_goal_string_by_label(group_label);
  ASSERT(group_name == "common");
  // remember that we read these bytes
  auto group_start = (group_label.offset / 4) - 1;
  for (int j = 0; j < align16(8 + 1 + (int)group_name.length()) / 4; j++) {
    read_words.at(group_start + j)++;
  }

  // read each text...
  for (u32 i = 0; i < text_count; i++) {
    // id number
    read_words.at(offset)++;
    auto text_id = get_word<u32>(words.at(offset++));

    // label to string
    read_words.at(offset)++;
    auto text_label = get_label(data, words.at(offset++));

    // actual string
    auto text = data.linked_data.get_goal_string_by_label(text_label);
    result.total_text++;
    result.total_chars += text.length();

    // no duplicate ids
    if (result.text.find(text_id) != result.text.end()) {
      ASSERT(false);
    }

    // escape characters
    if (font_bank_exists(version)) {
      result.text[text_id] = get_font_bank(version)->convert_game_to_utf8(text.c_str());
    } else {
      result.text[text_id] = goos::get_readable_string(text.c_str());  // HACK!
    }

    // remember what we read (-1 for the type tag)
    auto string_start = (text_label.offset / 4) - 1;
    // 8 for type tag and length fields, 1 for null char.
    for (int j = 0, m = align16(8 + 1 + (int)text.length()) / 4;
         j < m && string_start + j < (int)read_words.size(); j++) {
      read_words.at(string_start + j)++;
    }
  }

  // alignment to the string section.
  while (offset & 3) {
    read_words.at(offset)++;
    offset++;
  }

  // make sure we read each thing at least once.
  // reading more than once is ok, some text is duplicated.
  for (int i = 0; i < int(words.size()); i++) {
    if (read_words[i] < 1) {
      std::string debug;
      data.linked_data.append_word_to_string(debug, words.at(i));
      printf("[%d] %d 0x%s\n", i, int(read_words[i]), debug.c_str());
      ASSERT(false);
    }
  }

  return result;
}

std::string write_game_text(
    const std::unordered_map<int, std::unordered_map<int, std::string>>& data) {
  // first sort languages:
  std::vector<int> langauges;
  for (const auto& lang : data) {
    langauges.push_back(lang.first);
  }
  std::sort(langauges.begin(), langauges.end());

  // build map
  std::map<int, std::vector<std::string>> text_by_id;
  for (auto lang : langauges) {
    for (auto& text : data.at(lang)) {
      text_by_id[text.first].push_back(text.second);
    }
  }

  // write!
  std::string result;  // = "\xEF\xBB\xBF";  // UTF-8 encode (don't need this anymore)
  result += fmt::format("(language-count {})\n", langauges.size());
  result += "(group-name \"common\")\n";
  for (auto& x : text_by_id) {
    result += fmt::format("(#x{:04x}\n  ", x.first);
    for (auto& y : x.second) {
      result += fmt::format("\"{}\"\n  ", y);
    }
    result += ")\n\n";
  }

  return result;
}
}  // namespace decompiler
