//
// ClassLibrary.h
//
// Library: Foundation
// Package: SharedLibrary
// Module:  ClassLoader
//
// Definitions for class libraries.
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//


#ifndef Foundation_ClassLibrary_INCLUDED
#define Foundation_ClassLibrary_INCLUDED


#include "Foundation.h"
#include "Manifest.h"
#include <typeinfo>


#if defined(_WIN32)
    #define POCO_LIBRARY_API __declspec(dllexport)
#else
    #define POCO_LIBRARY_API
#endif


//
// the entry points for every class library
//
extern "C"
{
    bool POCO_LIBRARY_API pocoBuildManifest(Poco::ManifestBase* pManifest);
    void POCO_LIBRARY_API pocoInitializeLibrary();
    void POCO_LIBRARY_API pocoUninitializeLibrary();
}


//
// additional support for named manifests
//
#define POCO_DECLARE_NAMED_MANIFEST(name) \
extern "C"	\
{			\
    bool POCO_LIBRARY_API POCO_JOIN(pocoBuildManifest, name)(Poco::ManifestBase* pManifest); \
}


//
// Macros to automatically implement pocoBuildManifest
//
// usage:
//
// POCO_BEGIN_MANIFEST(MyBaseClass)
//     POCO_EXPORT_CLASS(MyFirstClass)
//     POCO_EXPORT_CLASS(MySecondClass)
//     ...
// POCO_END_MANIFEST
//

#define POCO_BEGIN_MANIFEST_IMPL(fnName, ...) \
    bool fnName(Poco::ManifestBase* pManifest_) \
    { \
        typedef Poco::Manifest<__VA_ARGS__> _Manifest; \
        std::string requiredType(typeid(_Manifest).name()); \
        std::string actualType(pManifest_->className()); \
        if (requiredType == actualType) \
        { \
            auto pManifest = static_cast<_Manifest*>(pManifest_); \


#define POCO_BEGIN_MANIFEST(...) \
    POCO_BEGIN_MANIFEST_IMPL(pocoBuildManifest, __VA_ARGS__)

#define POCO_BEGIN_NAMED_MANIFEST(name, ...)  \
    POCO_DECLARE_NAMED_MANIFEST(name)                  \
    POCO_BEGIN_MANIFEST_IMPL(POCO_JOIN(pocoBuildManifest, name), __VA_ARGS__)

#define POCO_END_MANIFEST \
            return true;	\
        }					\
        else return false;	\
    }


#define POCO_EXPORT_CLASS(cls) \
    pManifest->add<cls>(#cls);


#endif // Foundation_ClassLibrary_INCLUDED
