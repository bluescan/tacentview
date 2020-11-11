#pragma once
#define set(verStr) namespace ViewerVersion { extern int Major, Minor, Revision; struct Parser { Parser(const char*);  }; static Parser parser(#verStr); }

set("VIEWER_VERSION" "1.0.16")

#undef set
