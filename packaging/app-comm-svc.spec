Name: remote-server
#year.month // 2016 => 3
Version: 3.1
#day
Release: 18
Summary: Remote server NService Framework

License: Apache-2.0
Source0: %{name}-%{version}.tar.gz
Source1: %{name}.service
Source2: org.tizen.multiscreen.service

ExcludeArch: aarch64 x86_64

BuildRequires: cmake
BuildRequires: boost-system
BuildRequires: boost-thread
BuildRequires: boost-devel
BuildRequires: openssl
BuildRequires: openssl-devel
BuildRequires: expat-devel
BuildRequires: pkgconfig(capi-system-info)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(dbus-glib-1)
BuildRequires: pkgconfig(libcurl)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(syspopup)
BuildRequires: pkgconfig(syspopup-caller)
BuildRequires: pkgconfig(capi-system-info)
BuildRequires: pkgconfig(capi-network-wifi)
BuildRequires: pkgconfig(capi-network-connection)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-system-system-settings)
BuildRequires: pkgconfig(capi-appfw-app-manager)
BuildRequires: pkgconfig(libsystemd)
BuildRequires: pkgconfig(jsoncpp)
BuildRequires: pkgconfig(deviced)
BuildRequires: pkgconfig(libxml-2.0)
BuildRequires: pkgconfig(bluetooth-api)
BuildRequires: pkgconfig(argos_watchdog)
BuildRequires: pkgconfig(aul)
Requires: dbus
#Requires(post): sys-assert
Requires: syspopup

%package devel
Summary:	remote-server header files and .pc file
Group: 		Development/Libraries
Requires:   %{name} = %{version}

%description devel
THis package contains the head files and .pc file for remote-server
%description
Remote server NService Framework to handle the communication between TV App and Mobile App

%define _systemddir /usr/lib/systemd/system
%define _descriptiondir /opt/usr/apps/remote-server
%define _chip_platform `cat %{BUILD_FLAG_PATH}/model-config-chip`
%define _product_type `cat %{BUILD_FLAG_PATH}/model-config-product-type`
%define _node_dir /usr/apps/

%prep
%setup -q
%build

export CFLAGS+=" -fPIC -fvisibility=hidden"
export LDFLAGS+=" -Wl,--rpath=/usr/lib -Wl,--as-needed -Wl,--unresolved-symbols=ignore-in-shared-libs -pie"


cmake . -DCMAKE_INSTALL_PREFIX=/usr/  \
    -DCHIP_NAME=%{_chip_platform}  \
    -DPRODUCT_TYPE=%{_product_type}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp %{_builddir}/%{name}-%{version}/LICENSE.APLv2  %{buildroot}/usr/share/license/%{name}

%make_install
rm -Rf %{buildroot}%{_descriptiondir}
mkdir -p %{buildroot}%{_descriptiondir}
mkdir -p %{buildroot}/usr/apps/org.tizen.multiscreen

%post
mkdir -p %{_unitdir_user}/default.target.wants
ln -s ../%{name}.service %{_unitdir_user}/default.target.wants/
ln -s ../org.tizen.multiscreen.service %{_unitdir_user}/default.target.wants/
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%manifest remote-server.manifest
%defattr(-,system,system,-)
%{_bindir}/remote-server
%attr(644,root,root) %{_unitdir_user}/%{name}.service
%attr(644,root,root) %{_unitdir_user}/org.tizen.multiscreen.service
%{_node_dir}/*
/usr/share/license/%{name}

%files devel
