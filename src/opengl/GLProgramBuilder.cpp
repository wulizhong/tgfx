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

#include "GLProgramBuilder.h"
#include "GLContext.h"
#include "GLUtil.h"

namespace tgfx {
static std::string TypeModifierString(bool isDesktopGL, ShaderVar::TypeModifier t,
                                      ShaderFlags flag) {
  switch (t) {
    case ShaderVar::TypeModifier::None:
      return "";
    case ShaderVar::TypeModifier::Attribute:
      return isDesktopGL ? "in" : "attribute";
    case ShaderVar::TypeModifier::Varying:
      return isDesktopGL ? (flag == ShaderFlags::Vertex ? "out" : "in") : "varying";
    case ShaderVar::TypeModifier::Uniform:
      return "uniform";
    case ShaderVar::TypeModifier::Out:
      return "out";
  }
}

static constexpr std::pair<SLType, const char*> SLTypes[] = {
    {SLType::Void, "void"},
    {SLType::Float, "float"},
    {SLType::Float2, "vec2"},
    {SLType::Float3, "vec3"},
    {SLType::Float4, "vec4"},
    {SLType::Float2x2, "mat2"},
    {SLType::Float3x3, "mat3"},
    {SLType::Float4x4, "mat4"},
    {SLType::Int, "int"},
    {SLType::Int2, "ivec2"},
    {SLType::Int3, "ivec3"},
    {SLType::Int4, "ivec4"},
    {SLType::Texture2DRectSampler, "sampler2DRect"},
    {SLType::TextureExternalSampler, "samplerExternalOES"},
    {SLType::Texture2DSampler, "sampler2D"},
};

static std::string SLTypeString(SLType t) {
  for (const auto& pair : SLTypes) {
    if (pair.first == t) {
      return pair.second;
    }
  }
  return "";
}

std::unique_ptr<Program> ProgramBuilder::CreateProgram(Context* context, const Pipeline* pipeline) {
  GLProgramBuilder builder(context, pipeline);
  if (!builder.emitAndInstallProcessors()) {
    return nullptr;
  }
  return builder.finalize();
}

GLProgramBuilder::GLProgramBuilder(Context* context, const Pipeline* pipeline)
    : ProgramBuilder(context, pipeline), _varyingHandler(this), _uniformHandler(this),
      _vertexBuilder(this), _fragBuilder(this) {
}

std::string GLProgramBuilder::versionDeclString() {
  return isDesktopGL() ? "#version 150\n" : "#version 100\n";
}

std::string GLProgramBuilder::textureFuncName() const {
  return isDesktopGL() ? "texture" : "texture2D";
}

std::string GLProgramBuilder::getShaderVarDeclarations(const ShaderVar& var,
                                                       ShaderFlags flag) const {
  std::string ret;
  if (var.typeModifier() != ShaderVar::TypeModifier::None) {
    ret += TypeModifierString(isDesktopGL(), var.typeModifier(), flag);
    ret += " ";
    // On Android，fragment shader's varying needs high precision.
    if (var.typeModifier() == ShaderVar::TypeModifier::Varying && flag == ShaderFlags::Fragment) {
      ret += "highp ";
    }
  }
  ret += SLTypeString(var.type());
  ret += " ";
  ret += var.name();
  return ret;
}

std::unique_ptr<GLProgram> GLProgramBuilder::finalize() {
  if (isDesktopGL()) {
    fragmentShaderBuilder()->declareCustomOutputColor();
  }
  finalizeShaders();

  auto vertex = vertexShaderBuilder()->shaderString();
  auto fragment = fragmentShaderBuilder()->shaderString();
  auto programID = CreateGLProgram(context, vertex, fragment);
  if (programID == 0) {
    return nullptr;
  }
  computeCountsAndStrides(programID);
  resolveProgramResourceLocations(programID);

  return createProgram(programID);
}

void GLProgramBuilder::computeCountsAndStrides(unsigned int programID) {
  auto gl = GLFunctions::Get(context);
  vertexStride = 0;
  for (const auto* attr : pipeline->getGeometryProcessor()->vertexAttributes()) {
    GLProgram::Attribute attribute;
    attribute.gpuType = attr->gpuType();
    attribute.offset = vertexStride;
    vertexStride += static_cast<int>(attr->sizeAlign4());
    attribute.location = gl->getAttribLocation(programID, attr->name().c_str());
    if (attribute.location >= 0) {
      attributes.push_back(attribute);
    }
  }
}

void GLProgramBuilder::resolveProgramResourceLocations(unsigned programID) {
  _uniformHandler.resolveUniformLocations(programID);
}

bool GLProgramBuilder::checkSamplerCounts() {
  auto caps = GLCaps::Get(context);
  if (numFragmentSamplers > caps->maxFragmentSamplers) {
    LOGE("Program would use too many fragment samplers.");
    return false;
  }
  return true;
}

std::unique_ptr<GLProgram> GLProgramBuilder::createProgram(unsigned programID) {
  auto uniformBuffer = _uniformHandler.makeUniformBuffer();
  auto program =
      new GLProgram(context, programID, std::move(uniformBuffer), attributes, vertexStride);
  program->setupSamplerUniforms(_uniformHandler.samplers);
  return std::unique_ptr<GLProgram>(program);
}

bool GLProgramBuilder::isDesktopGL() const {
  auto caps = GLCaps::Get(context);
  return caps->standard == GLStandard::GL;
}
}  // namespace tgfx
