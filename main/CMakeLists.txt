file(GLOB_RECURSE srcs "main.c" "src/*.c" "src/*.cpp")

message($srcs)
idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "./include")