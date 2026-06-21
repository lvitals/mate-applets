project('mate-applets', 'c',
  version: '1.28.1',
  meson_version: '>= 0.59.0',
  default_options: [
    'warning_level=1',
    'c_std=gnu99',
  ]
)

gnome = import('gnome')
i18n = import('i18n')
cc = meson.get_compiler('c')

gettext_package = 'mate-applets'

prefix = get_option('prefix')
datadir = join_paths(prefix, get_option('datadir'))
libdir = join_paths(prefix, get_option('libdir'))
libexecdir = join_paths(prefix, get_option('libexecdir'))
sysconfdir = join_paths(prefix, get_option('sysconfdir'))
localedir = join_paths(datadir, 'locale')
pkgdatadir = join_paths(datadir, 'mate-applets')
pkgmoduledir = join_paths(libdir, 'mate-applets')
appletdir = join_paths(datadir, 'mate-panel', 'applets')
servicedir = join_paths(datadir, 'dbus-1', 'services')
schemadir = join_paths(datadir, 'glib-2.0', 'schemas')

add_project_arguments('-DHAVE_CONFIG_H', language: 'c')

glib_dep = dependency('glib-2.0', version: '>= 2.50.0')
gobject_dep = dependency('gobject-2.0', version: '>= 2.50.0')
gio_dep = dependency('gio-2.0', version: '>= 2.50.0')
gio_unix_dep = dependency('gio-unix-2.0')
gtk_dep = dependency('gtk+-3.0', version: '>= 3.22.0')
mate_panel_dep = dependency('libmatepanelapplet-4.0', version: '>= 1.25.2')
mate_desktop_dep = dependency('mate-desktop-2.0', version: '>= 1.27.1')
libm_dep = cc.find_library('m', required: false)

gdk_x11_dep = dependency('gdk-x11-3.0')
x11_dep = dependency('x11')
gdk_wayland_dep = dependency('gdk-wayland-3.0')
have_x11 = true
have_wayland = true
x11_deps = [gdk_x11_dep, x11_dep]

in_process = get_option('in-process')
applet_in_process = in_process
applet_in_process_string = in_process ? 'true' : 'false'
default_platforms = 'X11;Wayland;'
x11_only_platforms = 'X11;'
xwayland_platforms = default_platforms

if in_process
  add_project_arguments('-DPANEL_APPLET_FACTORY=MATE_PANEL_APPLET_IN_PROCESS_FACTORY', language: 'c')
  add_project_arguments('-DENABLE_IN_PROCESS', language: 'c')
else
  add_project_arguments('-DPANEL_APPLET_FACTORY=MATE_PANEL_APPLET_OUT_PROCESS_FACTORY', language: 'c')
endif

conf = configuration_data()
conf.set_quoted('PACKAGE', meson.project_name())
conf.set_quoted('PACKAGE_URL', 'https://mate-desktop.org')
conf.set_quoted('VERSION', meson.project_version())
conf.set_quoted('GETTEXT_PACKAGE', gettext_package)
conf.set_quoted('MATELOCALEDIR', localedir)
conf.set_quoted('MATE_PIXMAPSDIR', join_paths(datadir, 'pixmaps'))
conf.set_quoted('PKGDATADIR', pkgdatadir)
conf.set_quoted('DATADIR', datadir)
conf.set_quoted('PKG_DATA_DIR', pkgdatadir)
conf.set_quoted('SYSCONFDIR', sysconfdir)
conf.set_quoted('LIBDIR', libdir)
conf.set_quoted('PREFIX', prefix)
conf.set('ENABLE_NLS', 1)
conf.set('HAVE_CONFIG_H', 1)
conf.set('HAVE_ERR_H', cc.has_header('err.h'))
conf.set('HAVE_SYS_SYSCTL_H', cc.has_header('sys/sysctl.h'))
conf.set('ENABLE_IPV6', cc.compiles('#include <sys/types.h>\n#include <sys/socket.h>\nint main(void) { return socket(AF_INET6, SOCK_STREAM, 0); }', name: 'IPv6 sockets'))
conf.set('HAVE_GETADDRINFO', cc.has_function('getaddrinfo'))
conf.set('HAVE_TM_TM_GMOFF', cc.compiles('#include <time.h>\nint main(void) { struct tm t; return (int)t.tm_gmtoff; }', name: 'struct tm.tm_gmtoff'))
conf.set('HAVE_TIMEZONE', cc.has_header_symbol('time.h', 'timezone'))
conf.set('HAVE__NL_MEASUREMENT_MEASUREMENT', cc.compiles('#include <langinfo.h>\nint main(void) { char c = *((unsigned char *) nl_langinfo(_NL_MEASUREMENT_MEASUREMENT)); return c; }', name: '_NL_MEASUREMENT_MEASUREMENT'))
if have_x11
  conf.set('HAVE_X11', 1)
