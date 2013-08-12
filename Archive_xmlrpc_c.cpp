/*
 * Archive_xmlrpc_c.cpp
 *
 *  Created on: Jun 27, 2013
 *      Author: burghart
 */
#define BOOST_ARCHIVE_SOURCE

#include <boost/version.hpp>
#include "Archive_xmlrpc_c.h"

#if (BOOST_VERSION == 104100)
   // For Boost 1.41, we must explicitly instantiate some implementation for
   // this type of stream
#  include <boost/archive/impl/archive_serializer_map.ipp>
   template class boost::archive::detail::common_oarchive<Oarchive_xmlrpc_c>;
   template class boost::archive::detail::common_iarchive<Iarchive_xmlrpc_c>;
   template class boost::archive::detail::archive_serializer_map<Iarchive_xmlrpc_c>;
#endif
