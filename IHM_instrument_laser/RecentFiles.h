#ifndef RECENTFILES_H
#define RECENTFILES_H

#include "lib/yyjson.h"

#include <QStringList>

class RecentFiles
{
private:
    QStringList m_ListSoundFonts;
    QStringList m_ListConfigs;

    char** m_pointers;
    unsigned int m_idPointer;

    void setListJson(yyjson_val* key, QStringList& List);
    void addQStrArray(yyjson_mut_doc* doc, yyjson_mut_val* arr, QStringList& Qstr);

    void addQStringList(QStringList& StrList, QString& QStr);

public:
    RecentFiles();
    void write();

    QStringList getListSoundFonts();
    QStringList getListConfigs();

    void addListConfigs(QString& QStr);
    void addListSoundFonts(QString& QStr);
};

#endif // RECENTFILES_H