endif
if have_wayland
  conf.set('HAVE_WAYLAND', 1)
endif

config_inc = include_directories('.')
top_inc = include_directories('.')

common_deps = [glib_dep, gio_dep, gio_unix_dep, gtk_dep, mate_panel_dep]
po_dir = join_paths(meson.project_source_root(), 'po')

libnotify_dep = dependency('libnotify', version: '>= 0.7.0', required: false)
if libnotify_dep.found()
  conf.set('HAVE_LIBNOTIFY', 1)
endif
gucharmap_dep = dependency('gucharmap-2.90', version: '>= 3.0.0', required: false)
if gucharmap_dep.found()
  conf.set('HAVE_GUCHARMAP', 1)
endif
upower_dep = dependency('upower-glib', version: '>= 0.99.8', required: get_option('battstat').enabled())
if upower_dep.found()
  conf.set('HAVE_UPOWER', 1)
endif
xml_dep = dependency('libxml-2.0', version: '>= 2.5.0', required: get_option('stickynotes').enabled())
libwnck_dep = dependency('libwnck-3.0', version: '>= 3.0.0', required: get_option('stickynotes').enabled())
gtop_dep = dependency('libgtop-2.0', version: '>= 2.12.0', required: get_option('gtop-applets').enabled())
mateweather_dep = dependency('mateweather', version: '>= 1.17.0', required: get_option('mateweather').enabled())
libsoup_dep = dependency('libsoup-3.0', version: '>= 3.0.0', required: get_option('invest').enabled())
json_glib_dep = dependency('json-glib-1.0', version: '>= 1.0.0', required: get_option('invest').enabled())
dbus_glib_dep = dependency('dbus-glib-1', version: '>= 0.74', required: false)
polkit_dep = dependency('polkit-gobject-1', version: '>= 0.97', required: get_option('polkit').enabled())
if polkit_dep.found()
  conf.set('HAVE_POLKIT', 1)
endif
nl_dep = dependency('libnl-genl-3.0', required: get_option('nl').enabled())
if nl_dep.found()
  conf.set('HAVE_NL', 1)
endif
iw_dep = disabler()
if not get_option('iw').disabled() and not nl_dep.found()
  iw_dep = cc.find_library('iw', required: get_option('iw').enabled())
  if iw_dep.found() and cc.has_header('iwlib.h')
    conf.set('HAVE_IW', 1)
  endif
endif

cpufreq_lib_dep = disabler()
have_libcpufreq = false
if cc.has_header('cpufreq.h')
  cpupower_dep = cc.find_library('cpupower', required: false)
  cpufreq_dep = cc.find_library('cpufreq', required: false)
  if cpupower_dep.found()
    cpufreq_lib_dep = cpupower_dep
    have_libcpufreq = true
    if cc.has_function('cpupower_is_cpu_online', dependencies: cpupower_dep)
      conf.set('HAVE_IS_CPU_ONLINE', 1)
    endif
    if cc.has_function('cpufreq_get_frequencies', dependencies: cpupower_dep)
      conf.set('HAVE_GET_FREQUENCIES', 1)
    endif
  elif cpufreq_dep.found()
    cpufreq_lib_dep = cpufreq_dep
    have_libcpufreq = true
  endif
endif
if have_libcpufreq
  conf.set('HAVE_LIBCPUFREQ', 1)
endif

system_apm_header = cc.has_header('apm.h')

configure_file(output: 'config.h', configuration: conf)

subdir('po')

install_schema = find_program('glib-compile-schemas', required: false)

