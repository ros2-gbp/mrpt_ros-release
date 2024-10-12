%bcond_without tests
%bcond_without weak_deps

%global __os_install_post %(echo '%{__os_install_post}' | sed -e 's!/usr/lib[^[:space:]]*/brp-python-bytecompile[[:space:]].*$!!g')
%global __provides_exclude_from ^/opt/ros/rolling/.*$
%global __requires_exclude_from ^/opt/ros/rolling/.*$

Name:           ros-rolling-mrpt-libapps
Version:        2.14.3
Release:        1%{?dist}%{?release_suffix}
Summary:        ROS mrpt_libapps package

License:        BSD
URL:            https://www.mrpt.org/
Source0:        %{name}-%{version}.tar.gz

Requires:       ros-rolling-mrpt-libgui
Requires:       ros-rolling-mrpt-libhwdrivers
Requires:       ros-rolling-mrpt-libmaps
Requires:       ros-rolling-mrpt-libslam
Requires:       ros-rolling-mrpt-libtclap
Requires:       ros-rolling-ros-workspace
BuildRequires:  assimp-devel
BuildRequires:  cmake3
BuildRequires:  ffmpeg-free-devel
BuildRequires:  freeglut-devel
BuildRequires:  glfw-devel
BuildRequires:  libXrandr-devel
BuildRequires:  libXxf86vm-devel
BuildRequires:  libfreenect-devel
BuildRequires:  libfreenect-openni
BuildRequires:  libjpeg-turbo-devel
BuildRequires:  libpcap-devel
BuildRequires:  libusbx-devel
BuildRequires:  mesa-libGL-devel
BuildRequires:  mesa-libGLU-devel
BuildRequires:  octomap-devel
BuildRequires:  opencv-devel
BuildRequires:  pkgconfig
BuildRequires:  pybind11-devel
BuildRequires:  python3-pip
BuildRequires:  ros-rolling-ament-cmake
BuildRequires:  ros-rolling-cv-bridge
BuildRequires:  ros-rolling-mrpt-libgui
BuildRequires:  ros-rolling-mrpt-libhwdrivers
BuildRequires:  ros-rolling-mrpt-libmaps
BuildRequires:  ros-rolling-mrpt-libslam
BuildRequires:  ros-rolling-mrpt-libtclap
BuildRequires:  ros-rolling-rclcpp
BuildRequires:  ros-rolling-ros-environment
BuildRequires:  ros-rolling-rosbag2-storage
BuildRequires:  systemd-devel
BuildRequires:  tinyxml2-devel
BuildRequires:  wxGTK3-devel
BuildRequires:  zlib-devel
BuildRequires:  ros-rolling-ros-workspace
Provides:       %{name}-devel = %{version}-%{release}
Provides:       %{name}-doc = %{version}-%{release}
Provides:       %{name}-runtime = %{version}-%{release}

%description
Mobile Robot Programming Toolkit (MRPT) libraries (apps C++ libraries). This
package contains: mrpt-apps lib, mrpt-graphslam

%prep
%autosetup -p1

%build
# In case we're installing to a non-standard location, look for a setup.sh
# in the install tree and source it.  It will set things like
# CMAKE_PREFIX_PATH, PKG_CONFIG_PATH, and PYTHONPATH.
if [ -f "/opt/ros/rolling/setup.sh" ]; then . "/opt/ros/rolling/setup.sh"; fi
mkdir -p .obj-%{_target_platform} && cd .obj-%{_target_platform}
%cmake3 \
    -UINCLUDE_INSTALL_DIR \
    -ULIB_INSTALL_DIR \
    -USYSCONF_INSTALL_DIR \
    -USHARE_INSTALL_PREFIX \
    -ULIB_SUFFIX \
    -DCMAKE_INSTALL_PREFIX="/opt/ros/rolling" \
    -DCMAKE_PREFIX_PATH="/opt/ros/rolling" \
    -DSETUPTOOLS_DEB_LAYOUT=OFF \
%if !0%{?with_tests}
    -DBUILD_TESTING=OFF \
%endif
    ..

%make_build

%install
# In case we're installing to a non-standard location, look for a setup.sh
# in the install tree and source it.  It will set things like
# CMAKE_PREFIX_PATH, PKG_CONFIG_PATH, and PYTHONPATH.
if [ -f "/opt/ros/rolling/setup.sh" ]; then . "/opt/ros/rolling/setup.sh"; fi
%make_install -C .obj-%{_target_platform}

%if 0%{?with_tests}
%check
# Look for a Makefile target with a name indicating that it runs tests
TEST_TARGET=$(%__make -qp -C .obj-%{_target_platform} | sed "s/^\(test\|check\):.*/\\1/;t f;d;:f;q0")
if [ -n "$TEST_TARGET" ]; then
# In case we're installing to a non-standard location, look for a setup.sh
# in the install tree and source it.  It will set things like
# CMAKE_PREFIX_PATH, PKG_CONFIG_PATH, and PYTHONPATH.
if [ -f "/opt/ros/rolling/setup.sh" ]; then . "/opt/ros/rolling/setup.sh"; fi
CTEST_OUTPUT_ON_FAILURE=1 \
    %make_build -C .obj-%{_target_platform} $TEST_TARGET || echo "RPM TESTS FAILED"
else echo "RPM TESTS SKIPPED"; fi
%endif

%files
/opt/ros/rolling

%changelog
* Sat Oct 12 2024 Jose-Luis Blanco-Claraco <joseluisblancoc@gmail.com> - 2.14.3-1
- Autogenerated by Bloom

* Sat Oct 05 2024 Jose-Luis Blanco-Claraco <joseluisblancoc@gmail.com> - 2.14.2-1
- Autogenerated by Bloom

* Wed Sep 25 2024 Jose-Luis Blanco-Claraco <joseluisblancoc@gmail.com> - 2.14.1-1
- Autogenerated by Bloom

* Sun Sep 15 2024 Jose-Luis Blanco-Claraco <joseluisblancoc@gmail.com> - 2.14.0-1
- Autogenerated by Bloom

* Sat Sep 07 2024 Jose-Luis Blanco-Claraco <joseluisblancoc@gmail.com> - 2.13.8-1
- Autogenerated by Bloom

* Thu Aug 29 2024 Jose-Luis Blanco-Claraco <joseluisblancoc@gmail.com> - 2.13.7-3
- Autogenerated by Bloom

* Sun Aug 25 2024 Jose-Luis Blanco-Claraco <joseluisblancoc@gmail.com> - 2.13.7-2
- Autogenerated by Bloom

* Sat Aug 24 2024 Jose-Luis Blanco-Claraco <joseluisblancoc@gmail.com> - 2.13.7-1
- Autogenerated by Bloom

* Wed Aug 21 2024 Jose-Luis Blanco-Claraco <joseluisblancoc@gmail.com> - 2.13.6-4
- Autogenerated by Bloom

* Tue Aug 20 2024 Jose-Luis Blanco-Claraco <joseluisblancoc@gmail.com> - 2.13.6-3
- Autogenerated by Bloom

* Sun Aug 18 2024 Jose-Luis Blanco-Claraco <joseluisblancoc@gmail.com> - 2.13.6-2
- Autogenerated by Bloom

* Wed Aug 14 2024 Jose-Luis Blanco-Claraco <joseluisblancoc@gmail.com> - 2.13.6-1
- Autogenerated by Bloom

