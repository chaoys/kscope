#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QString>

class FileUtils
{
public:
    static bool removeDir(const QString &dir, bool rmSelf);
};

#endif // FILEUTILS_H
