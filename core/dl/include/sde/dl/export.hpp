#pragma once

#define SDE_EXPORT extern "C"

#define SDE_EXPORT_OBJECT_CREATOR(ObjectT) create

#define SDE_EXPORT_OBJECT_CREATOR_FN(ObjectT) SDE_EXPORT void* SDE_EXPORT_OBJECT_CREATOR(ObjectT)

#define SDE_EXPORT_OBJECT_CREATOR_DECL(ObjectT) SDE_EXPORT_OBJECT_CREATOR_FN(ObjectT)();

#define SDE_EXPORT_OBJECT_CREATOR_IMPL(ObjectT)                                                                        \
  SDE_EXPORT_OBJECT_CREATOR_FN(ObjectT)() { return reinterpret_cast<void*>(new ObjectT()); }
