/*
 * global_init.cc
 *
 *  Created on: Dec 9, 2020
 *      Author: runsisi
 */

// cd to root dir of ceph code, then:
//
// g++ -std=c++11 global_init.cc -o global-init
// -Ibuild/boost/include -Ibuild/include -Ibuild/src/include -Isrc
// -Lbuild/boost/lib -Lbuild/lib
// -lglobal -lceph-common -lerasure_code -ldl -lpthread

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>

#include "common/ceph_argparse.h"
#include "common/config.h"
#include "common/debug.h"
#include "common/errno.h"
#include "global/global_init.h"
#include "global/signal_handler.h"

#define dout_context g_ceph_context
#define dout_subsys ceph_subsys_client
#undef dout_prefix
#define dout_prefix *_dout << __func__ << ": "

std::mutex m_lock{};
std::condition_variable m_cond{};
std::atomic<bool> m_stopping = { false };

static void handle_signal(int signum)
{
  dout(20) << signum << dendl;

  std::lock_guard<std::mutex> l(m_lock);

  switch (signum) {
  case SIGHUP:
    break;
  case SIGINT:
  case SIGTERM:
    m_stopping = true;
    m_cond.notify_one();
    break;
  default:
    ceph_abort();
  }
}

int main(int argc, const char **argv)
{
    std::vector<const char*> args;
    env_to_vec(args);
    argv_to_vec(argc, argv, args);

    auto cct = global_init(nullptr, args, CEPH_ENTITY_TYPE_CLIENT,
            CODE_ENVIRONMENT_DAEMON, CINIT_FLAG_UNPRIVILEGED_DAEMON_DEFAULTS);

    for (auto i = args.begin(); i != args.end(); ++i)
    {
        if (ceph_argparse_flag(args, i, "-h", "--help", (char*) NULL))
        {
            return 0;
        }
    }

    if (g_conf->daemonize)
    {
        global_init_daemonize (g_ceph_context);
    }
    g_ceph_context->enable_perf_counter();

    common_init_finish (g_ceph_context);

    init_async_signal_handler();
    register_async_signal_handler(SIGHUP, handle_signal);
    register_async_signal_handler_oneshot(SIGINT, handle_signal);
    register_async_signal_handler_oneshot(SIGTERM, handle_signal);

    std::vector<const char*> cmd_args;
    argv_to_vec(argc, argv, cmd_args);

    while (!m_stopping) {
        std::unique_lock<std::mutex> l(m_lock);
        dout(0) << "running.." << dendl;
        m_cond.wait_for(l, std::chrono::duration<double>(2.0));
    }

    unregister_async_signal_handler(SIGHUP, handle_signal);
    unregister_async_signal_handler(SIGINT, handle_signal);
    unregister_async_signal_handler(SIGTERM, handle_signal);
    shutdown_async_signal_handler();

    return 0;
}
