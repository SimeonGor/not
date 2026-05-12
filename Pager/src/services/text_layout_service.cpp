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
}  // namespace

TextLayout TextLayoutService::wrapProgmemText(const char *textFromProgmem) {
  TextLayout layout = {};
  uint16_t index = 0;

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
          if (lineLen == 0 || lineLen + wordLen <= LINE_CHAR_CAPACITY) {
            for (uint8_t i = 0; i < wordLen; i++) {
              layout.lines[layout.lineCount][lineLen++] =
                  static_cast<char>(pgm_read_byte(textFromProgmem + wordStart + i));
            }
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
          layout.lines[layout.lineCount][i] =
              static_cast<char>(pgm_read_byte(textFromProgmem + index - wordLen + i));
        }
        lineLen = LINE_CHAR_CAPACITY;
        index = wordStart + wordLen;
        break;
      }
    }

    if (lineLen == 0 && wordLen > 0) {
      const uint8_t copyLen = min(wordLen, LINE_CHAR_CAPACITY);
      for (uint8_t i = 0; i < copyLen; i++) {
        layout.lines[layout.lineCount][i] =
            static_cast<char>(pgm_read_byte(textFromProgmem + wordStart + i));
      }
      lineLen = copyLen;
      index = wordStart + wordLen;
    }

    if (lineLen == 0) {
      break;
    }

    layout.lines[layout.lineCount][lineLen] = '\0';
    layout.lineCount++;
  }

  const int32_t totalContentHeight = static_cast<int32_t>(layout.lineCount) * LINE_HEIGHT;
  const int32_t maxScroll = totalContentHeight - CONTENT_VIEW_HEIGHT;
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
