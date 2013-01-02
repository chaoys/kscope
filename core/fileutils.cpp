#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include "fileutils.h"

/**
 * remove a directory along with all its contents
 * @param   dir path of directory to remove
 * @return  true on success; false on errors
 */
bool FileUtils::removeDir(const QString &dir)
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
                result = removeDir(info.absoluteFilePath());
            else
                result = QFile::remove(info.absoluteFilePath());
            if (!result)
                return result;
        }
        result = path.rmdir(dir);
    }
    return result;
}
