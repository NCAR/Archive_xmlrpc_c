// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#ifndef _ARCHIVE_XMLRPC_C_H_
#define _ARCHIVE_XMLRPC_C_H_

#include <map>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <xmlrpc-c/base.hpp>
#include <boost/archive/detail/common_iarchive.hpp>
#include <boost/archive/detail/common_oarchive.hpp>
#include <boost/archive/detail/register_archive.hpp>

using namespace xmlrpc_c;

// Template forward reference
template<typename T> class XmlrpcSerializable;

/// @brief Boost output archive class to populate an xmlrpc_c::value_struct
/// dictionary
///
/// Here's a simple class with a templated serialize() method:
///
/// class foo {
/// public:
///   foo() { _a = -1; _b = -1.0; _c = true }
///   foo(const xmlrpc_c::value_struct & dict) { serialize(Iarchive_xmlrpc_c(dict); }
///   foo(const xmlrpc_c::value_struct & statusDict) {
///      // Create an input archiver wrapper around the xmlrpc_c::value_struct
///      // dictionary, and use serialize() to populate our members from its
///      // content.
///      Iarchive_xmlrpc_c iar(statusDict);
///         iar >> *this;
///   }
///
///   xmlrpc_c::value_struct
///   to_value_struct() {
///         xmlrpc_c::value_struct * vsPtr = 0;
///      Oarchive_xmlrpc_c oar(vsPtr);
///      oar << *this;
///      xmlrpc_c::value_struct retval(*vsPtr);
///      delete(vsPtr);
///      return(retval);
///   }
///
///   template<class Archive>
///   void serialize(Archive & ar, const int version) {
///      ar & BOOST_SERIALIZATION_NVP(_a);
///      ar & BOOST_SERIALIZATION_NVP(_b);
///      ar & BOOST_SERIALIZATION_NVP(_c);
///   }
/// private:
///   int _a;
///   double _b;
///   bool _c;
/// };
///