appmenu_by_path = {
  'accessx-status/data': ['accessx-status-applet-menu.xml'],
  'battstat': ['battstat-applet-menu.xml', 'battstat-preferences.ui'],
  'charpick': ['charpick-applet-menu.xml'],
  'command/data': ['command-preferences.ui'],
  'cpufreq/data': ['cpufreq-applet-menu.xml', 'cpufreq-preferences.ui'],
  'drivemount/data': ['drivemount-applet-menu.xml'],
  'geyes/data': ['geyes-applet-menu.xml', 'themes.ui'],
  'mateweather/data': ['mateweather-applet-menu.xml', 'mateweather-dialog.ui'],
  'multiload/data': ['multiload-applet-menu.xml', 'properties.ui'],
  'netspeed/data': ['netspeed-details.ui', 'netspeed-menu.xml', 'netspeed-preferences.ui'],
  'stickynotes': ['stickynotes-applet-menu.xml', 'sticky-notes-delete.ui', 'sticky-notes-delete-all.ui', 'sticky-notes-note.ui', 'sticky-notes-preferences.ui', 'sticky-notes-properties.ui'],
  'timerapplet/data': ['timerapplet-preferences.ui'],
  'trashapplet/data': ['trashapplet-menu.xml', 'trashapplet-empty-progress.ui'],
}

foreach appdir, files : appmenu_by_path
  install_files = []
  foreach data_file : files
    install_files += join_paths(appdir, data_file)
  endforeach
  install_data(install_files, install_dir: join_paths(pkgdatadir, appdir.split('/')[0]))
endforeach

install_subdir('geyes/themes', install_dir: join_paths(pkgdatadir, 'geyes'))
install_subdir('cpufreq/pixmaps', install_dir: join_paths(datadir, 'pixmaps', 'mate-cpufreq-applet'))
install_subdir('stickynotes/pixmaps', install_dir: join_paths(datadir, 'pixmaps'))
install_subdir('invest-applet/data/art', install_dir: join_paths(datadir, 'mate-applets', 'invest-applet'))

def_install_data = []

app_specs = []

common_x11_deps = x11_deps

app_specs += [{
  'id': 'charpick',
  'module': 'mate-charpick-applet',
  'exec': 'mate-charpick-applet',
  'sources': ['charpick/charpick.c', 'charpick/properties.c'],
  'includes': ['charpick'],
  'deps': common_deps + [gucharmap_dep],
  'c_args': ['-DCHARPICK_RESOURCE_PATH="/org/mate/mate-applets/charpick/"'],
  'gresource': 'charpick/charpick-resources.gresource.xml',
  'gresource_source_dir': 'charpick',
  'gresource_c_name': 'charpick',
  'desktop': 'charpick/org.mate.applets.CharpickerApplet.mate-panel-applet.desktop.in.in',
  'service': 'charpick/org.mate.panel.applet.CharpickerAppletFactory.service.in',
  'schema': 'charpick/org.mate.panel.applet.charpick.gschema.xml.in',
  'platforms': default_platforms,
  'enabled': true,
}]

app_specs += [{
  'id': 'command',
  'module': 'command-applet',
  'exec': 'command-applet',
  'sources': ['command/src/command.c', 'command/src/ma-command.c'],
  'includes': ['command/src'],
  'deps': common_deps,
  'c_args': [],
  'gresource': 'command/data/command-resources.gresource.xml',
  'gresource_source_dir': 'command/data',
  'gresource_c_name': 'command',
  'desktop': 'command/data/org.mate.applets.CommandApplet.mate-panel-applet.desktop.in.in',
  'service': 'command/data/org.mate.panel.applet.CommandAppletFactory.service.in',
  'schema': 'command/data/org.mate.panel.applet.command.gschema.xml.in',
  'platforms': default_platforms,
  'enabled': true,
}]

app_specs += [{
  'id': 'drivemount',
  'module': 'mate-drivemount-applet',
  'exec': 'mate-drivemount-applet',
  'sources': ['drivemount/src/drivemount.c', 'drivemount/src/drive-list.c', 'drivemount/src/drive-button.c'],
  'includes': ['drivemount/src'],
  'deps': common_deps + [mate_desktop_dep],
  'c_args': ['-DDRIVEMOUNT_RESOURCE_PATH="/org/mate/mate-applets/drivemount/"'],
  'gresource': 'drivemount/data/drivemount-resources.gresource.xml',
  'gresource_source_dir': 'drivemount/data',
  'gresource_c_name': 'drivemount',
  'desktop': 'drivemount/data/org.mate.applets.DriveMountApplet.mate-panel-applet.desktop.in.in',
  'service': 'drivemount/data/org.mate.panel.applet.DriveMountAppletFactory.service.in',
  'schema': 'drivemount/data/org.mate.drivemount.gschema.xml.in',
  'platforms': default_platforms,
  'enabled': true,
}]

