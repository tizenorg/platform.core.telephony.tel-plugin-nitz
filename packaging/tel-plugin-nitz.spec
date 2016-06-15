%define test_enable 0
%define test_bindir /opt/usr/devel/usr/bin

%define major 0
%define minor 1
%define patchlevel 76

Name:       tel-plugin-nitz
Summary:    nitz plugin for telephony
Version:    %{major}.%{minor}.%{patchlevel}
Release:    1
Group:      System/Libraries
License:    Apache-2.0
Source0:    tel-plugin-nitz-%{version}.tar.gz
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%if "%{profile}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

BuildRequires:  cmake
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(icu-i18n)
BuildRequires:  pkgconfig(tcore)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(alarm-service)
BuildRequires:  pkgconfig(capi-system-device)

%description
nitz plugin for telephony

%prep
%setup -q

%build
versionint=$[%{major} * 1000000 + %{minor} * 1000 + %{patchlevel}]
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} \
	-DLIB_INSTALL_DIR=%{_libdir} \
	-DVERSION=$versionint
make %{?_smp_mflags}

%post
/sbin/ldconfig

%postun -p /sbin/ldconfig

%install
rm -rf %{buildroot}
%make_install

%files
%manifest tel-plugin-nitz.manifest
%defattr(644,root,root,-)
%{_libdir}/telephony/plugins/*
%if 0%{?test_enable}
%{test_bindir}/nitz_test.sh
%endif
/opt/data/etc/mcctable.xml
