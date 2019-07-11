#include "SharedLibrary_UNIX.h"
#include "Exception.h"
#include <dlfcn.h>

namespace Poco {


FastMutex SharedLibraryImpl::_mutex;


SharedLibraryImpl::SharedLibraryImpl()
{
    _handle = 0;
}


SharedLibraryImpl::~SharedLibraryImpl()
{
}


void SharedLibraryImpl::loadImpl(const std::string& path, int flags)
{
    ScopedLock lock(_mutex);

    if (_handle) throw LibraryAlreadyLoadedException(path);
    int realFlags = RTLD_NOW;
    if (flags & SHLIB_LOCAL_IMPL)
        realFlags |= RTLD_LOCAL;
    else
        realFlags |= RTLD_GLOBAL;
    _handle = dlopen(path.c_str(), realFlags);
    if (!_handle)
    {
        const char* err = dlerror();
        throw LibraryLoadException(err ? std::string(err) : path);
    }
    _path = path;
}


void SharedLibraryImpl::unloadImpl()
{
    ScopedLock lock(_mutex);

    if (_handle)
    {
        dlclose(_handle);
        _handle = 0;
    }
}


bool SharedLibraryImpl::isLoadedImpl() const
{
    return _handle != 0;
}


void* SharedLibraryImpl::findSymbolImpl(const std::string& name)
{
    ScopedLock lock(_mutex);

    void* result = 0;
    if (_handle)
    {
        result = dlsym(_handle, name.c_str());
    }
    return result;
}


const std::string& SharedLibraryImpl::getPathImpl() const
{
    return _path;
}


std::string SharedLibraryImpl::suffixImpl()
{
    return ".so";
}


} // namespace Poco
