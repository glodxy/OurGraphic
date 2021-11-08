//
// Created by chaytian on 2021/11/2.
//

#include "SoftPipeline.h"
#include "SoftContext.h"
#include "Utils/SoftTransform.h"
#include "SDL2/SDL.h"

namespace our_graph {
static uint32_t GetColor(const Color& color) {
  return (uint32_t(color.r) << 24) |
      (uint32_t(color.g) << 16) |
      (uint32_t(color.b) << 8) |
      (uint32_t(color.a));
}

void BresenhamLine(int x0, int y0, int x1, int y1, std::function<void(int,int)> set_pixel) {
  bool swap=false;//代表是否交换x和y
  //选择变化较大的一向作为遍历向
  if(std::abs(x0-x1)<std::abs(y0-y1)){
    std::swap(x0,y0);
    std::swap(x1,y1);
    swap=true;
  }
  //选择较小的为遍历起点
  if(x0>x1){
    std::swap(x0,x1);
    std::swap(y0,y1);
  }

  int dx=x1-x0;
  int dy=y1-y0;
  int i=dy>0?1:-1;//根据dy判断该直线是上升还是下降
  int step=std::abs(dy)*2;//每次check所添加的数值
  int check=0;//判断是否上升
  int y=y0;

  //开始遍历
  for(int x=x0;x<=x1;++x){
    if(swap){
      set_pixel(y, x);
    }
    else{
      set_pixel(x, y);
    }

    check+=step;
    if(check>dx){
      y+=i;
      check-=2*dx;
    }
  }
}

static Vertex vertices[3] = {
    {1, 0, 0, 1},
    {0 ,1,0, 1},
    {0, 0, 1, 1}
};

/**
 * 单顶点的着色
 * todo：顶点变换
 * */
void SoftPipeline::SingleVertexShade(const Vertex&src, Vertex &dst) {
  Vec4 tmp = SoftTransform::View({-1, -1, -1}, {1,1,1}, {0, 1, 0})*src;
  Frustum frustum {
    .fov = 120,
    .aspect = 800.f/600.f,
    .n = -.1f,
    .f = -100.f
  };
  Mat4 pers = SoftTransform::Perspective(frustum);
  dst = pers * tmp;
}

/**
 * 单个三角形的光栅化
 * todo：视口变换以及采样
 * */
void SoftPipeline::RasterizerSingleTriangle(const Triangle &src, std::vector<Pixel> &pixels) {
  uint32_t width = SoftContext::Get().window_width_;
  uint32_t height = SoftContext::Get().window_height_;
  Vec2 p1 = SoftTransform::Extract<2>(*src.a);
  Vec2 p2 = SoftTransform::Extract<2>(*src.b);
  Vec2 p3 = SoftTransform::Extract<2>(*src.c);

  Rect2D<int> bbox = SoftTransform::GetTriangleBBox(p1, p2, p3);
  // 遍历包围盒
  for (int i = bbox.b; i<= bbox.t; ++i) {
    if (i < 0 || i >= height) continue;
    for (int j = bbox.l; j < bbox.r; ++j) {
      if (j < 0 || j >= width) continue;
      Vec3 barycentric = SoftTransform::Barycentric({j, i}, p1, p2, p3);
      if (barycentric.x < 0 || barycentric.y < 0 || barycentric.z < 0 ||
          barycentric.x > 1.f || barycentric.y > 1.f || barycentric.z > 1.f) {
        continue;
      }
      pixels.push_back(Pixel(j, i));
    }
  }
}

void SoftPipeline::VertexShade(const Vertex *vertex, size_t size, Vertex *&dst_vertex, size_t &dst_size) {
  dst_size = size;
  dst_vertex = new Vertex[dst_size];

  for (int i = 0; i < size; ++i) {
    SingleVertexShade(vertex[i], dst_vertex[i]);
    // 视口变换
    dst_vertex[i].x /= dst_vertex[i].w;
    dst_vertex[i].y /= dst_vertex[i].w;
    dst_vertex[i].z /= dst_vertex[i].w;
    dst_vertex[i] = SoftTransform::Scale({400,300,1})*dst_vertex[i];
    dst_vertex[i].x += 400;
    dst_vertex[i].y += 300;
  }

}

bool SoftPipeline::GeometryTriangle(const Vertex *vertex, size_t size, Triangle *&triangle, size_t &triangle_size) {
  if (size % 3 != 0) {
    LOG_ERROR("SoftPipeline", "vertices size error:{}", size);
    return false;
  }
  triangle_size = size / 3;
  triangle = new Triangle[triangle_size];
  for (int i = 0; i < triangle_size; ++i) {
    triangle[i].a = vertex + (i * 3);
    triangle[i].b = vertex + (i*3 + 1);
    triangle[i].c = vertex + (i*3 + 2);
  }
  return true;
}

void SoftPipeline::Rasterize(const Triangle *triangles, size_t size, Pixel *&pixel, size_t &pixel_size) {
  std::vector<Pixel> pixels;
  // 遍历所有三角形
  for (int i = 0; i < size; ++i) {
    RasterizerSingleTriangle(triangles[i], pixels);
  }
  pixel = new Pixel[pixels.size()];
  memcpy(pixel, pixels.data(), sizeof(Pixel) * pixels.size());
  pixel_size = pixels.size();
}

void WireFrame(const Triangle *triangles, size_t size, Pixel *&pixel, size_t &pixel_size) {
  std::vector<Pixel> pixels;
  auto set_pixel_func = [&pixels](int x, int y) {
    if (x < 0 || y < 0 || x >=800 || y >=600) {
      return;
    }
    Pixel p;
    p.x = x;
    p.y = y;
    p.color.r = 255;
    pixels.push_back(p);
  };
  LOG_INFO("DrawTriangle", "{}-{};{}-{};{}-{}",
           triangles->a->x, triangles->a->y,
           triangles->b->x, triangles->b->y,
           triangles->c->x, triangles->c->y);
  // 遍历所有三角形
  for (int i = 0; i < size; ++i) {
    BresenhamLine((int)triangles[i].a->x, (int)triangles[i].a->y,
                  (int)triangles[i].b->x, (int)triangles[i].b->y, set_pixel_func);
    BresenhamLine((int)triangles[i].b->x, (int)triangles[i].b->y,
                  (int)triangles[i].c->x, (int)triangles[i].c->y, set_pixel_func);
    BresenhamLine((int)triangles[i].c->x, (int)triangles[i].c->y,
                  (int)triangles[i].a->x, (int)triangles[i].a->y, set_pixel_func);
  }
  pixel = new Pixel[pixels.size()];
  memcpy(pixel, pixels.data(), sizeof(Pixel) * pixels.size());
  pixel_size = pixels.size();
}

/**
 * 目前不进行任何测试，仅拷贝像素
 * */
void SoftPipeline::Test(const Pixel *src_pixel, size_t src_size, Pixel *&dst_pixel, size_t &dst_size) {
  dst_size = src_size;
  dst_pixel = new Pixel[dst_size];
  memcpy(dst_pixel, src_pixel, sizeof(Pixel) * dst_size);
}

void SoftPipeline::PixelShade(Pixel *pixel, size_t size) {
  for (int i = 0; i < size; ++i) {
    pixel[i].color.r = 255;
  }
}

/**
 * 此处假设x为列，y为行
 * */
void SoftPipeline::PixelBlit(const Pixel *pixel, size_t size) {
  SDL_Texture* current_texture = SoftContext::Get().current_tex_;
  assert(current_texture);

  void* tex;
  int pitch;
  SDL_LockTexture(current_texture, nullptr, &tex, &pitch);
  for (int i = 0; i < size; ++i) {
    const Pixel p = pixel[i];
    uint32_t idx = p.y * (pitch/4) + p.x;
    ((uint32_t*)tex)[idx] = GetColor(p.color);
  }
  LOG_INFO("SoftPipeline", "PixelBlit {} pixels", size);
  SDL_UnlockTexture(current_texture);
}

void SoftPipeline::DestroyVertex(Vertex *&vertex, size_t size) {
  if (!vertex) {return;}
  delete [] vertex;
  vertex = nullptr;
}

void SoftPipeline::DestroyTriangle(Triangle *&triangle, size_t size) {
  if (!triangle) {return;}
  delete [] triangle;
  triangle = nullptr;
}

void SoftPipeline::DestroyPixel(Pixel *&pixel, size_t size) {
  if (!pixel) {return;}
  delete [] pixel;
  pixel = nullptr;
}

void SoftPipeline::Execute(const Vertex *vertex, size_t size) {
  Vertex* transformed_vertex = nullptr;
  size_t transformed_vertex_cnt;
  // 目前仅使用固定三角形
  VertexShade(vertices, 3, transformed_vertex, transformed_vertex_cnt);

  Triangle* triangles = nullptr;
  size_t triangle_cnt;
  if (!GeometryTriangle(transformed_vertex, transformed_vertex_cnt,
                        triangles, triangle_cnt)) {
    LOG_ERROR("Pipeline", "Execute Failed! geometry failed!");
    return;
  }

  Pixel* src_pixels;
  size_t src_pixel_cnt;
  Rasterize(triangles, triangle_cnt,
            src_pixels, src_pixel_cnt);
  //WireFrame(triangles, triangle_cnt, src_pixels, src_pixel_cnt);

  Pixel* tested_pixels;
  size_t tested_pixel_cnt;
  Test(src_pixels, src_pixel_cnt,
       tested_pixels, tested_pixel_cnt);

  // 此处只需改变颜色了，所以直接复用
  PixelShade(tested_pixels, tested_pixel_cnt);

  PixelBlit(tested_pixels, tested_pixel_cnt);

  //清除临时生成的资源
  DestroyPixel(tested_pixels, tested_pixel_cnt);
  DestroyPixel(src_pixels, src_pixel_cnt);
  DestroyTriangle(triangles, triangle_cnt);
  DestroyVertex(transformed_vertex, transformed_vertex_cnt);

}

}