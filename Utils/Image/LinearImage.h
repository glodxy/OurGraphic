//
// Created by Glodxy on 2022/2/10.
//

#ifndef OUR_GRAPHIC_UTILS_IMAGE_LINEARIMAGE_H_
#define OUR_GRAPHIC_UTILS_IMAGE_LINEARIMAGE_H_
#include <cstdint>
#include <memory>

namespace our_graph::image {
/**
 * 该类用于做实际文件与内部图片数据之间的中转
 * @note:目前仅用于导出图片
 * */
class LinearImage {
 public:
  LinearImage(uint32_t w, uint32_t h, uint32_t channel);

  // 此处进行浅拷贝
  LinearImage(const LinearImage& rhs);
  LinearImage& operator=(const LinearImage& rhs);

  LinearImage() : data_(nullptr), w_(0), h_(0), channel_(0), ppr_(0) {}

  operator bool() const {
    return data_ != nullptr;
  }

  float* GetData() {return data_;}
  template<class T> T* Get() {return reinterpret_cast<T*>(data_);}
  const float* GetData() const {return data_;}
  template<class T> const T* Get() const {return reinterpret_cast<T*>(data_);}

  float* GetPixel(uint32_t row, uint32_t col) {
    return data_ + (col + row * ppr_) * channel_;
  }
  template<class T> T* Get(uint32_t row, uint32_t col) {
    return reinterpret_cast<T*>(data_ + (col + row * ppr_) * channel_);
  }
  const float* GetPixel(uint32_t row, uint32_t col) const {
    //! 为了sub image的准确，此处需要用bpr来作为每行的数据量
    return data_ + (col + row * ppr_) * channel_;
  }
  template<class T> const T* Get(uint32_t row, uint32_t col) const {
    return reinterpret_cast<T*>(data_ + (col + row * ppr_) * channel_);
  }

  uint32_t GetWidth() const {return w_;}
  uint32_t GetHeight() const {return h_;}
  uint32_t GetChannels() const {return channel_;}
  void Reset() {*this = LinearImage();}

  // 获取每行的字节数
  size_t GetBytesPerRow() const { return GetBytesPerPixel() * w_;}
  // 获取每个像素的字节数
  size_t GetBytesPerPixel() const {return channel_ * sizeof(float);}

  //! 获取每行的绝对字节数，用于定位
  size_t GetAbsoluteBytesPerRow() const {return ppr_ * GetBytesPerPixel();}

  // 获取该图片的子集
  //! 但使用同一份图片数据
  //! @note:注意此处的参数为x，y，而非row col
  void Subset(const LinearImage& image, size_t x, size_t y, size_t w, size_t h);

  ~LinearImage();

 private:

  // 因为使用浅拷贝，所以使用shared ptr来确保数据不会销毁
  struct SharedReference{
    SharedReference(uint32_t width, uint32_t height, uint32_t channel);

    std::shared_ptr<float> pixels;
  };
  // 该指针存储了对数据的引用
  SharedReference* data_ref_ = nullptr;
  // 该指针指向真实的数据
  float* data_;
  uint32_t w_;
  uint32_t h_;
  uint32_t channel_;

  // 对于实际的图片每行的数据像素个数，用于做定位
  size_t ppr_;
};
}  // namespace our_graph::image
#endif //OUR_GRAPHIC_UTILS_IMAGE_LINEARIMAGE_H_
