%define major 0
%define minor 1
%define patchlevel 19
Name:       tel-plugin-nitz
Summary:    nitz plugin for telephony
Version:    %{major}.%{minor}.%{patchlevel}
Release:    1
Group:      System/Libraries
License:    Apache
Source0:    tel-plugin-nitz-%{version}.tar.gz
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  cmake
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(gio-2.0)
BuildRequires:  pkgconfig(tcore)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(deviced)

%description
nitz plugin for telephony

%prep
%setup -q

%build
versionint=$[%{major} * 1000000 + %{minor} * 1000 + %{patchlevel}]
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} -DVERSION=$versionint
make %{?jobs:-j%jobs}

%post
/sbin/ldconfig

%postun -p /sbin/ldconfig

%install
rm -rf %{buildroot}
%make_install

%files
%manifest tel-plugin-nitz.manifest
%defattr(-,root,root,-)
#%doc COPYING
%{_libdir}/telephony/plugins/*
