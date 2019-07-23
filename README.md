# Pine Overview
Rine is tiny RTMP (Real Time Messaging Protocol) video streaming server pusher and client for embeded system. it's written in C/C++ and fast, easy to install, and supports multi path. H.264/265 AVC.. and no dependence.

- a static library for RTMP pusher and client ( librtmp.a)
- a executable binary for  push video stream to RTMP server (pine-pusher).
- a executable binary for  live client to receive video stream from RTMP server (pine-client).
- a executable binary for  RTPM server (pine-rtmpsrv) 

## Build Instructions

- It's simple.

- Rine uses CMake as build system, please install cmake first.

  ```shell
  sudo apt-get install cmake
  ```

- Run build.sh shell script

  ```shell
  ./build.sh
  ```

- You will get executable binary under ./build/bin/x86_64 folder.

- if you want to change build tool chain for arm arch, modify CMakeLists.txt.

  ```cmake
  SET(TARGET_STAGING_PREFIX /opt/s32v-build/staging)
  SET(TARGET_SYSROOT /opt/s32v-build/sysroot)
  SET(TARGET_C_COMPILER /opt/s32v-build/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc)
  SET(TARGET_CXX_COMPILER /opt/s32v-build/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++)
  
  #disable/enable build for arm linux system,like S32V
  SET(CMAKE_SYSTEM_PROCESSOR arm)
  #SET(CMAKE_SYSTEM_PROCESSOR x86_64)
  ```

## Testing with FFmpeg

```shell
#start RTMP server
./build/bin/x86_64/pine-rtmpsrv
#To encode and push a stream:
ffmpeg -re -i video.mp4 -f flv rtmp://127.0.0.1/live/stream
#To watch the stream:
ffplay rtmp://server/live/stream
```

## Testing with VLC Player

```shell
#start RTMP server
./build/bin/x86_64/pine-rtmpsrv
#To encode and push a stream:
ffmpeg -re -i video.mp4 -f flv rtmp://127.0.0.1/live/stream
#To watch the stream:
vlc media -> open network stream -> rtmp://server/live/stream
```



## Testing with H.264 file

```shell
#start RTMP server
./build/bin/x86_64/pine-rtmpsrv

#set up a client for receving video stream.
./build/bin/x86_64/pine-client rtmp://127.0.0.1/live/stream

#push a video stream to server
./build/bin/x86_64/pine-pusher test.h264 rtmp://127.0.0.1/live/stream
```



## Testing with Multi path

```shell
#start RTMP server
./build/bin/x86_64/pine-rtmpsrv

#set up a client for receving video stream.
./build/bin/x86_64/pine-client rtmp://127.0.0.1/live/one_stream

./build/bin/x86_64/pine-client rtmp://127.0.0.1/live/two_stream

vlc media -> open network stream -> rtmp://127.0.0.1/live/two_stream

#push a video stream to server
./build/bin/x86_64/pine-pusher test.h264 rtmp://127.0.0.1/live/one_stream
./build/bin/x86_64/pine-pusher test.h264 rtmp://127.0.0.1/live/two_stream
```