app_specs += [{
  'id': 'geyes',
  'module': 'mate-geyes-applet',
  'exec': 'mate-geyes-applet',
  'sources': ['geyes/src/geyes.c', 'geyes/src/themes.c'],
  'includes': ['geyes/src'],
  'deps': common_deps + [libm_dep],
  'c_args': ['-DGEYES_THEMES_DIR="@0@"'.format(join_paths(pkgdatadir, 'geyes')), '-DGEYES_RESOURCE_PATH="/org/mate/mate-applets/eyes/"'],
  'gresource': 'geyes/data/geyes-resources.gresource.xml',
  'gresource_source_dir': 'geyes/data',
  'gresource_c_name': 'eyes',
  'desktop': 'geyes/data/org.mate.applets.GeyesApplet.mate-panel-applet.desktop.in.in',
  'service': 'geyes/data/org.mate.panel.applet.GeyesAppletFactory.service.in',
  'schema': 'geyes/data/org.mate.panel.applet.geyes.gschema.xml.in',
  'platforms': default_platforms,
  'enabled': true,
}]

app_specs += [{
  'id': 'trash',
  'module': 'mate-trash-applet',
  'exec': 'trashapplet',
  'sources': ['trashapplet/src/trashapplet.c', 'trashapplet/src/trash-empty.c'],
  'includes': ['trashapplet/src'],
  'deps': common_deps + common_x11_deps,
  'c_args': ['-DGRESOURCE="/org/mate/mate-applets/trash/"'],
  'gresource': 'trashapplet/data/trashapplet-resources.gresource.xml',
  'gresource_source_dir': 'trashapplet/data',
  'gresource_c_name': 'trashapplet',
  'desktop': 'trashapplet/data/org.mate.applets.TrashApplet.mate-panel-applet.desktop.in.in',
  'service': 'trashapplet/data/org.mate.panel.applet.TrashAppletFactory.service.in',
  'schema': '',
  'platforms': default_platforms,
  'enabled': true,
}]

if libnotify_dep.found() and not get_option('timer').disabled()
  app_specs += [{
    'id': 'timer',
    'module': 'mate-timer-applet',
    'exec': 'timer-applet',
    'sources': ['timerapplet/src/timerapplet.c'],
    'includes': ['timerapplet/src'],
    'deps': common_deps + [libnotify_dep],
    'c_args': [],
    'gresource': 'timerapplet/data/timerapplet-resources.gresource.xml',
    'gresource_source_dir': 'timerapplet/data',
    'gresource_c_name': 'timerapplet',
    'desktop': 'timerapplet/data/org.mate.applets.TimerApplet.mate-panel-applet.desktop.in.in',
    'service': 'timerapplet/data/org.mate.panel.applet.TimerAppletFactory.service.in',
    'schema': 'timerapplet/data/org.mate.panel.applet.timer.gschema.xml.in',
    'platforms': default_platforms,
    'enabled': true,
  }]
endif

