## Opencv Node Addon 

### Opencv 安装使用

- 下载Opencv安装包: http://opencv.org/releases.html

- 解压进入Opencv目录

- 编译Opencv(需要cmake)

```bash
mkdir release
cd release
cmake -G "Unix Makefiles" .. (为Unix 系统生成Makefile，Mac OSX是基于Unix的。未安装cmake 可以通过Homebrew安装，未安装Homebrew需安装Homebrew）
make
```

- 安装Opencv

```bash
make install
```

- 安装完成的目录

```bash
/usr/local/lib (Opencv库文件)
/usr/local/include (Opencv头文件）
/usr/local/share/ (Opencv xml配置文件)
```

- 使用Opencv(C++ Version)

    - 编写CMakeLists.txt文件

      ```
      project( ORBFeatureAndCompare ) // 项目的名称
      cmake_minimum_required(VERSION 2.8) // cmake的版本要求
      find_package( OpenCV REQUIRED ) // 查找对应的Opencv依赖库
      find_package(Boost COMPONENTS log log_setup thread filesystem system) // 查找对应的Boost依赖库(下文出现Boost的安装方法)
      add_executable( ORBFeatureAndCompare ORBFeatureAndCompare )  // 指定可运行的文件
      // 引入对应的依赖库文件的位置
      target_link_libraries(ORBFeatureAndCompare
          ${OpenCV_LIBS}
          ${Boost_LOG_SETUP_LIBRARY}
          ${Boost_LOG_LIBRARY}
          ${Boost_FILESYSTEM_LIBRARY}
          ${Boost_THREAD_LIBRARY}
          ${Boost_SYSTEM_LIBRARY}
      )
      ```
    - 编写Opencv的cpp文件

      ```cpp
      //必要的头文件
      #include <opencv2/core/core.hpp>
      #include <opencv2/highgui/highgui.hpp>
      #include <opencv2/imgproc/imgproc.hpp>
      #include <opencv2/features2d/features2d.hpp>
      #include <boost/filesystem.hpp>
      #include <boost/filesystem/fstream.hpp>
      #include <iostream>
      #include <fstream>
      #include <cstring>
      #include <iterator>
      #include <vector>

      using namespace boost::filesystem;  
      namespace newfs = boost::filesystem;
      using namespace cv;
      using namespace std;

      int main () {
        ....
        return 0;
      }

      ```
    - 创建一个images(其他命名都可以)的资源文件夹

      ```bash
      cd images
      cmake ..
      make
      ./ORBFeatureAndCompare //编译出的可执行程序
      ```

### 安装使用Boost进行数据序列化

- 下载Boost安装包: https://dl.bintray.com/boostorg/release/1.64.0/source/

- 解压进入Boost目录

- 编译安装Boost

```bash
./bootstrap.sh
./b2
```
- 引入Boost库进行数据序列化,同样是在CmakeList中引入文件路径, 在cpp文件中使用


### 封装cpp程序为Node Addon

- 依赖安装

  - 全局安装node-gyp
  - 本地安装nan

- 编写binding.gyp

      ```
      {
        "targets": [
        {
          "target_name": "feature",
          "sources": [ "./src/feature.cc" ],
          "include_dirs": [
            "<!(node -e \"require('nan')\")",
            "/usr/local/include/boost",
            "/usr/local/include/opencv2"
          ],
          "conditions": [
            [ "OS==\"linux\" or OS==\"freebsd\" or OS==\"openbsd\" or OS==\"solaris\" or OS==\"aix\"", {
              }
            ],
            ["OS==\"mac\"", {
                "libraries": [
                  "/usr/local/lib/libboost_log_setup-mt.dylib",
                  "/usr/local/lib/libboost_log-mt.dylib",
                  "/usr/local/lib/libboost_filesystem-mt.dylib",
                  "/usr/local/lib/libboost_thread-mt.dylib",
                  "/usr/local/lib/libboost_system-mt.dylib",
                  "/usr/local/lib/libopencv_core.3.2.0.dylib",
                  "/usr/local/lib/libopencv_highgui.3.2.0.dylib",
                  "/usr/local/lib/libopencv_imgproc.3.2.0.dylib",
                  "/usr/local/lib/libopencv_features2d.3.2.0.dylib"
                ]
              }
            ]
          ]
        }
        ]
      }
      ```

- 编写node c++ 插件

```cpp
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
  cv::Ptr<cv::ORB> orb = cv::ORB::create(4000, 1.2f, 8, 31, 0, 2,             cv::ORB::HARRIS_SCORE, 31, 20);
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
```

- 编写Js文件

```js
const feature = require('./build/Release/feature');

exports.getImageFeature = (filePath) => {
  return feature.imageFeature(filePath).toString('utf8');
};

exports.getImageSimilarity = (descriptor1, descriptor2) => {
  let matBuffer1 = Buffer.from(descriptor1);
  let matBuffer2 = Buffer.from(descriptor2);
  return feature.similarity(matBuffer1, matBuffer1.length, matBuffer2, matBuffer2.length);
};
```

- 编译运行

```bash
node-gyp configure build
node test.js
```

