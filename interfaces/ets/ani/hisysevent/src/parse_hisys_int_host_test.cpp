#include "parse_hisys_int.h"
#include <cstdint>
#include <cassert>
#include <iostream>
int main() {
    int64_t v = 7;
    assert(!ParseHisysInt64("", v) && v == 7);
    assert(!ParseHisysInt64("abc", v));
    assert(!ParseHisysInt64("9999999999999999999", v));
    assert(ParseHisysInt64("0", v) && v == 0);
    assert(ParseHisysInt64("42", v) && v == 42);
    std::cout << "ok" << std::endl;
    return 0;
}
