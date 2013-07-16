// Boost archive classes to serialize to/from xmlrpc_c::value_struct dictionaries.
#ifndef _ARCHIVE_XMLRPC_C_H_
#define _ARCHIVE_XMLRPC_C_H_

#include <map>
#include <xmlrpc-c/base.hpp>
#include <boost/archive/detail/common_iarchive.hpp>
#include <boost/archive/detail/common_oarchive.hpp>

using namespace xmlrpc_c;

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
    
    // Handle name-value pairs (nvp). By default, complain about unhandled type.
    template<class T>
    void save_override(const boost::serialization::nvp<T> & pair, BOOST_PFTO int) {
        std::cerr << "Oarchive_xmlrpc_c has no save_override for NVP " <<
                "with value type '" << typeid(T).name() << "'";
    }

    // bool name-value pair handling
    void save_override(const boost::serialization::nvp<bool> & pair, BOOST_PFTO int) {
        _dict[pair.name()] = xmlrpc_c::value_boolean(pair.value());
    }

    // int name-value pair handling
    void save_override(const boost::serialization::nvp<int> & pair, BOOST_PFTO int) {
        _dict[pair.name()] = xmlrpc_c::value_int(pair.value());
    }

    // uint16_t name-value pair handling
    void save_override(const boost::serialization::nvp<uint16_t> & pair, BOOST_PFTO int) {
        _dict[pair.name()] = xmlrpc_c::value_int(pair.value());
    }

    // double name-value pair handling
    void save_override(const boost::serialization::nvp<double> & pair, BOOST_PFTO int) {
        _dict[pair.name()] = xmlrpc_c::value_double(pair.value());
    }

    // float name-value pair handling
    void save_override(const boost::serialization::nvp<float> & pair, BOOST_PFTO int) {
        _dict[pair.name()] = xmlrpc_c::value_double(pair.value());
    }
    
    // Not sure why we need this, but things won't compile without it...
    template<class T>
    void save(T & t) {
        std::cerr <<
            "Oarchive_xmlrpc_c only deals with name-value pairs, " <<
            "failed to save from (mangled) type: " << typeid(T).name() <<
            std::endl;
        std::cerr << "(Try 'c++filt -t <type>' to demangle the type name.)" <<
            std::endl;
        abort();
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
        _archiveMap(static_cast<const std::map<std::string, xmlrpc_c::value> >(archive)) {}
    
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
            std::cerr << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'!" << std::endl;
            for (std::map<std::string, xmlrpc_c::value>::const_iterator it = _archiveMap.begin(); it != _archiveMap.end(); ++it)
                std::cerr << it->first << std::endl;

            abort();  
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
    
    // Extract name-value pairs (nvp) using key/value entries in our XmlRpcValue 
    // dictionary.
    template<class T>
    void load_override(
#ifndef BOOST_NO_FUNCTION_TEMPLATE_ORDERING
            const
#endif
            boost::serialization::nvp<T> & pair,
            BOOST_PFTO int)
    {
        std::cerr << "Iarchive_xmlrpc_c has no load_override for NVP " <<
                "with value type '" << typeid(T).name() << "'";
    }

    // Loader for name-value pair with bool value.
    void load_override(const boost::serialization::nvp<bool> & pair, BOOST_PFTO int) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::cerr << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'!" << std::endl;
            abort();
        }
        xmlrpc_c::value_boolean bval(_archiveMap.find(key)->second);
        pair.value() = static_cast<bool>(bval);
    }

    // Loader for name-value pair with int value.
    void load_override(const boost::serialization::nvp<int> & pair, BOOST_PFTO int) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::cerr << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'!" << std::endl;
            abort();
        }
        xmlrpc_c::value_int ival(_archiveMap.find(key)->second);
        pair.value() = static_cast<int>(ival);
    }

    // Loader for name-value pair with uint16_t value.
    void load_override(const boost::serialization::nvp<uint16_t> & pair, BOOST_PFTO int) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::cerr << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'!" << std::endl;
            abort();
        }
        xmlrpc_c::value_int ival(_archiveMap.find(key)->second);
        pair.value() = static_cast<uint16_t>(ival);
    }

    // Loader for name-value pair with double value.
    void load_override(const boost::serialization::nvp<double> & pair, BOOST_PFTO int) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::cerr << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'!" << std::endl;
            abort();
        }
        xmlrpc_c::value_double dval(_archiveMap.find(key)->second);
        pair.value() = static_cast<double>(dval);
    }
    
    // Loader for name-value pair with float value.
    void load_override(const boost::serialization::nvp<float> & pair, BOOST_PFTO int) {
        const char * key = pair.name();
        if (_archiveMap.find(key) == _archiveMap.end()) {
            std::cerr << "xmlrpc_c::value_struct dictionary does not contain requested key '" <<
                    key << "'!" << std::endl;
            abort();
        }
        xmlrpc_c::value_double dval(_archiveMap.find(key)->second);
        pair.value() = static_cast<float>(dval);
    }

    // Not sure why we need this, but things won't compile without it...
    template<class T>
    void load(T & t) {
        std::cerr <<
            "Iarchive_xmlrpc_c only deals with name-value pairs, " <<
            "failed to load to (mangled) type: " << typeid(T).name() <<
            std::endl;
        std::cerr << "(Try 'c++filt -t <type>' to demangle the type name.)" <<
            std::endl;
        abort();
    }

private:
    // For boost::serialization, we must make our superclass our friend!
    friend class boost::archive::detail::common_iarchive<Iarchive_xmlrpc_c>;
    const std::map<std::string, xmlrpc_c::value> _archiveMap;
};

#endif // ifndef _ARCHIVE_XMLRPC_C_H_
