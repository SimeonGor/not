#include "services/text_layout_service.h"

#include <pgmspace.h>

namespace {

bool isSpaceChar(const char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void skipSpacesPGM(const char *text, uint16_t &index) {
  while (pgm_read_byte(text + index) != '\0') {
    const char ch = static_cast<char>(pgm_read_byte(text + index));
    if (!isSpaceChar(ch)) {
      break;
    }
    index++;
  }
}

void skipSpacesRam(const char *text, uint16_t &index) {
  while (text[index] != '\0') {
    const char ch = text[index];
    if (!isSpaceChar(ch)) {
      break;
    }
    index++;
  }
}

void finalizeLine(TextLayout &layout, const uint8_t lineLen) {
  layout.lines[layout.lineCount][lineLen] = '\0';
  layout.lineCount++;
}

void applyScrollMetrics(TextLayout &layout) {
  const int32_t totalContentHeight =
      static_cast<int32_t>(layout.lineCount) * static_cast<int32_t>(LINE_HEIGHT);
  const int32_t maxScrollPx = totalContentHeight - static_cast<int32_t>(MESSAGE_SCROLL_VIEW_HEIGHT);
  if (maxScrollPx <= 0) {
    layout.maxScrollLines = 0;
    return;
  }
  layout.maxScrollLines = static_cast<int16_t>(
      (maxScrollPx + static_cast<int32_t>(LINE_HEIGHT) - 1) / static_cast<int32_t>(LINE_HEIGHT));
}

}  // namespace

// Перенос по словам: несколько слов на строке, если помещаются. Слишком длинное слово
// режется на части по LINE_CHAR_CAPACITY без отката индекса (без дублирования).

TextLayout TextLayoutService::wrapProgmemText(const char *textFromProgmem) {
  TextLayout layout = {};
  uint16_t pos = 0;
  uint8_t lineLen = 0;

  auto charAt = [textFromProgmem](const uint16_t i) {
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

    const uint16_t wordStart = pos;
    while (!atEnd(pos) && !isSpaceChar(charAt(pos))) {
      pos++;
    }
    uint16_t wordLen = pos - wordStart;

    uint16_t wOff = wordStart;
    while (wordLen > LINE_CHAR_CAPACITY) {
      if (lineLen > 0) {
        finalizeLine(layout, lineLen);
        lineLen = 0;
        if (layout.lineCount >= MAX_WRAPPED_LINES) {
          applyScrollMetrics(layout);
          return layout;
        }
      }
      for (uint8_t i = 0; i < LINE_CHAR_CAPACITY; i++) {
        layout.lines[layout.lineCount][i] = charAt(static_cast<uint16_t>(wOff + i));
      }
      finalizeLine(layout, LINE_CHAR_CAPACITY);
      wOff += LINE_CHAR_CAPACITY;
      wordLen -= LINE_CHAR_CAPACITY;
      if (layout.lineCount >= MAX_WRAPPED_LINES) {
        applyScrollMetrics(layout);
        return layout;
      }
    }

    if (wordLen == 0) {
      continue;
    }

    const uint8_t tailLen = static_cast<uint8_t>(wordLen);
    const bool needSpace = (lineLen > 0);
    const uint8_t total =
        static_cast<uint8_t>(lineLen + (needSpace ? 1u : 0u) + static_cast<uint8_t>(tailLen));

    if (total <= LINE_CHAR_CAPACITY) {
      uint8_t col = lineLen;
      if (needSpace) {
        layout.lines[layout.lineCount][col++] = ' ';
      }
      for (uint8_t i = 0; i < tailLen; i++) {
        layout.lines[layout.lineCount][col++] = charAt(static_cast<uint16_t>(wOff + i));
      }
      lineLen = col;
    } else {
      if (lineLen > 0) {
        finalizeLine(layout, lineLen);
        lineLen = 0;
        if (layout.lineCount >= MAX_WRAPPED_LINES) {
          applyScrollMetrics(layout);
          return layout;
        }
      }
      for (uint8_t i = 0; i < tailLen; i++) {
        layout.lines[layout.lineCount][i] = charAt(static_cast<uint16_t>(wOff + i));
      }
      lineLen = tailLen;
    }
  }

  if (lineLen > 0 && layout.lineCount < MAX_WRAPPED_LINES) {
    finalizeLine(layout, lineLen);
  }

  if (layout.lineCount == 0) {
    layout.lines[0][0] = ' ';
    layout.lines[0][1] = '\0';
    layout.lineCount = 1;
  }

  applyScrollMetrics(layout);
  return layout;
}

TextLayout TextLayoutService::wrapRamText(const char *textFromRam) {
  TextLayout layout = {};
  if (textFromRam == nullptr || textFromRam[0] == '\0') {
    layout.lines[0][0] = ' ';
    layout.lines[0][1] = '\0';
    layout.lineCount = 1;
    layout.maxScrollLines = 0;
    return layout;
  }

  uint16_t pos = 0;
  uint8_t lineLen = 0;

  while (layout.lineCount < MAX_WRAPPED_LINES) {
    skipSpacesRam(textFromRam, pos);
    if (textFromRam[pos] == '\0') {
      break;
    }

    const uint16_t wordStart = pos;
    while (textFromRam[pos] != '\0' && !isSpaceChar(textFromRam[pos])) {
      pos++;
    }
    uint16_t wordLen = pos - wordStart;

    uint16_t wOff = wordStart;
    while (wordLen > LINE_CHAR_CAPACITY) {
      if (lineLen > 0) {
        finalizeLine(layout, lineLen);
        lineLen = 0;
        if (layout.lineCount >= MAX_WRAPPED_LINES) {
          applyScrollMetrics(layout);
          return layout;
        }
      }
      for (uint8_t i = 0; i < LINE_CHAR_CAPACITY; i++) {
        layout.lines[layout.lineCount][i] = textFromRam[wOff + i];
      }
      finalizeLine(layout, LINE_CHAR_CAPACITY);
      wOff += LINE_CHAR_CAPACITY;
      wordLen -= LINE_CHAR_CAPACITY;
      if (layout.lineCount >= MAX_WRAPPED_LINES) {
        applyScrollMetrics(layout);
        return layout;
      }
    }

    if (wordLen == 0) {
      continue;
    }

    const uint8_t tailLen = static_cast<uint8_t>(wordLen);
    const bool needSpace = (lineLen > 0);
    const uint8_t total =
        static_cast<uint8_t>(lineLen + (needSpace ? 1u : 0u) + static_cast<uint8_t>(tailLen));

    if (total <= LINE_CHAR_CAPACITY) {
      uint8_t col = lineLen;
      if (needSpace) {
        layout.lines[layout.lineCount][col++] = ' ';
      }
      for (uint8_t i = 0; i < tailLen; i++) {
        layout.lines[layout.lineCount][col++] = textFromRam[wOff + i];
      }
      lineLen = col;
    } else {
      if (lineLen > 0) {
        finalizeLine(layout, lineLen);
        lineLen = 0;
        if (layout.lineCount >= MAX_WRAPPED_LINES) {
          applyScrollMetrics(layout);
          return layout;
        }
      }
      for (uint8_t i = 0; i < tailLen; i++) {
        layout.lines[layout.lineCount][i] = textFromRam[wOff + i];
      }
      lineLen = tailLen;
    }
  }

  if (lineLen > 0 && layout.lineCount < MAX_WRAPPED_LINES) {
    finalizeLine(layout, lineLen);
  }

  if (layout.lineCount == 0) {
    layout.lines[0][0] = ' ';
    layout.lines[0][1] = '\0';
    layout.lineCount = 1;
  }

  applyScrollMetrics(layout);
  return layout;
}

int16_t TextLayoutService::clampScrollLine(const int16_t scrollLine,
                                           const int16_t maxScrollLines) const {
  if (scrollLine < 0) {
    return 0;
  }
  if (scrollLine > maxScrollLines) {
    return maxScrollLines;
  }
  return scrollLine;
}
