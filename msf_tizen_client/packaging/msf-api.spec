Name:       msf-api
Summary:    msf-api Shared Library
Version:    0.1.1
Release:    1
Group:      System/Libraries
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz

BuildRequires: cmake
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(json-glib-1.0)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(libwebsockets)
BuildRequires: libcurl-devel
BuildRequires: pkgconfig(openssl)
BuildRequires: capi-network-nsd
BuildRequires: capi-network-nsd-devel
BuildRequires: curl

%description
msf-api Shared Library

%prep
%setup -q

%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`

export   CFLAGS+=" -Wextra -Wcast-align -Wcast-qual -Wshadow -Wwrite-strings -Wswitch-default"
export CXXFLAGS+=" -Wextra -Wcast-align -Wcast-qual -Wshadow -Wwrite-strings -Wswitch-default -Wnon-virtual-dtor -Wno-c++0x-compat -std=c++0x"

export   CFLAGS+=" -Wno-unused-parameter -Wno-empty-body"
export CXXFLAGS+=" -Wno-unused-parameter -Wno-empty-body"

export   CFLAGS+=" -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-strict-aliasing -fno-unroll-loops -fsigned-char -fstrict-overflow -fno-common"
export CXXFLAGS+=" -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-strict-aliasing -fno-unroll-loops -fsigned-char -fstrict-overflow"

export   CFLAGS+=" -DTIZEN_ENGINEER_MODE"
export CXXFLAGS+=" -DTIZEN_ENGINEER_MODE"
export   FFLAGS+=" -DTIZEN_ENGINEER_MODE"

cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} -DMAJORVER=${MAJORVER} -DFULLVER=%{version}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}/usr/share/license
cp LICENSE %{buildroot}/usr/share/license/%{name}

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%manifest packaging/%{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/*.so*
/usr/share/license/%{name}
/usr/bin/msf-api-test*

%package devel
Summary:    msf-api Shared Library (Development)
Group:      System/Libraries
Requires:	%{name} = %{version}-%{release}

%description devel
msf-api Shared Library (DEV)

%files devel
%defattr(-,root,root,-)
%{_includedir}/msf-api/*.h
%{_libdir}/pkgconfig/%{name}.pc
