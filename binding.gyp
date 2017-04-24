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