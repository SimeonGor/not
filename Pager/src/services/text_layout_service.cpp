#include "services/text_layout_service.h"

namespace {
void skipSpacesPGM(const char *text, uint16_t &index) {
  while (pgm_read_byte(text + index) != '\0') {
    const char ch = static_cast<char>(pgm_read_byte(text + index));
    if (ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r') {
      break;
    }
    index++;
  }
}

void skipSpacesRam(const char *text, uint16_t &index) {
  while (text[index] != '\0') {
    const char ch = text[index];
    if (ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r') {
      break;
    }
    index++;
  }
}

// Дописать слово в текущую строку: при lineLen>0 перед словом — один пробел (если помещается).
template <typename ReadChar>
static bool tryAppendWordWithSpace(TextLayout &layout, uint8_t &lineLen, uint16_t wordStart,
                                   uint8_t wordLen, ReadChar readChar) {
  const uint8_t sep = (lineLen > 0) ? 1u : 0u;
  if (lineLen + sep + wordLen > LINE_CHAR_CAPACITY) {
    return false;
  }
  if (sep != 0) {
    layout.lines[layout.lineCount][lineLen++] = ' ';
  }
  for (uint8_t i = 0; i < wordLen; i++) {
    layout.lines[layout.lineCount][lineLen++] = readChar(static_cast<uint16_t>(wordStart + i));
  }
  return true;
}
}  // namespace

TextLayout TextLayoutService::wrapProgmemText(const char *textFromProgmem) {
  TextLayout layout = {};
  uint16_t index = 0;

  auto readPgm = [textFromProgmem](const uint16_t i) {
    return static_cast<char>(pgm_read_byte(textFromProgmem + i));
  };

  while (layout.lineCount < MAX_WRAPPED_LINES) {
    skipSpacesPGM(textFromProgmem, index);
    if (pgm_read_byte(textFromProgmem + index) == '\0') {
      break;
    }

    uint8_t lineLen = 0;
    uint16_t wordStart = index;
    uint8_t wordLen = 0;

    while (layout.lineCount < MAX_WRAPPED_LINES) {
      const char ch = static_cast<char>(pgm_read_byte(textFromProgmem + index));
      if (ch == '\0') {
        break;
      }

      if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
        if (wordLen > 0) {
          if (tryAppendWordWithSpace(layout, lineLen, wordStart, wordLen, readPgm)) {
            index = wordStart + wordLen;
            wordStart = index;
            wordLen = 0;
            skipSpacesPGM(textFromProgmem, index);
            wordStart = index;
            continue;
          }
          break;
        }

        index++;
        wordStart = index;
        continue;
      }

      wordLen++;
      index++;

      if (wordLen > LINE_CHAR_CAPACITY) {
        for (uint8_t i = 0; i < LINE_CHAR_CAPACITY; i++) {
          layout.lines[layout.lineCount][i] = readPgm(static_cast<uint16_t>(index - wordLen + i));
        }
        lineLen = LINE_CHAR_CAPACITY;
        index = wordStart + wordLen;
        break;
      }
    }

    if (wordLen > 0) {
      const uint8_t sep = (lineLen > 0) ? 1u : 0u;
      if (lineLen + sep + wordLen <= LINE_CHAR_CAPACITY) {
        if (sep != 0) {
          layout.lines[layout.lineCount][lineLen++] = ' ';
        }
        for (uint8_t i = 0; i < wordLen; i++) {
          layout.lines[layout.lineCount][lineLen++] = readPgm(static_cast<uint16_t>(wordStart + i));
        }
        index = wordStart + wordLen;
      } else if (lineLen > 0) {
        index = wordStart;
      } else {
        const uint8_t copyLen = min(wordLen, LINE_CHAR_CAPACITY);
        for (uint8_t i = 0; i < copyLen; i++) {
          layout.lines[layout.lineCount][lineLen++] = readPgm(static_cast<uint16_t>(wordStart + i));
        }
        index = wordStart + copyLen;
      }
    }

    if (lineLen == 0) {
      break;
    }

    layout.lines[layout.lineCount][lineLen] = '\0';
    layout.lineCount++;
  }

  const int32_t totalContentHeight = static_cast<int32_t>(layout.lineCount) * LINE_HEIGHT;
  const int32_t maxScroll = totalContentHeight - MESSAGE_SCROLL_VIEW_HEIGHT;
  layout.maxScrollOffset = (maxScroll > 0) ? static_cast<int16_t>(maxScroll) : 0;
  return layout;
}

