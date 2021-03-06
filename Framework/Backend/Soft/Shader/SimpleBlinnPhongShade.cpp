//
// Created by Glodxy on 2021/11/15.
//

#include "SimpleBlinnPhongShade.h"
#include "Backend/Soft/Utils/SoftTransform.h"
#include "Backend/Soft/SoftPipeline.h"
namespace our_graph {

void SimpleBlinnPhongShade::VertexShade(const Vertex *in, Vertex *out) {
  Vec4 src_ext(in->position, 1.0f);
  float angle = *(int*)context_ * 5;
  Mat4 model = SoftTransform::RotateY(angle) * SoftTransform::Translation(Vec3(-.5, -.5, .5));
  Mat4 view = SoftTransform::View({0, 0, 10}, {0,0,-1}, {0, 1, 0});
  Frustum frustum {
      .fov = 120,
      .aspect = 800.f/600.f,
      .n = -.1f,
      .f = -100.f
  };
  Mat4 pers = SoftTransform::Perspective(frustum);
  (out->data).world_position = model * src_ext;
  Vec4 world_normal = model * Vec4 (in->data.world_normal, 1.0f);
  world_normal /= world_normal.w;
  out->data.world_normal = SoftTransform::Extract<3>(world_normal);
  out->data.world_normal = glm::normalize(out->data.world_normal);

  out->clip_position = pers * view * model * src_ext;
}

void SimpleBlinnPhongShade::PixelShade(const Pixel *in, Pixel *out) {
  Vec3 material(1.0, 0.0, .0f);
  Vec3 normal = glm::normalize(in->data.world_normal);
  Vec3 light_pos(0, 0, 1);
  Vec3 view_pos(0, 0, 1);
  const CustomData* data = &in->data;
  Vec3 light_dir = data->world_position - light_pos;
  Vec3 view_dir = glm::normalize(data->world_position - view_pos);
  Vec3 light_color(1, 1, 1);
  float power = 2.f / glm::pow(glm::length(light_dir), 2);
  light_dir = glm::normalize(light_dir);
  Vec3 ambient = Vec3(0.2, 0.2, 0.2);
  Vec3 diffuse = Vec3(1,1,1) * power * std::max(0.f, glm::dot(-light_dir, normal));
  float specular_recv = glm::pow(std::max(0.f, glm::dot((-(glm::normalize(light_dir + view_dir))), normal)), 256);
  Vec3 specular = light_color * power * specular_recv;

  Vec3 color = material * (ambient + diffuse);
  color = glm::clamp(color, Vec3(0, 0, 0), Vec3(1, 1, 1));
//  color.g = glm::clamp(color.g, 0, 1);
//  color.b = glm::clamp(color.b, 0, 1);
  out->color.r = color.r * 255;
  out->color.g = color.y * 255;
  out->color.b = color.z * 255;
  out->color.a = 255;
}

}