if gtop_dep.found() and not get_option('gtop-applets').disabled()
  app_specs += [{
    'id': 'multiload',
    'module': 'mate-multiload-applet',
    'exec': 'mate-multiload-applet',
    'sources': ['multiload/src/linux-proc.c', 'multiload/src/load-graph.c', 'multiload/src/main.c', 'multiload/src/properties.c', 'multiload/src/netspeed.c', 'multiload/src/autoscaler.c'],
    'includes': ['multiload/src'],
    'deps': common_deps + [gtop_dep, libm_dep],
    'c_args': ['-DMULTILOAD_RESOURCE_PATH="/org/mate/mate-applets/multiload/"'],
    'gresource': 'multiload/data/multiload-resources.gresource.xml',
    'gresource_source_dir': 'multiload/data',
    'gresource_c_name': 'multiload',
    'desktop': 'multiload/data/org.mate.applets.MultiLoadApplet.mate-panel-applet.desktop.in.in',
    'service': 'multiload/data/org.mate.panel.applet.MultiLoadAppletFactory.service.in',
    'schema': 'multiload/data/org.mate.panel.applet.multiload.gschema.xml.in',
    'platforms': default_platforms,
    'enabled': true,
  }]
  netspeed_deps = common_deps + [gtop_dep, libm_dep]
  netspeed_sources = ['netspeed/src/backend.c', 'netspeed/src/netspeed.c', 'netspeed/src/netspeed-preferences.c']
  if nl_dep.found()
    netspeed_deps += [nl_dep]
    netspeed_sources += ['netspeed/src/nl80211.h', 'netspeed/src/ieee80211.h']
  endif
  if iw_dep.found()
    netspeed_deps += [iw_dep]
  endif
  app_specs += [{
    'id': 'netspeed',
    'module': 'mate-netspeed-applet',
    'exec': 'mate-netspeed-applet',
    'sources': netspeed_sources,
    'includes': ['netspeed/src'],
    'deps': netspeed_deps,
    'c_args': ['-DNETSPEED_RESOURCE_PATH="/org/mate/mate-applets/netspeed/"'],
    'gresource': 'netspeed/data/netspeed-resources.gresource.xml',
    'gresource_source_dir': 'netspeed/data',
    'gresource_c_name': 'netspeed',
    'desktop': 'netspeed/data/org.mate.applets.NetspeedApplet.mate-panel-applet.desktop.in.in',
    'service': 'netspeed/data/org.mate.panel.applet.NetspeedAppletFactory.service.in',
    'schema': 'netspeed/data/org.mate.panel.applet.netspeed.gschema.xml.in',
    'platforms': default_platforms,
    'enabled': true,
  }]
endif

if mateweather_dep.found() and not get_option('mateweather').disabled()
  app_specs += [{
    'id': 'mateweather',
    'module': 'mateweather-applet',
    'exec': 'mateweather-applet',
    'sources': ['mateweather/src/main.c', 'mateweather/src/mateweather-about.c', 'mateweather/src/mateweather-pref.c', 'mateweather/src/mateweather-dialog.c', 'mateweather/src/mateweather-applet.c'],
    'includes': ['mateweather/src'],
    'deps': common_deps + [mateweather_dep, libnotify_dep],
    'c_args': ['-DWEATHER_RESOURCE_PATH="/org/mate/mate-applets/mateweather/"'],
    'gresource': 'mateweather/data/mateweather-resources.gresource.xml',
    'gresource_source_dir': 'mateweather/data',
    'gresource_c_name': 'mateweather',
    'desktop': 'mateweather/data/org.mate.applets.MateWeatherApplet.mate-panel-applet.desktop.in.in',
    'service': 'mateweather/data/org.mate.panel.applet.MateWeatherAppletFactory.service.in',
    'schema': '',
    'platforms': default_platforms,
    'enabled': true,
  }]
endif

if upower_dep.found() and libnotify_dep.found() and host_machine.system() == 'linux' and not get_option('battstat').disabled()
  battstat_sources = ['battstat/battstat_applet.c', 'battstat/battstat-preferences.c', 'battstat/power-management.c', 'battstat/acpi-linux.c', 'battstat/acpi-freebsd.c', 'battstat/battstat-upower.c']
  battstat_includes = ['battstat']
  if not system_apm_header
    battstat_sources += ['battstat/apmlib/apmlib.c']
    battstat_includes += ['battstat/apmlib']
  endif
  app_specs += [{
    'id': 'battstat',
    'module': 'mate-battstat-applet',
    'exec': 'battstat-applet',
    'sources': battstat_sources,
    'includes': battstat_includes,
    'deps': common_deps + [libnotify_dep, upower_dep, libm_dep],
    'c_args': ['-DG_LOG_DOMAIN="battstat_applet"', '-DBATTSTAT_RESOURCE_PATH="/org/mate/mate-applets/battstat/"'],
    'gresource': 'battstat/battstat-resources.gresource.xml',
    'gresource_source_dir': 'battstat',
    'gresource_c_name': 'battstat',
    'desktop': 'battstat/org.mate.applets.BattstatApplet.mate-panel-applet.desktop.in.in',
    'service': 'battstat/org.mate.panel.applet.BattstatAppletFactory.service.in',
    'schema': 'battstat/org.mate.panel.applet.battstat.gschema.xml.in',
    'platforms': default_platforms,
    'enabled': true,
  }]
endif

