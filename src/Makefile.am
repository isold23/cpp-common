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

AUTOMAKE_OPTIONS=foreign
AM_CXXFLAGS = -fPIC
INCLUDES= -I$(top_srcdir)/inc
libdir=$(top_srcdir)/lib
lib_LIBRARIES = libcommon.a
