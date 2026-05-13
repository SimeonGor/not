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
}  // namespace

// Перенос: каждая строка до LINE_CHAR_CAPACITY символов. Пробел при col>0 завершает строку
// (сам пробел не пишем — skipSpaces на следующей итерации). Длинное слово без пробелов
// режется подряд на несколько строк без дублирования.

TextLayout TextLayoutService::wrapProgmemText(const char *textFromProgmem) {
  TextLayout layout = {};
  uint16_t pos = 0;

  auto readChar = [textFromProgmem](const uint16_t i) {
    return static_cast<char>(pgm_read_byte(textFromProgmem + i));
  };
  auto atEnd = [textFromProgmem](const uint16_t i) {
    return pgm_read_byte(textFromProgmem + i) == '\0';
  };

  while (layout.lineCount < MAX_WRAPPED_LINES) {
    skipSpacesPGM(textFromProgmem, pos);
    if (atEnd(pos)) {
      break;
    }

    uint8_t col = 0;
    while (col < LINE_CHAR_CAPACITY && !atEnd(pos)) {
      const char c = readChar(pos);
      if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        if (col == 0) {
          pos++;
          continue;
        }
        break;
      }
      layout.lines[layout.lineCount][col++] = c;
      pos++;
    }

    if (col == 0) {
      break;
    }

    layout.lines[layout.lineCount][col] = '\0';
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

TextLayout TextLayoutService::wrapRamText(const char *textFromRam) {
  TextLayout layout = {};
  if (textFromRam == nullptr || textFromRam[0] == '\0') {
    layout.lines[0][0] = ' ';
    layout.lines[0][1] = '\0';
    layout.lineCount = 1;
    layout.maxScrollOffset = 0;
    return layout;
  }

  uint16_t pos = 0;

  auto readChar = [textFromRam](const uint16_t i) { return textFromRam[i]; };
  auto atEnd = [textFromRam](const uint16_t i) { return textFromRam[i] == '\0'; };

  while (layout.lineCount < MAX_WRAPPED_LINES) {
    skipSpacesRam(textFromRam, pos);
    if (atEnd(pos)) {
      break;
    }

    uint8_t col = 0;
    while (col < LINE_CHAR_CAPACITY && !atEnd(pos)) {
      const char c = readChar(pos);
      if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        if (col == 0) {
          pos++;
          continue;
        }
        break;
      }
      layout.lines[layout.lineCount][col++] = c;
      pos++;
    }

    if (col == 0) {
      break;
    }

    layout.lines[layout.lineCount][col] = '\0';
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
