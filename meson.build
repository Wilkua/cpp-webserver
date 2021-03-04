project('libweb++', 'cpp',
    version : '1.0.0',
    default_options : ['cpp_std=c++17'])

threads_dep = dependency('threads')
lib = library('web++',
    'TcpServer.cpp', 'HttpServer.cpp', 'Parser.cpp', 'Request.cpp',
    'Response.cpp', 'StandardHandlers.cpp', 'Logger.cpp',
    dependencies : threads_dep,
    version : '1.0.0')

executable('servd', 'main.cpp', link_with : lib)
