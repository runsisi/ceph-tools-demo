/*
 * list_snaps.cc
 *
 *  Created on: Jul 15, 2020
 *      Author: runsisi
 */

#include "include/rados/librados.hpp"
#include "include/rbd/librbd.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <string.h>

#include "third/CLI11.hpp"
#include "third/json.hpp"
#include "third/scope_guard.hpp"
#include "third/spdlog/spdlog.h"

#include "common/Cond.h"
#include "common/ceph_argparse.h"
#include "include/Context.h"
#include "librbd/Utils.h"

namespace logger = spdlog;
using json = nlohmann::json;

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

  // from default env CEPH_ARGS
  cluster.conf_parse_env(nullptr);

  if (!conf_file_list.empty()) {
    r = cluster.conf_read_file(conf_file_list.c_str());
  } else {
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

namespace librados {
  // since v3.9.0
//  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(clone_info_t,
//      cloneid,
//      snaps,
//      overlap,
//      size
//  )
//  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(snap_set_t,
//      seq,
//      clones
//  )

  void to_json(json& j, const clone_info_t& c) {
    j = json{
      {"cloneid", int64_t(c.cloneid)},
      {"snaps", c.snaps},
      {"overlap", c.overlap},
      {"size", c.size}
    };
  }

  void to_json(json& j, const snap_set_t& ss) {
    j = json{
      {"seq", ss.seq},
      {"clones", ss.clones}
    };
  }
}

int main(int argc, const char** argv) {
  CLI::App app{"list-snaps"};

  std::string pool_name{"rbd"};
  std::string oid;

  app.add_option("-p,--pool", pool_name, "pool name");
  app.add_option("oid", oid, "object name")
      ->required(true);

  int rem_argc = 0;
  const char** rem_argv = new const char*[argc];
  const auto sg_del_rem_argv = sg::make_scope_guard([rem_argv]() {
    delete rem_argv;
  });

  std::string error;
  librados::Rados cluster;
  int r = connect_cluster(cluster, argc, argv, &rem_argc, rem_argv, error);
  if (r < 0) {
    logger::error(error);
    return r;
  }
  const auto sg_shutdown = sg::make_scope_guard([&cluster]() {
    cluster.shutdown();
  });

  CLI11_PARSE(app, rem_argc, rem_argv);

  librados::IoCtx ioctx;
  r = cluster.ioctx_create(pool_name.c_str(), ioctx);
  if (r < 0) {
    logger::error("cluster.ioctx_create failed with error: {0}", strerror(-r));
    return r;
  }

  C_SaferCond cond;
  librados::AioCompletion *comp = librbd::util::create_rados_callback(&cond);
  librados::ObjectReadOperation op;
  librados::snap_set_t ss;
  int ss_r;
  op.list_snaps(&ss, &ss_r);

  ioctx.snap_set_read(CEPH_SNAPDIR);
  r = ioctx.aio_operate(oid, comp, &op, nullptr);
  assert(r == 0);
  comp->release();

  r = cond.wait();
  if (r < 0) {
    logger::error("ioctx.aio_operate failed with error: {0}", strerror(-r));
    return r;
  }
  if (ss_r < 0) {
    logger::error("op.list_snaps failed with error: {0}", strerror(-ss_r));
    return r;
  }

  // print snap set
  json j = ss;
  std::cout << j.dump(2) << std::endl;

  return 0;
}
