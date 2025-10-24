#ifndef HELPERS_H
#define HELPERS_H

#include <cstdint> 

#define CONCAT_IMPL(x,y) x##y
#define CONCAT(x,y)      CONCAT_IMPL(x,y)

constexpr uint32_t fnv1a32(const char* s, uint32_t h = 2166136261u) {
	return *s ? fnv1a32(s + 1, (h ^ uint32_t(*s)) * 16777619u) : h;
}
constexpr uint64_t fileline_id(const char* f, int l) {
	return (uint64_t(fnv1a32(f)) << 32) | uint32_t(l);
}
#define FILELINE_ID  (fileline_id(__FILE__, __LINE__))

#endif // HELPERS_H