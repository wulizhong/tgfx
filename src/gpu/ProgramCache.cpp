/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Tencent is pleased to support the open source community by making tgfx available.
//
//  Copyright (C) 2023 THL A29 Limited, a Tencent company. All rights reserved.
//
//  Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
//  in compliance with the License. You may obtain a copy of the License at
//
//      https://opensource.org/licenses/BSD-3-Clause
//
//  unless required by applicable law or agreed to in writing, software distributed under the
//  license is distributed on an "as is" basis, without warranties or conditions of any kind,
//  either express or implied. see the license for the specific language governing permissions
//  and limitations under the license.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "ProgramCache.h"

namespace tgfx {
#define MAX_PROGRAM_COUNT 128

ProgramCache::ProgramCache(Context* context) : context(context) {
}

bool ProgramCache::empty() const {
  return programMap.empty();
}

Program* ProgramCache::getProgram(const ProgramInfo* programInfo) {
  BytesKey uniqueKey = {};
  programInfo->computeUniqueKey(context, &uniqueKey);
  auto result = programMap.find(uniqueKey);
  if (result != programMap.end()) {
    programLRU.remove(result->second);
    programLRU.push_front(result->second);
    return result->second;
  }
  auto program = programInfo->createProgram(context).release();
  if (program == nullptr) {
    return nullptr;
  }
  program->uniqueKey = uniqueKey;
  programLRU.push_front(program);
  programMap[uniqueKey] = program;
  while (programLRU.size() > MAX_PROGRAM_COUNT) {
    removeOldestProgram();
  }
  return program;
}

void ProgramCache::removeOldestProgram(bool releaseGPU) {
  auto program = programLRU.back();
  programLRU.pop_back();
  programMap.erase(program->uniqueKey);
  if (releaseGPU) {
    program->onReleaseGPU();
  }
  delete program;
}

void ProgramCache::releaseAll(bool releaseGPU) {
  while (!programLRU.empty()) {
    removeOldestProgram(releaseGPU);
  }
}
}  // namespace tgfx