class Oarchive_xmlrpc_c :
    public boost::archive::detail::common_oarchive<Oarchive_xmlrpc_c> {
public:
    /// @brief Archive to the given dictionary mapping string keys to
    /// xmlrpc_c::value objects.
    Oarchive_xmlrpc_c(std::map<std::string, xmlrpc_c::value> & dict) :
    	_dict(dict) {}

#ifdef BOOST_PFTO
    // default processing - kick back to our superclass
    template<class T>
    void save_override(const T & t, BOOST_PFTO int) {
        boost::archive::detail::common_oarchive<Oarchive_xmlrpc_c>::save_override(t, 0);
    }

    // Add special key "class_version" in the dictionary to hold the version
    // number of the class we're archiving.
    void save_override(const boost::archive::version_type & t, BOOST_PFTO int) {
        _dict["class_version"] = xmlrpc_c::value_int(static_cast<const int>(t));
    }

    // Don't bother archiving tracking_type, class_id_optional_type Boost special values
    void save_override(const boost::archive::tracking_type & t, BOOST_PFTO int) {
//        std::cerr << "Oarchive_xmlrpc_c dropping tracking_type" << std::endl;
    }
    void save_override(const boost::archive::class_id_optional_type & t, BOOST_PFTO int) {
//        std::cerr << "Oarchive_xmlrpc_c dropping class_id_optional_type" << std::endl;
    }

    // Template save_override implementation for boost::serialization:nvp<T>
    // when T is an enumerated type
    template <typename T>
    void nvp_save_override(const boost::serialization::nvp<T> & pair,
                            std::true_type,     // is_enum
                            std::false_type     // is_class
                           ) {
        const char * key = pair.name();
        _dict[key] = xmlrpc_c::value_int(int(pair.value()));
    }

    // Template save_override implementation for boost::serialization:nvp<T>
    // when T is a class with a serialize() method
    template <typename T>
    void nvp_save_override(const boost::serialization::nvp<T> & pair,
                            std::false_type,    // is_enum
                            std::true_type      // is_class
                           ) {
        const char * key = pair.name();
        XmlrpcSerializable<T> sValue(pair.value());
        _dict[key] = sValue;
    }

    // Template save_override for boost::serialization::nvp<T>
    // (name/value pairs)
    //
    // This template uses one of the nvp_save_override specializations above,
    // selected at compile time based on T's type traits
    template<typename T>
    void save_override(const boost::serialization::nvp<T> & pair,
                       BOOST_PFTO int) {
        // Select implementation at compile time depending on whether T is an
        // enumerated type or a class
        nvp_save_override(pair, std::is_enum<T>{}, std::is_class<T>{});
    }

    // name-value pair handling for bool values
    void save_override(const boost::serialization::nvp<bool> & pair, BOOST_PFTO int) {
        _dict[pair.name()] = xmlrpc_c::value_boolean(pair.value());
    }

    // name-value pair handling for int values
    void save_override(const boost::serialization::nvp<int> & pair, BOOST_PFTO int) {
        _dict[pair.name()] = xmlrpc_c::value_int(pair.value());
    }

    // name-value pair handling for uint8_t values
    void save_override(const boost::serialization::nvp<uint8_t> & pair, BOOST_PFTO int) {
        _dict[pair.name()] = xmlrpc_c::value_int(pair.value());
    }

    // name-value pair handling for uint16_t values
    void save_override(const boost::serialization::nvp<uint16_t> & pair, BOOST_PFTO int) {
        _dict[pair.name()] = xmlrpc_c::value_int(pair.value());
    }

    // name-value pair handling for uint32_t values
    void save_override(const boost::serialization::nvp<uint32_t> & pair, BOOST_PFTO int) {
        // Reinterpret the unsigned value as signed, and save it in that form.
        // The matching load_override() will change it back to unsigned.
        uint32_t uval = pair.value();
        int * ival_p = reinterpret_cast<int *>(&uval);
        _dict[pair.name()] = xmlrpc_c::value_int(*ival_p);
    }

    // name-value pair handling for uint64_t values
    void save_override(const boost::serialization::nvp<uint64_t> & pair, BOOST_PFTO int) {
        // Reinterpret the unsigned value as signed, and save it in that form.
        // The matching load_override() will change it back to unsigned.
        uint64_t uval = pair.value();
        int64_t * ival_p = reinterpret_cast<int64_t *>(&uval);
        _dict[pair.name()] = xmlrpc_c::value_i8(*ival_p);
    }

    // name-value pair handling or long values
    void save_override(const boost::serialization::nvp<long> & pair, BOOST_PFTO int) {
        _dict[pair.name()] = xmlrpc_c::value_int(pair.value());
    }

    // name-value pair handling for double values
    void save_override(const boost::serialization::nvp<double> & pair, BOOST_PFTO int) {
        _dict[pair.name()] = xmlrpc_c::value_double(pair.value());
    }

    // name-value pair handling for float values
    void save_override(const boost::serialization::nvp<float> & pair, BOOST_PFTO int) {
        _dict[pair.name()] = xmlrpc_c::value_double(pair.value());
    }

    // name-value pair handling for std::string values
    void save_override(const boost::serialization::nvp<std::string> & pair, BOOST_PFTO int) {
        _dict[pair.name()] = xmlrpc_c::value_string(pair.value());
    }
#else
    // default processing - kick back to our superclass
    template<class T>
    void save_override(const T & t) {
        boost::archive::detail::common_oarchive<Oarchive_xmlrpc_c>::save_override(t);
    }

    // Add special key "class_version" in the dictionary to hold the version
    // number of the class we're archiving.
    void save_override(const boost::archive::version_type & t) {
        _dict["class_version"] = xmlrpc_c::value_int(static_cast<const int>(t));
    }

    // Don't bother archiving tracking_type, class_id_optional_type Boost special values
    void save_override(const boost::archive::tracking_type & t) {
//        std::cerr << "Oarchive_xmlrpc_c dropping tracking_type" << std::endl;
    }
    void save_override(const boost::archive::class_id_optional_type & t) {
//        std::cerr << "Oarchive_xmlrpc_c dropping class_id_optional_type" << std::endl;
    }

    // Template save_override implementation for boost::serialization:nvp<T>
    // when T is an enumerated type
    template <typename T>
    void nvp_save_override(const boost::serialization::nvp<T> & pair,
                           std::true_type,     // is_enum
                           std::false_type     // is_class
                          ) {
        const char * key = pair.name();
        _dict[key] = xmlrpc_c::value_int(int(pair.value()));
    }

    // Template save_override implementation for boost::serialization:nvp<T>
    // when T is a class with a serialize() method
    template <typename T>
    void nvp_save_override(const boost::serialization::nvp<T> & pair,
                           std::false_type,    // is_enum
                           std::true_type      // is_class
                          ) {
        const char * key = pair.name();
        XmlrpcSerializable<T> sValue(pair.value());
        _dict[key] = sValue;
    }

    // Template save_override for boost::serialization::nvp<T>
    // (name/value pairs)
    //
    // This template uses one of the nvp_save_override specializations above,
    // selected at compile time based on T's type traits
    template<typename T>
    void save_override(const boost::serialization::nvp<T> & pair) {
        // Select implementation at compile time depending on whether T is an
        // enumerated type or a class
        nvp_save_override(pair, std::is_enum<T>{}, std::is_class<T>{});
    }

    // name-value pair handling for bool values
    void save_override(const boost::serialization::nvp<bool> & pair) {
        _dict[pair.name()] = xmlrpc_c::value_boolean(pair.value());
    }

    // name-value pair handling for int values
    void save_override(const boost::serialization::nvp<int> & pair) {
        _dict[pair.name()] = xmlrpc_c::value_int(pair.value());
    }

    // name-value pair handling for uint8_t values
    void save_override(const boost::serialization::nvp<uint8_t> & pair) {
        _dict[pair.name()] = xmlrpc_c::value_int(pair.value());
    }

    // name-value pair handling for uint16_t values
    void save_override(const boost::serialization::nvp<uint16_t> & pair) {
        _dict[pair.name()] = xmlrpc_c::value_int(pair.value());
    }

    // name-value pair handling for uint32_t values
    void save_override(const boost::serialization::nvp<uint32_t> & pair) {
        // Reinterpret the unsigned value as signed, and save it in that form.
        // The matching load_override() will change it back to unsigned.
        uint32_t uval = pair.value();
        int * ival_p = reinterpret_cast<int *>(&uval);
        _dict[pair.name()] = xmlrpc_c::value_int(*ival_p);
    }

    // name-value pair handling for uint64_t values
    void save_override(const boost::serialization::nvp<uint64_t> & pair) {
        // Reinterpret the unsigned value as signed, and save it in that form.
        // The matching load_override() will change it back to unsigned.
        uint64_t uval = pair.value();
        int64_t * ival_p = reinterpret_cast<int64_t *>(&uval);
        _dict[pair.name()] = xmlrpc_c::value_i8(*ival_p);
    }

    // name-value pair handling or long values
    void save_override(const boost::serialization::nvp<long> & pair) {
        _dict[pair.name()] = xmlrpc_c::value_int(pair.value());
    }

    // name-value pair handling for double values
    void save_override(const boost::serialization::nvp<double> & pair) {
        _dict[pair.name()] = xmlrpc_c::value_double(pair.value());
    }

    // name-value pair handling for float values
    void save_override(const boost::serialization::nvp<float> & pair) {
        _dict[pair.name()] = xmlrpc_c::value_double(pair.value());
    }

    // name-value pair handling for std::string values
    void save_override(const boost::serialization::nvp<std::string> & pair) {
        _dict[pair.name()] = xmlrpc_c::value_string(pair.value());
    }
#endif // ifdef BOOST_PFTO

    // Not sure why we need this, but things won't compile without it...
    template<class T>
    void save(T & t) {
        std::ostringstream ss;
        ss << "Oarchive_xmlrpc_c only deals with name-value pairs, \n" <<
              "failed to save from (mangled) type: " <<
              typeid(T).name() << "\n" <<
              "\n(Try 'c++filt -t <type>' to demangle the type name.)";
        throw(std::runtime_error(ss.str()));
    }
private:
    friend class boost::archive::detail::common_oarchive<Oarchive_xmlrpc_c>;
    std::map<std::string, xmlrpc_c::value> & _dict;
};

