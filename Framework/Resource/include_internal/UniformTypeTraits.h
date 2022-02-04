//
// Created by Glodxy on 2022/2/4.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_INTERNAL_UNIFORMTYPETRAITS_H_
#define OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_INTERNAL_UNIFORMTYPETRAITS_H_
#include "Backend/include/DriverEnum.h"
#include "Utils/Math/Math.h"
#include "Utils/OGLogging.h"
#include <string>
#include <cstdint>
#include <vector>
namespace our_graph {

// 分割字符串
std::vector<std::string> PartitionString(const std::string& value) {
  std::vector<std::string> res;
  if (*value.begin() != '(' || *value.rbegin() != ')') {
    LOG_ERROR("PartitionString", "format error! ()error!");
    return res;
  }
  std::string v = value.substr(1, value.size() - 2);

  size_t end = 0;
  uint32_t begin = 0;
  while((end = v.find(',', begin)) != std::string::npos) {
    std::string sub_str = v.substr(begin, end - begin);
    res.push_back(sub_str);
    begin = end + 1;
  }
  if (end != begin) {
    std::string sub_str = v.substr(begin);
    res.push_back(sub_str);
  }
  return res;
}

template<class T>
void GetArrayFromString(const std::string& value, T* out, size_t size);

template<>
void GetArrayFromString<float>(const std::string& value, float* out, size_t size) {
  const auto&str_list = PartitionString(value);
  for (size_t i = 0; i < size; ++i) {
    float v = std::stof(str_list[i]);
    out[i] = v;
  }
}

template<>
void GetArrayFromString<int>(const std::string& value, int* out, size_t size) {
  const auto&str_list = PartitionString(value);
  for (size_t i = 0; i < size; ++i) {
    int v = std::stoi(str_list[i]);
    out[i] = v;
  }
}

template<>
void GetArrayFromString<unsigned int >(const std::string& value, unsigned int* out, size_t size) {
  const auto&str_list = PartitionString(value);
  for (size_t i = 0; i < size; ++i) {
    unsigned int v = std::stoul(str_list[i]);
    out[i] = v;
  }
}

template<>
void GetArrayFromString<bool>(const std::string& value, bool* out, size_t size) {
  const auto&str_list = PartitionString(value);
  for (size_t i = 0; i < size; ++i) {
    bool v = str_list[i].find("true") != std::string::npos;
    out[i] = v;
  }
}

#define DECLARE_UNIFORM_DATA_TYPE(uniform_type, data_type)  \
  template<>                                                 \
  struct UniformDataType<UniformType::uniform_type> {        \
    using Type = data_type;                                                           \
  };\

template<UniformType uniform>
struct UniformDataType {
  using Type = math::Vec3;
};

DECLARE_UNIFORM_DATA_TYPE(FLOAT, float);
DECLARE_UNIFORM_DATA_TYPE(FLOAT2, math::Vec2);
DECLARE_UNIFORM_DATA_TYPE(FLOAT3, math::Vec3);
DECLARE_UNIFORM_DATA_TYPE(FLOAT4, math::Vec4);

DECLARE_UNIFORM_DATA_TYPE(UINT, unsigned int);
DECLARE_UNIFORM_DATA_TYPE(UINT2, math::Vec2u);
DECLARE_UNIFORM_DATA_TYPE(UINT3, math::Vec3u);
DECLARE_UNIFORM_DATA_TYPE(UINT4, math::Vec4u);

DECLARE_UNIFORM_DATA_TYPE(INT, int);
DECLARE_UNIFORM_DATA_TYPE(INT2, math::Vec2i);
DECLARE_UNIFORM_DATA_TYPE(INT3, math::Vec3i);
DECLARE_UNIFORM_DATA_TYPE(INT4, math::Vec4i);

DECLARE_UNIFORM_DATA_TYPE(BOOL, bool);
DECLARE_UNIFORM_DATA_TYPE(BOOL2, math::Vec2b);
DECLARE_UNIFORM_DATA_TYPE(BOOL3, math::Vec3b);
DECLARE_UNIFORM_DATA_TYPE(BOOL4, math::Vec4b);






template<UniformType uniform>
typename UniformDataType<uniform>::Type ParseFromString(const std::string& value);

template<>
typename UniformDataType<UniformType::FLOAT>::Type ParseFromString<UniformType::FLOAT>(const std::string& value) {
  return std::stof(value);
}
template<>
typename UniformDataType<UniformType::FLOAT2>::Type ParseFromString<UniformType::FLOAT2>(const std::string& value) {
  float v[2];
  GetArrayFromString(value, v, 2);
  return {v[0],v[1]};
}
template<>
typename UniformDataType<UniformType::FLOAT3>::Type ParseFromString<UniformType::FLOAT3>(const std::string& value) {
  float v[3];
  GetArrayFromString(value, v, 3);
  return {v[0], v[1], v[2]};
}
template<>
typename UniformDataType<UniformType::FLOAT4>::Type ParseFromString<UniformType::FLOAT4>(const std::string& value) {
  float v[4];
  GetArrayFromString(value, v, 4);
  return {v[0], v[1], v[2], v[3]};
}

template<>
typename UniformDataType<UniformType::INT>::Type ParseFromString<UniformType::INT>(const std::string& value) {
  return std::stoi(value);
}
template<>
typename UniformDataType<UniformType::INT2>::Type ParseFromString<UniformType::INT2>(const std::string& value) {
  int v[2];
  GetArrayFromString(value, v, 2);
  return {v[0],v[1]};
}
template<>
typename UniformDataType<UniformType::INT3>::Type ParseFromString<UniformType::INT3>(const std::string& value) {
  int v[3];
  GetArrayFromString(value, v, 3);
  return {v[0], v[1], v[2]};
}
template<>
typename UniformDataType<UniformType::INT4>::Type ParseFromString<UniformType::INT4>(const std::string& value) {
  int v[4];
  GetArrayFromString(value, v, 4);
  return {v[0], v[1], v[2], v[3]};
}

template<>
typename UniformDataType<UniformType::UINT>::Type ParseFromString<UniformType::UINT>(const std::string& value) {
  return std::stoul(value);
}
template<>
typename UniformDataType<UniformType::UINT2>::Type ParseFromString<UniformType::UINT2>(const std::string& value) {
  unsigned int v[2];
  GetArrayFromString(value, v, 2);
  return {v[0],v[1]};
}
template<>
typename UniformDataType<UniformType::UINT3>::Type ParseFromString<UniformType::UINT3>(const std::string& value) {
  unsigned int v[3];
  GetArrayFromString(value, v, 3);
  return {v[0], v[1], v[2]};
}
template<>
typename UniformDataType<UniformType::UINT4>::Type ParseFromString<UniformType::UINT4>(const std::string& value) {
  unsigned int v[4];
  GetArrayFromString(value, v, 4);
  return {v[0], v[1], v[2], v[3]};
}

template<>
typename UniformDataType<UniformType::BOOL>::Type ParseFromString<UniformType::BOOL>(const std::string& value) {
  return value.find("true") != std::string::npos;
}
template<>
typename UniformDataType<UniformType::BOOL2>::Type ParseFromString<UniformType::BOOL2>(const std::string& value) {
  bool v[2];
  GetArrayFromString(value, v, 2);
  return {uint32_t(v[0]),uint32_t(v[1])};
}
template<>
typename UniformDataType<UniformType::BOOL3>::Type ParseFromString<UniformType::BOOL3>(const std::string& value) {
  bool v[3];
  GetArrayFromString(value, v, 3);
  return {(uint32_t)v[0], (uint32_t)v[1], (uint32_t)v[2]};
}
template<>
typename UniformDataType<UniformType::BOOL4>::Type ParseFromString<UniformType::BOOL4>(const std::string& value) {
  bool v[4];
  GetArrayFromString(value, v, 4);
  return {(uint32_t)v[0], (uint32_t)v[1], (uint32_t)v[2], (uint32_t)v[3]};
}
}  // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_RESOURCE_INCLUDE_INTERNAL_UNIFORMTYPETRAITS_H_
