add_executable(list-snaps list_snaps.cc)

target_compile_options(list-snaps PRIVATE
  "-O0 -g"
)
target_link_libraries(list-snaps PRIVATE
  rbd
  rados
  ceph-common
)

install(TARGETS list-snaps DESTINATION bin)