//
// Created by chaytian on 2021/11/2.
//

#include "SoftPipeline.h"
#include "SoftContext.h"
#include "Resource/SoftRenderTarget.h"
#include "Utils/SoftTransform.h"
#include "SimpleSoftRasterizer.h"
#include "SimpleSoftTest.h"
#include "Shader/SimpleBlinnPhongShade.h"
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

SoftPipeline::SoftPipeline() {
  rasterizer_ = std::make_unique<SimpleSoftRasterizer>();
  tester_ = std::make_unique<SimpleSoftTest>();
  shader_ = std::make_unique<SimpleBlinnPhongShade>();
}

/**
 * 单顶点的着色
 * todo：顶点变换
 * */
void SoftPipeline::SingleVertexShade(const Vertex&src, Vec4 &dst) {
  Vec4 src_ext(src.position, 1.0f);
  float angle = frame_;
  Mat4 model = SoftTransform::RotateZ(angle);
  Mat4 view = SoftTransform::View({0, 0, 1}, {0,0,-1}, {0, 1, 0});
  Frustum frustum {
    .fov = 120,
    .aspect = 800.f/600.f,
    .n = -.1f,
    .f = -100.f
  };
  Mat4 pers = SoftTransform::Perspective(frustum);
  dst = pers * view * model * src_ext;
}


void SoftPipeline::VertexShade(const Vertex *vertex, size_t size, Vertex *&dst_vertex, size_t &dst_size) {
  dst_size = size;
  dst_vertex = new Vertex[dst_size];
  for (int i = 0; i < size; ++i) {
    shader_->VertexShade(&(vertex[i]), &(dst_vertex[i]));
    Vec4 tmp_vertex = dst_vertex[i].clip_position;
    // 视口变换
    tmp_vertex.x /= tmp_vertex.w;
    tmp_vertex.y /= tmp_vertex.w;
    tmp_vertex.z /= tmp_vertex.w;
    tmp_vertex = SoftTransform::Scale({400,300,1})*tmp_vertex;
    tmp_vertex.x += 400;
    tmp_vertex.y += 300;
    dst_vertex[i].position = SoftTransform::Extract<3>(tmp_vertex);
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
  rasterizer_->Rasterize(triangles, size, pixel, pixel_size);
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
           triangles->a->position.x, triangles->a->position.y,
           triangles->b->position.x, triangles->b->position.y,
           triangles->c->position.x, triangles->c->position.y);
  // 遍历所有三角形
  for (int i = 0; i < size; ++i) {
    BresenhamLine((int)triangles[i].a->position.x, (int)triangles[i].a->position.y,
                  (int)triangles[i].b->position.x, (int)triangles[i].b->position.y, set_pixel_func);
    BresenhamLine((int)triangles[i].b->position.x, (int)triangles[i].b->position.y,
                  (int)triangles[i].c->position.x, (int)triangles[i].c->position.y, set_pixel_func);
    BresenhamLine((int)triangles[i].c->position.x, (int)triangles[i].c->position.y,
                  (int)triangles[i].a->position.x, (int)triangles[i].a->position.y, set_pixel_func);
  }
  pixel = new Pixel[pixels.size()];
  memcpy(pixel, pixels.data(), sizeof(Pixel) * pixels.size());
  pixel_size = pixels.size();
}

/**
 * 目前不进行任何测试，仅拷贝像素
 * */
void SoftPipeline::Test(const Pixel *src_pixel, size_t src_size, Pixel *&dst_pixel, size_t &dst_size) {
  if (enable_test_) {
    tester_->Test(src_pixel, src_size, dst_pixel, dst_size);
  } else {
    dst_size = src_size;
    dst_pixel = new Pixel[dst_size];
    memcpy(dst_pixel, src_pixel, sizeof(Pixel) * dst_size);
  }
}

void SoftPipeline::PixelShade(Pixel *pixel, size_t size) {
  for (int i = 0; i < size; ++i) {
    shader_->PixelShade(&pixel[i], &pixel[i]);
  }
}

/**
 * 将所有的pixel绘制至rendertarget
 * */
void SoftPipeline::PixelBlit(const Pixel *pixel, size_t size, void* context) {
  for (int i = 0; i < size; ++i) {
    uint32_t color = GetColor(pixel[i].color);
    ((SoftRenderTarget*)context)->SetPixel(pixel[i].x, pixel[i].y, color);
  }
  LOG_INFO("SoftPipeline", "PixelBlit {} pixels", size);
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

void SoftPipeline::Execute(const Vertex *vertex, size_t size, void* context) {
  ++ frame_;
  LOG_INFO("SoftPipeline", "execute frame:{}", frame_);
  // 设置各模块的上下文
  rasterizer_->SetContext(context);
  tester_->SetContext(context);
  shader_->SetContext(&frame_);

  Vertex* transformed_vertex = nullptr;
  size_t transformed_vertex_cnt;
  // 目前仅使用固定三角形
  VertexShade(vertex, size, transformed_vertex, transformed_vertex_cnt);

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

  PixelBlit(tested_pixels, tested_pixel_cnt, context);

  //清除临时生成的资源
  DestroyPixel(tested_pixels, tested_pixel_cnt);
  DestroyPixel(src_pixels, src_pixel_cnt);
  DestroyTriangle(triangles, triangle_cnt);
  DestroyVertex(transformed_vertex, transformed_vertex_cnt);
}

}