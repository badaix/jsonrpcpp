prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${prefix}/@CMAKE_INSTALL_LIBDIR@
includedir=${prefix}/@CMAKE_INSTALL_INCLUDEDIR@

Name: @PROJECT_NAME@
Description: @PROJECT_DESCRIPTION@
URL: @PROJECT_URL@
Version: @PROJECT_VERSION@

Libs: -L${libdir} -ljsonrpcpp
Cflags: -I${includedir}
