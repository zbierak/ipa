cmake_minimum_required (VERSION 2.8.11)
project(ipa)

file(GLOB sources "*.c")
add_executable(ipa ${sources})
set_property(TARGET ipa PROPERTY C_STANDARD 11)
