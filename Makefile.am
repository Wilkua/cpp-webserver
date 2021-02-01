AM_CXXFLAGS = -std=c++17

lib_LTLIBRARIES = libweb++.la
libweb___la_SOURCES = \
				   Server.cpp \
				   Response.cpp \
				   Request.cpp \
				   Parser.cpp \
				   StandardHandlers.cpp \
				   Logger.cpp
include_HEADERS = webserver.h Logger.h

bin_PROGRAMS = servd
servd_SOURCES = main.cpp
servd_LDADD = libweb++.la