/// @brief Boost input archive class to unpack from an xmlrpc_c::value_struct
/// or std::map<std::string, xmlrpc_c::value>.
class Iarchive_xmlrpc_c :
    public boost::archive::detail::common_iarchive<Iarchive_xmlrpc_c> {
public:
    Iarchive_xmlrpc_c(const std::map<std::string, xmlrpc_c::value> & map) :
    	_archiveMap(map) {}
    Iarchive_xmlrpc_c(const xmlrpc_c::value_struct & archive) :
        _archiveMap(static_cast<const std::map<std::string, xmlrpc_c::value>>(archive)) {}

#ifdef BOOST_PFTO
    // default processing - kick back to our superclass
    template<class T>
    void load_override(T & t, BOOST_PFTO int) {
        boost::archive::detail::common_iarchive<Iarchive_xmlrpc_c>::load_override(t, 0);
    }

    // Get class version number from special key "class_version" in the
    // xmlrpc_c::value_struct dictionary.
    void load_override(boost::archive::version_type & t, BOOST_PFTO int) {
        const std::string key("class_version");
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value_int ival(_archiveMap.find(key)->second);
        t = boost::archive::version_type(static_cast<int>(ival));
    }

    // Don't bother loading tracking_type and class_id_optional_type Boost
    // special values
    void load_override(boost::archive::tracking_type & t, BOOST_PFTO int) {
//        std::cerr << "Iarchive_xmlrpc_c not loading tracking_type" << std::endl;
    }
    void load_override(boost::archive::class_id_optional_type & t, BOOST_PFTO int) {
//        std::cerr << "Iarchive_xmlrpc_c not loading class_id_optional_type" << std::endl;
    }

    // Template load_override implementation for boost::serialization:nvp<T>
    // when T is an enumerated type
    template <typename T>
    void nvp_load_override(const boost::serialization::nvp<T> & pair,
                           std::true_type,     // is_enum
                           std::false_type     // is_class
                          ) {
        const char * key = pair.name();
        auto archiveIter = _archiveMap.find(key);
        if (archiveIter == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                  key << "'";
            throw(std::runtime_error(ss.str()));
        }
        // The returned value should be of type xmlrpc_c::value_int. If it
        // isn't, the value_int cast below will throw an exception
        int intVal(xmlrpc_c::value_int(archiveIter->second));
        // Cast the integer value to the enumerated type
        pair.value() = static_cast<T>(intVal);
    }

    // Template load_override implementation for boost::serialization:nvp<T>
    // when T is a class with a serialize() method
    template <typename T>
    void nvp_load_override(const boost::serialization::nvp<T> & pair,
                           std::false_type,    // is_enum
                           std::true_type      // is_class
                          ) {
        const char * key = pair.name();
        auto archiveIter = _archiveMap.find(key);
        if (archiveIter == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                  key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value xmlrpcVal = archiveIter->second;
        XmlrpcSerializable<T> sValue(pair.value());
        pair.value() = sValue;
    }

    // Template load_override for boost::serialization::nvp<T>
    // (name/value pairs)
    //
    // This template uses one of the nvp_load_override specializations above,
    // selected at compile time based on T's type traits
    template<class T>
    void load_override(
#ifndef BOOST_NO_FUNCTION_TEMPLATE_ORDERING
            const
#endif
            boost::serialization::nvp<T> & pair,
            BOOST_PFTO int)
    {
        // Select implementation at compile time depending on whether T is an
        // enumerated type or a class
        nvp_load_override(pair, std::is_enum<T>{}, std::is_class<T>{});
    }

    // Loader for name-value pair with bool value
    void load_override(const boost::serialization::nvp<bool> & pair, BOOST_PFTO int) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value_boolean bval(_archiveMap.find(key)->second);
        pair.value() = static_cast<bool>(bval);
    }

    // Loader for name-value pair with int value
    void load_override(const boost::serialization::nvp<int> & pair, BOOST_PFTO int) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value_int ival(_archiveMap.find(key)->second);
        pair.value() = static_cast<int>(ival);
    }

    // Loader for name-value pair with uint8_t value
    void load_override(const boost::serialization::nvp<uint8_t> & pair, BOOST_PFTO int) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value_int ival(_archiveMap.find(key)->second);
        pair.value() = static_cast<uint8_t>(ival);
    }

    // Loader for name-value pair with uint16_t value
    void load_override(const boost::serialization::nvp<uint16_t> & pair, BOOST_PFTO int) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value_int ival(_archiveMap.find(key)->second);
        pair.value() = static_cast<uint16_t>(ival);
    }

    // Loader for name-value pair with uint32_t value
    void load_override(const boost::serialization::nvp<uint32_t> & pair, BOOST_PFTO int) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        // We get the value as a signed int (from the matching save_override()
        // above), and reinterpret to unsigned int.
        xmlrpc_c::value_int xml_ival(_archiveMap.find(key)->second);
        int ival = static_cast<int>(xml_ival);
        uint32_t * uval_p = reinterpret_cast<uint32_t *>(&ival);
        pair.value() = *uval_p;
    }

    // Loader for name-value pair with uint64_t value
    void load_override(const boost::serialization::nvp<uint64_t> & pair, BOOST_PFTO int) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        // We get the value as a signed int (from the matching save_override()
        // above), and reinterpret to unsigned int.
        xmlrpc_c::value_i8 xml_ival(_archiveMap.find(key)->second);
        int64_t ival = static_cast<int64_t>(xml_ival);
        uint64_t * uval_p = reinterpret_cast<uint64_t *>(&ival);
        pair.value() = *uval_p;
    }

    // Loader for name-value pair with long value
    void load_override(const boost::serialization::nvp<long> & pair, BOOST_PFTO int) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value_int ival(_archiveMap.find(key)->second);
        pair.value() = static_cast<long>(ival);
    }

    // Loader for name-value pair with double value
    void load_override(const boost::serialization::nvp<double> & pair, BOOST_PFTO int) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value_double dval(_archiveMap.find(key)->second);
        pair.value() = static_cast<double>(dval);
    }

    // Loader for name-value pair with float value
    void load_override(const boost::serialization::nvp<float> & pair, BOOST_PFTO int) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value_double dval(_archiveMap.find(key)->second);
        pair.value() = static_cast<float>(dval);
    }

    // Loader for name-value pair with std::string value
    void load_override(const boost::serialization::nvp<std::string> & pair, BOOST_PFTO int) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value_string sval(_archiveMap.find(key)->second);
        pair.value() = static_cast<std::string>(sval);
    }

