#include <node.h>
#include <nan.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <vector>

using namespace boost::filesystem;
namespace newfs = boost::filesystem;
using namespace v8;
using namespace std;

vector<uchar> matToString(cv::Mat descriptorMat) {
  vector<uchar> buf;
  imencode(".png", descriptorMat, buf);
  return buf;
}

vector<uchar> descriptorMat(cv::Mat image) {
  vector<cv::KeyPoint> keyPoint;
  cv::Ptr<cv::ORB> orb = cv::ORB::create(4000, 1.2f, 8, 31, 0, 2, cv::ORB::HARRIS_SCORE, 31, 20);
  orb->detect(image, keyPoint);
  cv::Mat descriptorMat;
  orb->compute(image, keyPoint, descriptorMat);
  return matToString(descriptorMat);
}

void imageFeature(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() < 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
    return;
  }
  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }
  String::Utf8Value pathValue(Local<String>::Cast(args[0]));
  string path = string(*pathValue);
  cv::Mat image = cv::imread(path, 1);
  if (image.empty()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Input image error")));
    return;
  }
  vector<uchar> descriptorString = descriptorMat(image);
  Local<Object> buf = Nan::NewBuffer(descriptorString.size()).ToLocalChecked();
  uchar* data = (uchar*) node::Buffer::Data(buf);
  memcpy(data, &descriptorString[0], descriptorString.size());
  v8::Local<v8::Object> globalObj = Nan::GetCurrentContext()->Global();
  v8::Local<v8::Function> bufferConstructor = v8::Local<v8::Function>::Cast(globalObj->Get(Nan::New<String>("Buffer").ToLocalChecked()));
  v8::Local<v8::Value> constructorArgs[3] = {buf, Nan::New<v8::Integer>((unsigned)descriptorString.size()), Nan::New<v8::Integer>(0)};
  v8::Local<v8::Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);
  args.GetReturnValue().Set(actualBuffer);
}

int bfMatcherCompare (cv::Mat &descriptors1, cv::Mat &descriptors2) {
  cv::BFMatcher matcher(cv::NORM_HAMMING);
  vector<cv::DMatch> matches;
  matcher.match(descriptors1, descriptors2, matches);
  double max_dist = 0;
  double min_dist = 100;
  /*
  for (int i = 0; i < descriptors1.rows; i++)
  {
    double dist = matches[i].distance;
    if (dist < min_dist)
      min_dist = dist;
    if (dist > max_dist)
      max_dist = dist;
  }
  vector<cv::DMatch> good_matches;
  for (int i = 0; i < descriptors1.rows; i++)
  {
    if (matches[i].distance < 3 * min_dist)
      good_matches.push_back(matches[i]);
  }
  return good_matches.size();
  */
  for (int i = 0; i < descriptors1.rows; i++) {
    double dist = matches[i].distance;
    if (dist < min_dist) {
      min_dist = dist;
    }
    if (dist > max_dist) {
      max_dist = dist;
    }
  }
  std::vector<cv::DMatch> good_matches;
  double good_matches_sum = 0.0;

  for (int i = 0; i < descriptors1.rows; i++) {
    double distance = matches[i].distance;
    if (distance <= std::max(2 * min_dist, 0.02)) {
      good_matches.push_back(matches[i]);
      good_matches_sum += distance;
    }
  }
  return (double) good_matches_sum / (double) good_matches.size();
}

void similarity(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Object> arg1 = args[0]->ToObject();
  int size1 = args[1]->NumberValue();
  Local<Object> arg2 = args[2]->ToObject();
  int size2 = args[3]->NumberValue();
  if (args.Length() < 4) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
    return;
  }
  uchar*buffer1 = (uchar*) node::Buffer::Data(arg1);
  std::vector<uchar> vec1(buffer1, buffer1 +  (unsigned int) size1);
  cv::Mat img_decode1;
  img_decode1 = cv::imdecode(vec1, CV_8U);
  uchar*buffer2 = (uchar*) node::Buffer::Data(arg2);
  std::vector<uchar> vec2(buffer2, buffer2 +  (unsigned int) size2);
  cv::Mat img_decode2;
  img_decode2 = cv::imdecode(vec2, CV_8U);
  int similarity = bfMatcherCompare(img_decode1, img_decode2);
  args.GetReturnValue().Set(similarity);
}

void init(Local<Object> exports, Local<Object> module) {
  NODE_SET_METHOD(exports, "imageFeature", imageFeature);
  NODE_SET_METHOD(exports, "similarity", similarity);
}

NODE_MODULE(addon, init)


// //特征值
// ORB表现不是很好, 比特的形式

// //serialize
// 二进制数据转换为字符串 80 * 500k = 40m

// 设备采用二进制数据存储方式

// //string-序列化: base64
// 云端存储标准: 特征值的提取标准orb(v1.0.0) 版本信息 md5

// //算法问题
// 100万本
// BOW 