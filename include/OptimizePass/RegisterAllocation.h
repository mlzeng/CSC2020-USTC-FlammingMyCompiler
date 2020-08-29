#ifndef REGISTER_ALLOCATION_H
#define REGISTER_ALLOCATION_H
#include "Module.h"
#include "Value.h"
#include <map>

extern std::map<Value *, int> *RegisterAllocation(Module *m);
// extern std::unordered_map<Value*, int> colorTable;

#endif