#else
    // default processing - kick back to our superclass
    template<class T>
    void load_override(T & t) {
        boost::archive::detail::common_iarchive<Iarchive_xmlrpc_c>::load_override(t);
    }

    // Get class version number from special key "class_version" in the
    // xmlrpc_c::value_struct dictionary.
    void load_override(boost::archive::version_type & t) {
        const std::string key("class_version");
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                  key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value_int ival(_archiveMap.find(key)->second);
        t = boost::archive::version_type(static_cast<int>(ival));
    }

    // Don't bother loading tracking_type and class_id_optional_type Boost
    // special values
    void load_override(boost::archive::tracking_type & t) {
//        std::cerr << "Iarchive_xmlrpc_c not loading tracking_type" << std::endl;
    }
    void load_override(boost::archive::class_id_optional_type & t) {
//        std::cerr << "Iarchive_xmlrpc_c not loading class_id_optional_type" << std::endl;
    }

    // Template load_override implementation for boost::serialization:nvp<T>
    // when T is an enumerated type
    template <typename T>
    void nvp_load_override(const boost::serialization::nvp<T> & pair,
                           std::true_type,     // is_enum
                           std::false_type     // is_class
                          ) {
        const char * key = pair.name();
        auto archiveIter = _archiveMap.find(key);
        if (archiveIter == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                  key << "'";
            throw(std::runtime_error(ss.str()));
        }
        // The returned value should be of type xmlrpc_c::value_int. If it
        // isn't, the value_int cast below will throw an exception
        int intVal(xmlrpc_c::value_int(archiveIter->second));
        // Cast the integer value to the enumerated type
        pair.value() = static_cast<T>(intVal);
    }

    // Template load_override implementation for boost::serialization:nvp<T>
    // when T is a class with a serialize() method
    template <typename T>
    void nvp_load_override(const boost::serialization::nvp<T> & pair,
                           std::false_type,    // is_enum
                           std::true_type      // is_class
                          ) {
        const char * key = pair.name();
        auto archiveIter = _archiveMap.find(key);
        if (archiveIter == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                  key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value xmlrpcVal = archiveIter->second;
        XmlrpcSerializable<T> sValue(pair.value());
        pair.value() = sValue;
    }

    // Template load_override for boost::serialization::nvp<T>
    // (name/value pairs)
    //
    // This template uses one of the nvp_load_override specializations above,
    // selected at compile time based on T's type traits
    template<class T>
    void load_override(const boost::serialization::nvp<T> & pair)
    {
        // Select implementation at compile time depending on whether T is an
        // enumerated type or a class
        nvp_load_override(pair, std::is_enum<T>{}, std::is_class<T>{});
    }

    // Loader for name-value pair with bool value
    void load_override(const boost::serialization::nvp<bool> & pair) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value_boolean bval(_archiveMap.find(key)->second);
        pair.value() = static_cast<bool>(bval);
    }

    // Loader for name-value pair with int value
    void load_override(const boost::serialization::nvp<int> & pair) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value_int ival(_archiveMap.find(key)->second);
        pair.value() = static_cast<int>(ival);
    }

    // Loader for name-value pair with uint8_t value
    void load_override(const boost::serialization::nvp<uint8_t> & pair) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value_int ival(_archiveMap.find(key)->second);
        pair.value() = static_cast<uint8_t>(ival);
    }

    // Loader for name-value pair with uint16_t value
    void load_override(const boost::serialization::nvp<uint16_t> & pair) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value_int ival(_archiveMap.find(key)->second);
        pair.value() = static_cast<uint16_t>(ival);
    }

    // Loader for name-value pair with uint32_t value
    void load_override(const boost::serialization::nvp<uint32_t> & pair) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        // We get the value as a signed int (from the matching save_override()
        // above), and reinterpret to unsigned int.
        xmlrpc_c::value_int xml_ival(_archiveMap.find(key)->second);
        int ival = static_cast<int>(xml_ival);
        uint32_t * uval_p = reinterpret_cast<uint32_t *>(&ival);
        pair.value() = *uval_p;
    }

    // Loader for name-value pair with uint64_t value
    void load_override(const boost::serialization::nvp<uint64_t> & pair) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        // We get the value as a signed int (from the matching save_override()
        // above), and reinterpret to unsigned int.
        xmlrpc_c::value_i8 xml_ival(_archiveMap.find(key)->second);
        int64_t ival = static_cast<int64_t>(xml_ival);
        uint64_t * uval_p = reinterpret_cast<uint64_t *>(&ival);
        pair.value() = *uval_p;
    }

    // Loader for name-value pair with long value
    void load_override(const boost::serialization::nvp<long> & pair) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value_int ival(_archiveMap.find(key)->second);
        pair.value() = static_cast<long>(ival);
    }

    // Loader for name-value pair with double value
    void load_override(const boost::serialization::nvp<double> & pair) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value_double dval(_archiveMap.find(key)->second);
        pair.value() = static_cast<double>(dval);
    }

    // Loader for name-value pair with float value
    void load_override(const boost::serialization::nvp<float> & pair) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value_double dval(_archiveMap.find(key)->second);
        pair.value() = static_cast<float>(dval);
    }

    // Loader for name-value pair with std::string value
    void load_override(const boost::serialization::nvp<std::string> & pair) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::ostringstream ss;
            ss << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'";
            throw(std::runtime_error(ss.str()));
        }
        xmlrpc_c::value_string sval(_archiveMap.find(key)->second);
        pair.value() = static_cast<std::string>(sval);
    }
