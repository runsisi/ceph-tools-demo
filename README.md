### Building

- copy to root of ceph source tree (which has all libraries been built)
- then build as normal CMake project (requires GCC 7+)
- verified on ceph v12.2.13 & v14.2.10

e.g.,

- building ceph

```bash
$ cd ~/build/14.2.10
$ mkdir build
$ cd build/
$ . /opt/rh/devtoolset-8/enable
$ cmake3 .. -DWITH_PYTHON3=3.6 -DWITH_MGR_DASHBOARD_FRONTEND=OFF -DWITH_PYTHON2=ON -DMGR_PYTHON_VERSION=3 -DWITH_LTTNG=OFF -DWITH_BABELTRACE=OFF -DWITH_RDMA=OFF -DWITH_LEVELDB=OFF -DWITH_LZ4=OFF -DWITH_GRAFANA=OFF -DWITH_RADOSGW_AMQP_ENDPOINT=OFF -DWITH_TESTS=ON
$ make -j16
```

- building ceph-tools-demo

```bash
$ cd ~/build/
$ cp -a ceph-tools-demo/ 14.2.10/
$ cd 14.2.10/ceph-tools-demo/
$ mkdir build
$ cd build/
$ . /opt/rh/devtoolset-8/enable
$ cmake ..
-- The C compiler identification is GNU 8.2.1
-- The CXX compiler identification is GNU 8.2.1
-- Check for working C compiler: /opt/rh/devtoolset-8/root/usr/bin/cc
-- Check for working C compiler: /opt/rh/devtoolset-8/root/usr/bin/cc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working CXX compiler: /opt/rh/devtoolset-8/root/usr/bin/c++
-- Check for working CXX compiler: /opt/rh/devtoolset-8/root/usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Performing Test COMPILER_SUPPORTS_CXX17
-- Performing Test COMPILER_SUPPORTS_CXX17 - Success
-- Performing Test COMPILER_SUPPORTS_GNU99
-- Performing Test COMPILER_SUPPORTS_GNU99 - Success
-- Configuring done
-- Generating done
-- Build files have been written to: /home/runsisi/build/14.2.10/ceph-tools-demo/build
$ make
Scanning dependencies of target list-snaps
[100%] Building CXX object list_snaps/CMakeFiles/list-snaps.dir/list_snaps.cc.o
Linking CXX executable list-snaps
[100%] Built target list-snaps
$ ldd list_snaps/list-snaps 
        linux-vdso.so.1 =>  (0x00007fff775e2000)
        librbd.so.1 => /home/runsisi/build/14.2.10/ceph-tools-demo/../build/lib/librbd.so.1 (0x00007ffb82911000)
        librados.so.2 => /home/runsisi/build/14.2.10/ceph-tools-demo/../build/lib/librados.so.2 (0x00007ffb825ed000)
        libceph-common.so.0 => /home/runsisi/build/14.2.10/ceph-tools-demo/../build/lib/libceph-common.so.0 (0x00007ffb7981f000)
...
$ readelf -d list_snaps/list-snaps

Dynamic section at offset 0x2b7db8 contains 31 entries:
  Tag        Type                         Name/Value
 0x0000000000000001 (NEEDED)             Shared library: [librbd.so.1]
 0x0000000000000001 (NEEDED)             Shared library: [librados.so.2]
 0x0000000000000001 (NEEDED)             Shared library: [libceph-common.so.0]
 0x0000000000000001 (NEEDED)             Shared library: [libstdc++.so.6]
 0x0000000000000001 (NEEDED)             Shared library: [libm.so.6]
 0x0000000000000001 (NEEDED)             Shared library: [libgcc_s.so.1]
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
 0x000000000000000f (RPATH)              Library rpath: [/home/runsisi/build/14.2.10/ceph-tools-demo/../build/boost/lib:/home/runsisi/build/14.2.10/ceph-tools-demo/../build/lib:]
...
$ make install DESTDIR=xxx
[100%] Built target list-snaps
Install the project...
-- Install configuration: ""
-- Installing: xxx/usr/bin/list-snaps
-- Set runtime path of "xxx/usr/bin/list-snaps" to "/usr/lib64/ceph"
$ readelf -d xxx/usr/bin/list-snaps

Dynamic section at offset 0x2b7db8 contains 31 entries:
  Tag        Type                         Name/Value
 0x0000000000000001 (NEEDED)             Shared library: [librbd.so.1]
 0x0000000000000001 (NEEDED)             Shared library: [librados.so.2]
 0x0000000000000001 (NEEDED)             Shared library: [libceph-common.so.0]
 0x0000000000000001 (NEEDED)             Shared library: [libstdc++.so.6]
 0x0000000000000001 (NEEDED)             Shared library: [libm.so.6]
 0x0000000000000001 (NEEDED)             Shared library: [libgcc_s.so.1]
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
 0x000000000000000f (RPATH)              Library rpath: [/usr/lib64/ceph]
...
```

- experiment

```bash
$ ./list_snaps/list-snaps -p rbd rbd_data.5f2a6b8b4567.0000000000000000
{
  "clones": [
    {
      "cloneid": 4,
      "overlap": [
        [
          0,
          624640
        ],
        [
          625664,
          1459200
        ],
        [
          2085888,
          265216
        ]
      ],
      "size": 2351104,
      "snaps": [
        4
      ]
    },
    {
      "cloneid": -2,
      "overlap": [],
      "size": 2351104,
      "snaps": []
    }
  ],
  "seq": 4
}
```
