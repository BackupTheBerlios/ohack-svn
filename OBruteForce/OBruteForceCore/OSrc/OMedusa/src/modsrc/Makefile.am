
modulesdir = $(libdir)/medusa/modules

if BUILD_UNTESTED
  modules_PROGRAMS = telnet.mod smbnt.mod http.mod ssh.mod mssql.mod pop3.mod mysql.mod pcanywhere.mod ftp.mod imap.mod rsh.mod rexec.mod wrapper.mod rlogin.mod ncp.mod vnc.mod postgres.mod svn.mod cvs.mod vmauthd.mod snmp.mod smtp-vrfy.mod smtp-auth.mod
else
  modules_PROGRAMS = telnet.mod smbnt.mod http.mod ssh.mod mssql.mod pop3.mod mysql.mod pcanywhere.mod ftp.mod imap.mod rsh.mod rexec.mod wrapper.mod rlogin.mod ncp.mod vnc.mod postgres.mod svn.mod cvs.mod vmauthd.mod snmp.mod
endif

smbnt_mod_SOURCES = smbnt.c ../medusa-trace.c
telnet_mod_SOURCES = telnet.c ../medusa-trace.c
http_mod_SOURCES = http.c ../medusa-trace.c
ssh_mod_SOURCES = ssh.c ../medusa-trace.c
mssql_mod_SOURCES = mssql.c ../medusa-trace.c
pop3_mod_SOURCES = pop3.c ../medusa-trace.c
ftp_mod_SOURCES = ftp.c ../medusa-trace.c
imap_mod_SOURCES = imap.c ../medusa-trace.c
rsh_mod_SOURCES = rsh.c ../medusa-trace.c
rexec_mod_SOURCES = rexec.c ../medusa-trace.c
rlogin_mod_SOURCES = rlogin.c ../medusa-trace.c
mysql_mod_SOURCES = mysql.c ../medusa-trace.c sha1.c
pcanywhere_mod_SOURCES = pcanywhere.c ../medusa-trace.c
wrapper_mod_SOURCES = wrapper.c ../medusa-trace.c
vnc_mod_SOURCES = vnc.c d3des.c ../medusa-trace.c
ncp_mod_SOURCES = ncp.c ../medusa-trace.c
postgres_mod_SOURCES = postgres.c ../medusa-trace.c
svn_mod_SOURCES = svn.c ../medusa-trace.c
cvs_mod_SOURCES = cvs.c ../medusa-trace.c
vmauthd_mod_SOURCES = vmauthd.c ../medusa-trace.c
snmp_mod_SOURCES = snmp.c ../medusa-trace.c
smtp_vrfy_mod_SOURCES = smtp-vrfy.c ../medusa-trace.c
smtp_auth_mod_SOURCES = smtp-auth.c ../medusa-trace.c

INCLUDES = -I$(top_srcdir)/src $(all_includes)
smbnt_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@
telnet_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@
http_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@
ssh_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@
mssql_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@

pop3_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@
ftp_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@
imap_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@
rsh_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@
rexec_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@
rlogin_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@
mysql_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@
pcanywhere_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@
wrapper_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@
vnc_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@
ncp_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@ 
postgres_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@ 
svn_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@ 
cvs_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@ 
vmauthd_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@ 
snmp_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@ 
smtp_vrfy_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@ 
smtp_auth_mod_LDFLAGS = -module -fPIC @MODULE_LDFLAGS@ 

noinst_HEADERS = module.h d3des.h sha1.h
EXTRA_DIST = wrapper/*.pl
