#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <cstdlib>

class FileSystem
{
private:
    typedef std::string (*Builder) (const std::string& path);
    
public:
    static std::string getPath(const std::string& path)
    {
        static std::string(*pathBuilder)(std::string const &) = getPathBuilder();
        return ((*pathBuilder)(path)).c_str();
    }
    
private:
    static std::string const & getRoot()
    {
        string path = __BASE_FILE__;
        path = path.substr(0, path.size()-17);
        
        static char const * envRoot = getenv("LOGL_ROOT_PATH");
        static std::string root = (envRoot != nullptr ? envRoot : path);
        return root;
    }
    
    static Builder getPathBuilder()
    {
        if (getRoot() != "")
            return &FileSystem::getPathRelativeRoot;
        else
            return &FileSystem::getPathRelativeBinary;
    }
    
    static std::string getPathRelativeRoot(const std::string& path)
    {
        return getRoot() + std::string("/") + path;
    }
    
    static std::string getPathRelativeBinary(const std::string& path)
    {
        return "../../../" + path;
    }
    
    
};

// FILESYSTEM_H
#endif
