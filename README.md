| Distro | Build dev | Release |
| --- | --- | --- |
| ROS 2 Humble (u22.04) | [![Build Status](https://build.ros2.org/job/Hdev__mrpt_ros__ubuntu_jammy_amd64/badge/icon)](https://build.ros2.org/job/Hdev__mrpt_ros__ubuntu_jammy_amd64/) | [![Version](https://img.shields.io/ros/v/humble/mrpt_ros)](https://index.ros.org/search/?term=mrpt_ros) |
| ROS 2 Iron (u22.04) | [![Build Status](https://build.ros2.org/job/Idev__mrpt_ros__ubuntu_jammy_amd64/badge/icon)](https://build.ros2.org/job/Idev__mrpt_ros__ubuntu_jammy_amd64/) | [![Version](https://img.shields.io/ros/v/iron/mrpt_ros)](https://index.ros.org/search/?term=mrpt_ros) |
| ROS 2 Jazzy (u24.04) | [![Build Status](https://build.ros2.org/job/Jdev__mrpt_ros__ubuntu_noble_amd64/badge/icon)](https://build.ros2.org/job/Jdev__mrpt_ros__ubuntu_noble_amd64/) | [![Version](https://img.shields.io/ros/v/jazzy/mrpt_ros)](https://index.ros.org/search/?term=mrpt_ros) |
| ROS 2 Rolling (u24.04) | [![Build Status](https://build.ros2.org/job/Rdev__mrpt_ros__ubuntu_noble_amd64/badge/icon)](https://build.ros2.org/job/Rdev__mrpt_ros__ubuntu_noble_amd64/) | [![Version](https://img.shields.io/ros/v/rolling/mrpt_ros)](https://index.ros.org/search/?term=mrpt_ros) |


# mrpt-ros
Fine-grained ROS packages for MRPT libraries and apps. This repository is a replacement for
the usage of the [upstream MRPT/mrpt](https://github.com/MRPT/mrpt) repo directly as the ROS
package `mrpt2`.

## Mapping between ROS packages <==> MRPT C++ libraries

These are the `<depend>...</depend>` tags you need to include in
your project `package.xml` depending on [what C++ libraries you use](https://docs.mrpt.org/reference/latest/modules.html):

| ROS 2 package name  | Included MRPT libraries |
|---|---|
| `<depend>mrpt_libbase</depend>`    | mrpt-io, mrpt-serialization, mrpt-random, mrpt-system, mrpt-rtti, mrpt-containers, mrpt-typemeta, mrpt-core, mrpt-random, mrpt-config, mrpt-expr |
| `<depend>mrpt_libgui</depend>`    | mrpt-gui |
| `<depend>mrpt_libhwdrivers</depend>`    | mrpt-hwdrivers, mrpt-comms |
| `<depend>mrpt_libapps</depend>`    | mrpt-apps |
| `<depend>mrpt_libmaps</depend>`    | mrpt-maps, mrpt-graphs |
| `<depend>mrpt_libmath</depend>`    | mrpt-math |
| `<depend>mrpt_libnav</depend>`    | mrpt-nav, mrpt-kinematics |
| `<depend>mrpt_libobs</depend>`    | mrpt-obs, mrpt-topography |
| `<depend>mrpt_libopengl</depend>`    | mrpt-opengl, mrpt-img |
| `<depend>mrpt_libposes</depend>`    | mrpt-poses, mrpt-tfest, mrpt-bayes |
| `<depend>mrpt_libros2bridge</depend>`    | mrpt-ros2bridge |
| `<depend>mrpt_libslam</depend>`    | mrpt-slam, mrpt-vision |
| `<depend>mrpt_libtclap</depend>`    | mrpt-tclap |
| `<depend>mrpt_apps</depend>`    | Executable [applications](https://docs.mrpt.org/reference/latest/applications.html): RawLogViewer, rawlog-edit, rawlog-grabber, SceneViewer3D, etc. |
| `<depend>python_mrpt</depend>`    | [pymrpt wrapper](https://docs.mrpt.org/reference/latest/wrappers.html) |

Keep in mind that including one C++ library automatically includes all its dependencies, so you do not need to list them all:

![mrpt_libs](docs/graph_mrpt_libs.png)

## Usage

Once binary packages are available via `apt install` from ROS build farm,
you can install required packages like:

```bash
sudo apt install ros-${ROS_DISTRO}-mrpt_libbase  # or any other as needed
```

In the meanwhile, clone this repo and build with colcon as usual:

```bash
cd ~/ros2_ws/src
git clone --recursive https://github.com/MRPT/mrpt_ros.git
```

## Build status matrix

| Package | ROS 2 Humble <br/> BinBuild |  ROS 2 Iron <br/> BinBuild | ROS 2 Jazzy <br/> BinBuild | ROS 2 Rolling <br/> BinBuild |
| --- | --- | --- | --- |--- |
| mrpt_apps | [![Build Status](https://build.ros2.org/job/Hbin_uJ64__mrpt_apps__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Hbin_uJ64__mrpt_apps__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Ibin_uJ64__mrpt_apps__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Ibin_uJ64__mrpt_apps__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Jbin_uN64__mrpt_apps__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Jbin_uN64__mrpt_apps__ubuntu_noble_amd64__binary/) |[![Build Status](https://build.ros2.org/job/Rbin_uN64__mrpt_apps__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Rbin_uN64__mrpt_apps__ubuntu_noble_amd64__binary/) |
| mrpt_libbase | [![Build Status](https://build.ros2.org/job/Hbin_uJ64__mrpt_libbase__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Hbin_uJ64__mrpt_libbase__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Ibin_uJ64__mrpt_libbase__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Ibin_uJ64__mrpt_libbase__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Jbin_uN64__mrpt_libbase__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Jbin_uN64__mrpt_libbase__ubuntu_noble_amd64__binary/) |[![Build Status](https://build.ros2.org/job/Rbin_uN64__mrpt_libbase__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Rbin_uN64__mrpt_libbase__ubuntu_noble_amd64__binary/) |
| mrpt_libgui | [![Build Status](https://build.ros2.org/job/Hbin_uJ64__mrpt_libgui__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Hbin_uJ64__mrpt_libgui__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Ibin_uJ64__mrpt_libgui__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Ibin_uJ64__mrpt_libgui__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Jbin_uN64__mrpt_libgui__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Jbin_uN64__mrpt_libgui__ubuntu_noble_amd64__binary/) |[![Build Status](https://build.ros2.org/job/Rbin_uN64__mrpt_libgui__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Rbin_uN64__mrpt_libgui__ubuntu_noble_amd64__binary/) |
| mrpt_libhwdrivers | [![Build Status](https://build.ros2.org/job/Hbin_uJ64__mrpt_libhwdrivers__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Hbin_uJ64__mrpt_libhwdrivers__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Ibin_uJ64__mrpt_libhwdrivers__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Ibin_uJ64__mrpt_libhwdrivers__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Jbin_uN64__mrpt_libhwdrivers__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Jbin_uN64__mrpt_libhwdrivers__ubuntu_noble_amd64__binary/) |[![Build Status](https://build.ros2.org/job/Rbin_uN64__mrpt_libhwdrivers__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Rbin_uN64__mrpt_libhwdrivers__ubuntu_noble_amd64__binary/) |
| mrpt_libapps | [![Build Status](https://build.ros2.org/job/Hbin_uJ64__mrpt_libapps__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Hbin_uJ64__mrpt_libapps__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Ibin_uJ64__mrpt_libapps__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Ibin_uJ64__mrpt_libapps__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Jbin_uN64__mrpt_libapps__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Jbin_uN64__mrpt_libapps__ubuntu_noble_amd64__binary/) |[![Build Status](https://build.ros2.org/job/Rbin_uN64__mrpt_libapps__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Rbin_uN64__mrpt_libapps__ubuntu_noble_amd64__binary/) |
| mrpt_libmaps | [![Build Status](https://build.ros2.org/job/Hbin_uJ64__mrpt_libmaps__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Hbin_uJ64__mrpt_libmaps__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Ibin_uJ64__mrpt_libmaps__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Ibin_uJ64__mrpt_libmaps__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Jbin_uN64__mrpt_libmaps__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Jbin_uN64__mrpt_libmaps__ubuntu_noble_amd64__binary/) |[![Build Status](https://build.ros2.org/job/Rbin_uN64__mrpt_libmaps__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Rbin_uN64__mrpt_libmaps__ubuntu_noble_amd64__binary/) |
| mrpt_libmath | [![Build Status](https://build.ros2.org/job/Hbin_uJ64__mrpt_libmath__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Hbin_uJ64__mrpt_libmath__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Ibin_uJ64__mrpt_libmath__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Ibin_uJ64__mrpt_libmath__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Jbin_uN64__mrpt_libmath__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Jbin_uN64__mrpt_libmath__ubuntu_noble_amd64__binary/) |[![Build Status](https://build.ros2.org/job/Rbin_uN64__mrpt_libmath__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Rbin_uN64__mrpt_libmath__ubuntu_noble_amd64__binary/) |
| mrpt_libnav | [![Build Status](https://build.ros2.org/job/Hbin_uJ64__mrpt_libnav__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Hbin_uJ64__mrpt_libnav__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Ibin_uJ64__mrpt_libnav__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Ibin_uJ64__mrpt_libnav__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Jbin_uN64__mrpt_libnav__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Jbin_uN64__mrpt_libnav__ubuntu_noble_amd64__binary/) |[![Build Status](https://build.ros2.org/job/Rbin_uN64__mrpt_libnav__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Rbin_uN64__mrpt_libnav__ubuntu_noble_amd64__binary/) |
| mrpt_libobs | [![Build Status](https://build.ros2.org/job/Hbin_uJ64__mrpt_libobs__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Hbin_uJ64__mrpt_libobs__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Ibin_uJ64__mrpt_libobs__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Ibin_uJ64__mrpt_libobs__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Jbin_uN64__mrpt_libobs__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Jbin_uN64__mrpt_libobs__ubuntu_noble_amd64__binary/) |[![Build Status](https://build.ros2.org/job/Rbin_uN64__mrpt_libobs__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Rbin_uN64__mrpt_libobs__ubuntu_noble_amd64__binary/) |
| mrpt_libopengl | [![Build Status](https://build.ros2.org/job/Hbin_uJ64__mrpt_libopengl__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Hbin_uJ64__mrpt_libopengl__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Ibin_uJ64__mrpt_libopengl__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Ibin_uJ64__mrpt_libopengl__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Jbin_uN64__mrpt_libopengl__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Jbin_uN64__mrpt_libopengl__ubuntu_noble_amd64__binary/) |[![Build Status](https://build.ros2.org/job/Rbin_uN64__mrpt_libopengl__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Rbin_uN64__mrpt_libopengl__ubuntu_noble_amd64__binary/) |
| mrpt_libposes | [![Build Status](https://build.ros2.org/job/Hbin_uJ64__mrpt_libposes__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Hbin_uJ64__mrpt_libposes__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Ibin_uJ64__mrpt_libposes__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Ibin_uJ64__mrpt_libposes__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Jbin_uN64__mrpt_libposes__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Jbin_uN64__mrpt_libposes__ubuntu_noble_amd64__binary/) |[![Build Status](https://build.ros2.org/job/Rbin_uN64__mrpt_libposes__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Rbin_uN64__mrpt_libposes__ubuntu_noble_amd64__binary/) |
| mrpt_libros2bridge | [![Build Status](https://build.ros2.org/job/Hbin_uJ64__mrpt_libros2bridge__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Hbin_uJ64__mrpt_libros2bridge__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Ibin_uJ64__mrpt_libros2bridge__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Ibin_uJ64__mrpt_libros2bridge__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Jbin_uN64__mrpt_libros2bridge__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Jbin_uN64__mrpt_libros2bridge__ubuntu_noble_amd64__binary/) |[![Build Status](https://build.ros2.org/job/Rbin_uN64__mrpt_libros2bridge__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Rbin_uN64__mrpt_libros2bridge__ubuntu_noble_amd64__binary/) |
| mrpt_libslam | [![Build Status](https://build.ros2.org/job/Hbin_uJ64__mrpt_libslam__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Hbin_uJ64__mrpt_libslam__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Ibin_uJ64__mrpt_libslam__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Ibin_uJ64__mrpt_libslam__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Jbin_uN64__mrpt_libslam__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Jbin_uN64__mrpt_libslam__ubuntu_noble_amd64__binary/) |[![Build Status](https://build.ros2.org/job/Rbin_uN64__mrpt_libslam__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Rbin_uN64__mrpt_libslam__ubuntu_noble_amd64__binary/) |
| mrpt_libtclap | [![Build Status](https://build.ros2.org/job/Hbin_uJ64__mrpt_libtclap__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Hbin_uJ64__mrpt_libtclap__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Ibin_uJ64__mrpt_libtclap__ubuntu_jammy_amd64__binary/badge/icon)](https://build.ros2.org/job/Ibin_uJ64__mrpt_libtclap__ubuntu_jammy_amd64__binary/) | [![Build Status](https://build.ros2.org/job/Jbin_uN64__mrpt_libtclap__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Jbin_uN64__mrpt_libtclap__ubuntu_noble_amd64__binary/) |[![Build Status](https://build.ros2.org/job/Rbin_uN64__mrpt_libtclap__ubuntu_noble_amd64__binary/badge/icon)](https://build.ros2.org/job/Rbin_uN64__mrpt_libtclap__ubuntu_noble_amd64__binary/) |


## Motivation for the change
- Faster build times (for each individual package). It was common to see ROS build farms to time out.
- Finer grained dependencies: ROS users can now specify in their `<depend>` tags a part of MRPT only, not the whole thing.

So, **the ROS package `mrpt2` is obsolete now (Jul, 2024)**.

## License
BSD-3
