bin_PROGRAMS = medusa
medusa_SOURCES = listModules.c medusa.c medusa-net.c medusa-trace.c medusa-utils.c http-ntlm.c

# set the include path found by configure
INCLUDES = -I$(top_srcdir)/src $(all_includes)


# the library search path.
#medusa_LDFLAGS = -rdynamic -ldl -lpthread -lssl
noinst_HEADERS = medusa.h medusa-net.h medusa-trace.h medusa-utils.h http-ntlm.h
#AM_CFLAGS = -DLIBOPENSSL
SUBDIRS = modsrc
