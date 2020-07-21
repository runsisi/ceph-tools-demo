/*
 * list_snaps.cc
 *
 *  Created on: Jul 15, 2020
 *      Author: runsisi
 */
#include <iostream>
#include <string>

#include "third/CLI11.hpp"
#include "third/json.hpp"
#include "third/scope_guard.hpp"
#include "third/spdlog/spdlog.h"

#include "include/rados/librados.hpp"
#include "include/rbd/librbd.hpp"
#include "common/Cond.h"
#include "librbd/Utils.h"

#include "common/connect.hpp"

namespace logger = spdlog;
using json = nlohmann::json;

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
  app.allow_extras(true);

  // print help
  CLI11_PARSE(app, argc, argv);

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

  // do the real parsing
  app.allow_extras(false);
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