#endif // ifdef BOOST_PFTO
    // Not sure why we need this, but things won't compile without it...
    template<class T>
    void load(T & t) {
        std::ostringstream ss;
        ss << "Iarchive_xmlrpc_c only deals with name-value pairs, \n" <<
              "failed to load from (mangled) type: " <<
              typeid(T).name() << "\n" <<
              "\n(Try 'c++filt -t <type>' to demangle the type name.)";
        throw(std::runtime_error(ss.str()));
    }

private:
    // For boost::serialization, we must make our superclass our friend!
    friend class boost::archive::detail::common_iarchive<Iarchive_xmlrpc_c>;
    const std::map<std::string, xmlrpc_c::value> _archiveMap;
};

BOOST_SERIALIZATION_REGISTER_ARCHIVE(Oarchive_xmlrpc_c)
BOOST_SERIALIZATION_REGISTER_ARCHIVE(Iarchive_xmlrpc_c)

/// Mix-in class which allows objects of its class to be serialized to/from
/// Oarchive_xmlrpc_c/Iarchive_xmlrpc_c archives as composite members
/// of other classes.
///
/// Members of this class must provide a boost::serialization serialize()
/// template method which serializes to/from boost::serialization::nvp
/// name-value pair representation.
template<typename T>
class XmlrpcSerializable : public T {
public:
    /// @brief Default constructor
    XmlrpcSerializable() : T() {}

