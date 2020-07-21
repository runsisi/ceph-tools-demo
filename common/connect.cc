#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "third/scope_guard.hpp"

#include "include/rados/librados.hpp"
#include "common/ceph_argparse.h"

int connect_cluster(librados::Rados& cluster,
    int argc, const char** argv,
    int* rem_argc, const char** rem_argv,
    std::string& error)
{
  vector<const char*> args;
  argv_to_vec(argc, argv, args);

  std::string cluster_name;
  std::string conf_file_list;
  CephInitParameters iparams = ceph_argparse_early_args(args,
      CEPH_ENTITY_TYPE_CLIENT, &cluster_name, &conf_file_list);

  std::string name = iparams.name.to_str();
  int r = cluster.init2(name.c_str(), cluster_name.c_str(), 0);
  if (r) {
    std::ostringstream oss;
    oss << "cluster.init2 failed with error: "
        << strerror(-r);
    error = oss.str();
    return r;
  }

  // parse env: CEPH_ARGS, CEPH_KEYRING, CEPH_LIB
  cluster.conf_parse_env(nullptr);

  if (!conf_file_list.empty()) {
    r = cluster.conf_read_file(conf_file_list.c_str());
  } else {
    // if cluster name not specified, then default it to 'ceph'
    // parse conf file CEPH_CONF, or default conf files CEPH_CONF_FILE_DEFAULT
    r = cluster.conf_read_file(nullptr);
  }
  if (r) {
    cluster.shutdown();
    std::ostringstream oss;
    oss << "cluster.conf_read_file failed with error: "
        << strerror(-r);
    error = oss.str();
    return r;
  }

  int new_argc;
  const char **new_argv;
  vec_to_argv(argv[0], args, &new_argc, &new_argv);
  const auto sg_free_argv = sg::make_scope_guard([new_argv]() {
    free(new_argv);
  });
  r = cluster.conf_parse_argv_remainder(new_argc, new_argv, rem_argv);
  if (r) {
    cluster.shutdown();
    std::ostringstream oss;
    oss << "cluster.conf_parse_argv_remainder failed with error: "
        << strerror(-r);
    error = oss.str();
    return r;
  }

  *rem_argc = 0;
  for (int i = 0; i < new_argc; i++) {
    if (rem_argv[i] != nullptr) {
      (*rem_argc)++;
    }
  }

  r = cluster.connect();
  if (r) {
    cluster.shutdown();
    std::ostringstream oss;
    oss << "cluster.connect failed with error: "
        << strerror(-r);
    error = oss.str();
    return r;
  }
  return 0;
}