TextLayout TextLayoutService::wrapRamText(const char *textFromRam) {
  TextLayout layout = {};
  if (textFromRam == nullptr || textFromRam[0] == '\0') {
    layout.lines[0][0] = ' ';
    layout.lines[0][1] = '\0';
    layout.lineCount = 1;
    layout.maxScrollOffset = 0;
    return layout;
  }

  uint16_t index = 0;

  auto readRam = [textFromRam](const uint16_t i) { return textFromRam[i]; };

  while (layout.lineCount < MAX_WRAPPED_LINES) {
    skipSpacesRam(textFromRam, index);
    if (textFromRam[index] == '\0') {
      break;
    }

    uint8_t lineLen = 0;
    uint16_t wordStart = index;
    uint8_t wordLen = 0;

    while (layout.lineCount < MAX_WRAPPED_LINES) {
      const char ch = textFromRam[index];
      if (ch == '\0') {
        break;
      }

      if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
        if (wordLen > 0) {
          if (tryAppendWordWithSpace(layout, lineLen, wordStart, wordLen, readRam)) {
            index = wordStart + wordLen;
            wordStart = index;
            wordLen = 0;
            skipSpacesRam(textFromRam, index);
            wordStart = index;
            continue;
          }
          break;
        }

        index++;
        wordStart = index;
        continue;
      }

      wordLen++;
      index++;

      if (wordLen > LINE_CHAR_CAPACITY) {
        for (uint8_t i = 0; i < LINE_CHAR_CAPACITY; i++) {
          layout.lines[layout.lineCount][i] = readRam(static_cast<uint16_t>(index - wordLen + i));
        }
        lineLen = LINE_CHAR_CAPACITY;
        index = wordStart + wordLen;
        break;
      }
    }

    if (wordLen > 0) {
      const uint8_t sep = (lineLen > 0) ? 1u : 0u;
      if (lineLen + sep + wordLen <= LINE_CHAR_CAPACITY) {
        if (sep != 0) {
          layout.lines[layout.lineCount][lineLen++] = ' ';
        }
        for (uint8_t i = 0; i < wordLen; i++) {
          layout.lines[layout.lineCount][lineLen++] = readRam(static_cast<uint16_t>(wordStart + i));
        }
        index = wordStart + wordLen;
      } else if (lineLen > 0) {
        index = wordStart;
      } else {
        const uint8_t copyLen = min(wordLen, LINE_CHAR_CAPACITY);
        for (uint8_t i = 0; i < copyLen; i++) {
          layout.lines[layout.lineCount][lineLen++] = readRam(static_cast<uint16_t>(wordStart + i));
        }
        index = wordStart + copyLen;
      }
    }

    if (lineLen == 0) {
      break;
    }

    layout.lines[layout.lineCount][lineLen] = '\0';
    layout.lineCount++;
  }

  if (layout.lineCount == 0) {
    layout.lines[0][0] = ' ';
    layout.lines[0][1] = '\0';
    layout.lineCount = 1;
  }

  const int32_t totalContentHeight = static_cast<int32_t>(layout.lineCount) * LINE_HEIGHT;
  const int32_t maxScroll = totalContentHeight - MESSAGE_SCROLL_VIEW_HEIGHT;
  layout.maxScrollOffset = (maxScroll > 0) ? static_cast<int16_t>(maxScroll) : 0;
  return layout;
}

int16_t TextLayoutService::clampScrollOffset(const int16_t scrollOffset,
                                             const int16_t maxScrollOffset) const {
  if (scrollOffset < 0) {
    return 0;
  }
  if (scrollOffset > maxScrollOffset) {
    return maxScrollOffset;
  }
  return scrollOffset;
}