    /// @brief Constructor which copies from an instance of T
    /// @param t the instance of type T to copy
    XmlrpcSerializable(const T & t) : T(t) {}

    /// @brief Construct from an xmlrpc_c::value (which must be
    /// xmlrpc_c::value_struct)
    /// @param xmlrpcVal the xmlrpc_c::value holding the content from which
    /// to construct
    XmlrpcSerializable(const xmlrpc_c::value & xmlrpcVal) : T() {
        // Cast the xmlrpc_c::value to xmlrpc_c::value_struct, then from that
        // to std::map<std::string, xmlrpc_c::value>.
        xmlrpc_c::value_struct statusStruct(xmlrpcVal);
        std::map<std::string, xmlrpc_c::value> statusMap(statusStruct);

        // Create an input archiver wrapper around the map and use serialize()
        // to populate our members from its content.
        Iarchive_xmlrpc_c iar(statusMap);
        iar >> *this;
    }

    virtual ~XmlrpcSerializable() {};

    /// @brief Cast to xmlrpc_c::value
    operator xmlrpc_c::value() const { return(_toXmlRpcValueStruct()); }

private:
    /// @brief Return an xmlrpc_c::value containing a struct (dictionary) with
    /// the object's serialized representation
    xmlrpc_c::value_struct _toXmlRpcValueStruct() const {
        std::map<std::string, xmlrpc_c::value> statusMap;
        // Stuff our content into the statusMap, i.e., _serialize() to an
        // output archiver wrapped around the statusMap.
        Oarchive_xmlrpc_c oar(statusMap);
        oar << *this;
        // Finally, return a value_struct constructed from the map
        return(xmlrpc_c::value_struct(statusMap));
    }

};


#endif // ifndef _ARCHIVE_XMLRPC_C_H_
