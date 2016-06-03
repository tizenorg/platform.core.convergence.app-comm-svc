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
BuildRequires: pkgconfig(libtzplatform-config)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(json-glib-1.0)
BuildRequires: pkgconfig(libwebsockets)
BuildRequires: pkgconfig(openssl)
BuildRequires: curl
BuildRequires: libcurl-devel
BuildRequires: capi-network-nsd
BuildRequires: capi-network-nsd-devel
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
mkdir -p %{buildroot}%{_libdir}/systemd/system
mkdir -p %{buildroot}%{_libdir}/systemd/system/multi-user.target.wants
ln -sf ../%{name}.service %{buildroot}%{_libdir}/systemd/system/multi-user.target.wants/%{name}.service
ln -sf ../org.tizen.multiscreen.service %{buildroot}%{_libdir}/systemd/system/multi-user.target.wants/org.tizen.multiscreen.service
rm -Rf %{buildroot}%{_descriptiondir}
mkdir -p %{buildroot}%{_descriptiondir}
mkdir -p %{buildroot}/usr/apps/org.tizen.multiscreen

%post
systemctl enable remote-server.service
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%manifest remote-server.manifest
%defattr(-,system,system,-)
%{_bindir}/remote-server
%{_libdir}/systemd/system/%{name}.service
%{_libdir}/systemd/system/org.tizen.multiscreen.service
%{_libdir}/systemd/system/multi-user.target.wants/%{name}.service
%{_libdir}/systemd/system/multi-user.target.wants/org.tizen.multiscreen.service
%{_node_dir}/*
/usr/share/license/%{name}

%files devel

%package -n msf-api
Summary:    Msf-api Shared Library (Development)
Group:      System/Libraries

%description -n msf-api
msf-api Shared Library (DEV)

%post -n msf-api
/sbin/ldconfig

%postun -n msf-api
/sbin/ldconfig

%files -n msf-api
%manifest remote-server.manifest
%defattr(-,root,root,-)
%{_libdir}/libmsf-api.so*
/usr/bin/msf-api-test*

%package -n msf-api-devel
Summary:    Msf-api Shared Library (Development)
Group:      System/Libraries

%description -n msf-api-devel
msf-api Shared Library (DEV)

%post -n msf-api-devel
/sbin/ldconfig

%postun -n msf-api-devel
/sbin/ldconfig

%files -n msf-api-devel
%defattr(-,root,root,-)
%{_includedir}/msf-api/*.h
%{_libdir}/libmsf-api.so*
%{_libdir}/pkgconfig/msf-api.pc
