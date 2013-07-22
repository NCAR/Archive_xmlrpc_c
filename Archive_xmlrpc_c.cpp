/*
 * Archive_xmlrpc_c.cpp
 *
 *  Created on: Jun 27, 2013
 *      Author: burghart
 */

#define BOOST_ARCHIVE_SOURCE

#include "Archive_xmlrpc_c.h"

// we must explicitly instantiate some implementation for this type of stream
#include <boost/archive/impl/archive_serializer_map.ipp>
template class boost::archive::detail::common_oarchive<Oarchive_xmlrpc_c>;
template class boost::archive::detail::common_iarchive<Iarchive_xmlrpc_c>;
template class boost::archive::detail::archive_serializer_map<Iarchive_xmlrpc_c>;
