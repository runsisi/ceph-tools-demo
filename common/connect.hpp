/*
 * connect.hpp
 *
 *  Created on: Jul 21, 2020
 *      Author: runsisi
 */

#ifndef COMMON_CONNECT_HPP_
#define COMMON_CONNECT_HPP_

#include <string>

#include "include/rados/librados.hpp"

int connect_cluster(librados::Rados& cluster,
    int argc, const char** argv,
    int* rem_argc, const char** rem_argv,
    std::string& error);

#endif /* COMMON_CONNECT_HPP_ */
