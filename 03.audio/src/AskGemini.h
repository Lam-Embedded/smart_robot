#ifndef ASK_GEMINI_H
#define ASK_GEMINI_H

#include <Arduino.h>

extern const char* GEMINI_API_KEY;
extern const char* GEMINI_URL;
extern const int GEMINI_MAX_TOKENS;

String askGemini(const String& question);
String cleanTextForTTS(const String& input);

#endif