if host_machine.system() == 'linux' and not get_option('cpufreq').disabled()
  cpufreq_sources = ['cpufreq/src/cpufreq-applet.c', 'cpufreq/src/cpufreq-utils.c', 'cpufreq/src/cpufreq-prefs.c', 'cpufreq/src/cpufreq-selector.c', 'cpufreq/src/cpufreq-popup.c', 'cpufreq/src/cpufreq-monitor.c', 'cpufreq/src/cpufreq-monitor-factory.c']
  cpufreq_deps = common_deps + [gio_dep]
  if have_libcpufreq
    cpufreq_sources += ['cpufreq/src/cpufreq-monitor-libcpufreq.c']
    cpufreq_deps += [cpufreq_lib_dep]
  else
    cpufreq_sources += ['cpufreq/src/cpufreq-monitor-cpuinfo.c', 'cpufreq/src/cpufreq-monitor-sysfs.c']
  endif
  app_specs += [{
    'id': 'cpufreq',
    'module': 'mate-cpufreq-applet',
    'exec': 'mate-cpufreq-applet',
    'sources': cpufreq_sources,
    'includes': ['cpufreq/src'],
    'deps': cpufreq_deps,
    'c_args': ['-DCPUFREQ_RESOURCE_PATH="/org/mate/mate-applets/cpufreq/"'],
    'gresource': 'cpufreq/data/cpufreq-resources.gresource.xml',
    'gresource_source_dir': 'cpufreq/data',
    'gresource_c_name': 'cpufreq',
    'desktop': 'cpufreq/data/org.mate.applets.CPUFreqApplet.mate-panel-applet.desktop.in.in',
    'service': 'cpufreq/data/org.mate.panel.applet.CPUFreqAppletFactory.service.in',
    'schema': 'cpufreq/data/org.mate.panel.applet.cpufreq.gschema.xml.in',
    'platforms': default_platforms,
    'enabled': true,
  }]
endif

if libsoup_dep.found() and json_glib_dep.found() and not get_option('invest').disabled()
  app_specs += [{
    'id': 'invest',
    'module': 'invest-applet',
    'exec': 'invest-applet',
    'sources': ['invest-applet/invest/invest-applet.c', 'invest-applet/invest/invest-applet-chart.c'],
    'includes': ['invest-applet/invest'],
    'deps': common_deps + [libsoup_dep, json_glib_dep],
    'c_args': [],
    'gresource': '',
    'gresource_source_dir': '',
    'gresource_c_name': '',
    'desktop': 'invest-applet/data/org.mate.applets.InvestApplet.mate-panel-applet.in.in',
    'service': 'invest-applet/data/org.mate.panel.applet.InvestAppletFactory.service.in',
    'schema': 'invest-applet/data/org.mate.panel.applet.invest.gschema.xml.in',
    'platforms': default_platforms,
    'enabled': true,
  }]
endif

if cc.has_header('X11/extensions/XKB.h') and not get_option('accessx').disabled()
  app_specs += [{
    'id': 'accessx-status',
    'module': 'mate-accessx-status-applet',
    'exec': 'accessx-status-applet',
    'sources': ['accessx-status/src/applet.c'],
    'includes': ['accessx-status/src'],
    'deps': common_deps + x11_deps,
    'c_args': ['-DACCESSX_RESOURCE_PATH="/org/mate/mate-applets/accessx-status/"'],
    'gresource': 'accessx-status/data/accessx-status-resources.gresource.xml',
    'gresource_source_dir': 'accessx-status/data',
    'gresource_c_name': 'accessx',
    'desktop': 'accessx-status/data/org.mate.applets.AccessxStatusApplet.mate-panel-applet.desktop.in.in',
    'service': 'accessx-status/data/org.mate.panel.applet.AccessxStatusAppletFactory.service.in',
    'schema': '',
    'platforms': in_process ? x11_only_platforms : xwayland_platforms,
    'xwayland': not in_process,
    'enabled': true,
  }]
endif

