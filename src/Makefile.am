libcommon_a_SOURCES = md5.cpp \
common.cpp \
lm_en_decrypt.cpp \
timestamp.cpp \
thread_group.cpp \
critical_section.cpp \
debugtrace.cpp \
standard_serialize.cpp \
net_pack.cpp \
tcp_socket.cpp \
net_epoll.cpp \
udp_socket.cpp \
configure.cpp \
dynamic_lib.cpp \
tcp_stream.cpp \
libnet.cpp \
host_ip_cache.cpp \
http_client.cpp \
file_stat_manager.cpp \
time_base.cpp

libcommon_a_HEADERS = define.h \
md5.h \
list.h \
debugtrace.h \
common.h \
lm_en_decrypt.h \
safelist.h \
timestamp.h \
thread_group.h \
pool.h \
sigslot.h \
critical_section.h \
standard_serialize.h \
base_pack.h \
include.h \
net_address.h \
net_pack.h \
tcp_socket.h \
net_epoll.h \
udp_socket.h \
configure.h \
dynamic_lib.h \
tcp_stream.h \
libnet.h \
host_ip_cache.h \
http_client.h \
file_stat_manager.h \
base64.h \
time_base.h \
base_encrypt.h \
convert_charset.h \
file_lock.h

AUTOMAKE_OPTIONS=foreign
AM_CXXFLAGS = -fPIC
libdir=$(top_srcdir)/common/lib
libcommon_adir=$(top_srcdir)/common/include
lib_LIBRARIES = libcommon.a
#common_a_LDADD = -lssl
