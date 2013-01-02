#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include "fileutils.h"

/**
 * remove a directory along with all its contents
 * @param   dir     path of directory to remove
 * @param   rmSelf  remove dir and its contents if true; remove contents only and keep dir itself if false
 * @return  true on success; false on errors
 */
bool FileUtils::removeDir(const QString &dir, bool rmSelf)
{
    bool result = true;
    QDir path(dir);

    if (path.exists(dir))
    {
        Q_FOREACH(QFileInfo info,
                  path.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files,
                                     QDir::DirsFirst))
        {
            if (info.isDir())
                result = removeDir(info.absoluteFilePath(), true);
            else
                result = QFile::remove(info.absoluteFilePath());
            if (!result)
                return result;
        }
        if (rmSelf)
            result = path.rmdir(dir);
    }
    return result;
}