if libwnck_dep.found() and xml_dep.found() and not get_option('stickynotes').disabled()
  app_specs += [{
    'id': 'stickynotes',
    'module': 'mate-stickynotes-applet',
    'exec': 'stickynotes-applet',
    'sources': ['stickynotes/util.c', 'stickynotes/stickynotes.c', 'stickynotes/stickynotes_callbacks.c', 'stickynotes/stickynotes_applet.c', 'stickynotes/stickynotes_applet_callbacks.c'],
    'includes': ['stickynotes'],
    'deps': common_deps + [libwnck_dep, xml_dep] + x11_deps,
    'c_args': ['-DGRESOURCE="/org/mate/mate-applets/sticky-notes/"'],
    'gresource': 'stickynotes/sticky-notes-resources.gresource.xml',
    'gresource_source_dir': 'stickynotes',
    'gresource_c_name': 'sticky_notes',
    'desktop': 'stickynotes/org.mate.applets.StickyNotesApplet.mate-panel-applet.desktop.in.in',
    'service': 'stickynotes/org.mate.panel.applet.StickyNotesAppletFactory.service.in',
    'schema': 'stickynotes/org.mate.stickynotes.gschema.xml.in',
    'platforms': in_process ? x11_only_platforms : xwayland_platforms,
    'xwayland': not in_process,
    'enabled': true,
  }]
endif

foreach spec : app_specs
  app_sources = spec['sources']
  app_includes = [config_inc]
  foreach app_include : spec['includes']
    app_includes += include_directories(app_include)
  endforeach
  if spec['gresource'] != ''
    app_sources += gnome.compile_resources(
      spec['id'] + '-resources',
      spec['gresource'],
      source_dir: spec['gresource_source_dir'],
      c_name: spec['gresource_c_name']
    )
  endif

  if in_process
    app_location = join_paths(pkgmoduledir, 'lib' + spec['module'] + '.so')
    service_location = app_location
    shared_library(spec['module'],
      app_sources,
      include_directories: app_includes,
      c_args: spec['c_args'],
      dependencies: spec['deps'],
      install: true,
      install_dir: pkgmoduledir
    )
  else
    app_location = join_paths(libexecdir, spec['exec'])
    service_location = app_location
    if spec.get('xwayland', false)
      service_location = 'env GDK_BACKEND=x11 ' + app_location
    endif
    executable(spec['exec'],
      app_sources,
      include_directories: app_includes,
      c_args: spec['c_args'],
      dependencies: spec['deps'],
      install: true,
      install_dir: libexecdir
    )
  endif

  applet_conf = configuration_data()
  applet_conf.set('APPLET_LOCATION', app_location)
  applet_conf.set('APPLET_IN_PROCESS', applet_in_process_string)
  applet_conf.set('APPLET_PLATFORMS', spec['platforms'])
  applet_conf.set('VERSION', meson.project_version())

  desktop_in = configure_file(
    input: spec['desktop'],
    output: '@BASENAME@'.replace('.in.in', '.in'),
    configuration: applet_conf
  )
  desktop_name = spec['desktop'].split('/')[-1]
  if desktop_name.endswith('.mate-panel-applet.desktop.in.in')
    desktop_name = desktop_name.replace('.mate-panel-applet.desktop.in.in', '.mate-panel-applet')
  elif desktop_name.endswith('.desktop.in.in')
    desktop_name = desktop_name.replace('.desktop.in.in', '.mate-panel-applet')
  else
    desktop_name = desktop_name.replace('.in.in', '')
  endif
  i18n.merge_file(
    input: desktop_in,
    output: desktop_name,
    type: 'desktop',
    po_dir: po_dir,
    install: true,
    install_dir: appletdir
  )

  if spec['service'] != '' and not in_process
    service_conf = configuration_data()
    service_conf.set('APPLET_LOCATION', service_location)
    configure_file(
      input: spec['service'],
      output: spec['service'].split('/')[-1].replace('.in', ''),
      configuration: service_conf,
      install: true,
      install_dir: servicedir
    )
  endif

  if spec['schema'] != ''
    schema_name = spec['schema'].split('/')[-1]
    if schema_name.endswith('.xml.in')
      schema_name = schema_name.replace('.xml.in', '.xml')
    endif
    schema_conf = configuration_data()
    schema_conf.set('GETTEXT_PACKAGE', gettext_package)
    configure_file(
      input: spec['schema'],
      output: schema_name,
      configuration: schema_conf,
      install: true,
      install_dir: schemadir
    )
  endif
endforeach

summary('X11 support', have_x11)
summary('Wayland support', have_wayland)
summary('Applet mode', applet_in_process ? 'in-process' : 'out-of-process')
built_applets = []
foreach spec : app_specs
  built_applets += spec['id']
endforeach
summary('Built applets', ', '.join(built_applets))

gnome.post_install(
  glib_compile_schemas: true,
  gtk_update_icon_cache: true,
)
