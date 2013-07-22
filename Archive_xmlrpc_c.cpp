/*
 * Archive_xmlrpc_c.cpp
 *
 *  Created on: Jun 27, 2013
 *      Author: burghart
 */

// Verify that we're compiling against Boost 1.42 or later. With Boost 1.41,
// we can end up with link time errors like this:
//    undefined reference to `boost::archive::detail::archive_serializer_map<Iarchive_xmlrpc_c>::find(boost::serialization::extended_type_info const&)'
#include <boost/version.hpp>
#if BOOST_VERSION < 104200
#   error "Boost version 1.42 or later is required for the archive_xmlrpc_c tool!"
#endif

#define BOOST_ARCHIVE_SOURCE

#include "Archive_xmlrpc_c.h"

// we must explicitly instantiate some implementation for this type of stream
#include <boost/archive/impl/archive_serializer_map.ipp>
template class boost::archive::detail::common_oarchive<Oarchive_xmlrpc_c>;
template class boost::archive::detail::common_iarchive<Iarchive_xmlrpc_c>;
template class boost::archive::detail::archive_serializer_map<Iarchive_xmlrpc_c>;
