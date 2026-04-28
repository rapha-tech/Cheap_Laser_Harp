#include "RecentFiles.h"

//     {
//        "ListSoundFonts":["Florestan.sf2", "SoundFont.sf2"],
//        "ListConfigs":["config.json", "piano.json", "musique1.json"],
//     }

#include <QDebug>

#define RECENT_FILES_PATH "RecentFiles.json"

RecentFiles::RecentFiles()
{
    // Default values if no file is found
    m_ListSoundFonts = QStringList();
    m_ListConfigs = QStringList();

    // Read the JSON file
    yyjson_doc *doc = yyjson_read_file(RECENT_FILES_PATH, 0, NULL, NULL);

    // Iterate over the root object
    if (doc)
    {
        yyjson_val *obj = yyjson_doc_get_root(doc);
        yyjson_obj_iter iter;
        yyjson_obj_iter_init(obj, &iter);
        yyjson_val *key;
        while ((key = yyjson_obj_iter_next(&iter)))
        {
            if(!strcmp(yyjson_get_str(key), "ListSoundFonts"))
            {
                setListJson(key, m_ListSoundFonts);
            }
            else if(!strcmp(yyjson_get_str(key), "ListConfigs"))
            {
                setListJson(key, m_ListConfigs);
            }
        }
    }
    else
    {
        qDebug() <<"RecentFile : read error";
    }

    // Free the doc
    yyjson_doc_free(doc);
}

void RecentFiles::setListJson(yyjson_val* key, QStringList& QstrList)
{
    yyjson_val *val;
    yyjson_val *arr = yyjson_obj_iter_get_val(key);
    yyjson_arr_iter iter = yyjson_arr_iter_with(arr);

    while ((val = yyjson_arr_iter_next(&iter)))
    {
        QString string = QString(yyjson_get_str(val));
        QstrList.append(string);
    }
}


void RecentFiles::write()
{
    // Create a mutable document. ie can be modified
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);

    // Create an object, the value's memory is held by doc.
    yyjson_mut_val *root = yyjson_mut_obj(doc);

    // notes arrays
    yyjson_mut_val *ListSoundFonts_arr = yyjson_mut_arr(doc);
    yyjson_mut_val *ListConfigs_arr = yyjson_mut_arr(doc);

    unsigned int Npointers = m_ListSoundFonts.length() + m_ListConfigs.length();
    m_pointers = new char*[Npointers];
    m_idPointer = 0;

    addQStrArray(doc, ListSoundFonts_arr, m_ListSoundFonts);
    addQStrArray(doc, ListConfigs_arr, m_ListConfigs);

    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "ListSoundFonts"), ListSoundFonts_arr);
    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "ListConfigs"), ListConfigs_arr);

    // Set the document's root value.
    yyjson_mut_doc_set_root(doc, root);

    bool suc = yyjson_mut_write_file(RECENT_FILES_PATH, doc, YYJSON_WRITE_PRETTY_TWO_SPACES, NULL, NULL);
    if (suc)
        qDebug() << "RecentFile : successfully saved";

    // Free the memory of doc and all values which is created from this doc.
    yyjson_mut_doc_free(doc);

    for(unsigned int i = 0; i < Npointers; i++)
    {
        delete m_pointers[i];
    }
    delete[] m_pointers;
}

void RecentFiles::addQStrArray(yyjson_mut_doc* doc, yyjson_mut_val* arr, QStringList& QstrList)
{
    yyjson_mut_val *value;
    for(int i = 0; i < QstrList.count(); i++)
    {
        char* str = new char[QstrList[i].length() + 1];
        strcpy(str, QstrList[i].toLocal8Bit());
        value = yyjson_mut_str(doc, str);
        yyjson_mut_arr_append(arr, value);

        m_pointers[m_idPointer++] = str; // store the pointer for later delete
    }
}

QStringList RecentFiles::getListConfigs()
{
    return m_ListConfigs;
}

QStringList RecentFiles::getListSoundFonts()
{
    return m_ListSoundFonts;
}

void RecentFiles::addListConfigs(QString& QStr)
{
    addQStringList(m_ListConfigs, QStr);
}

void RecentFiles::addListSoundFonts(QString& QStr)
{
    addQStringList(m_ListSoundFonts, QStr);
}

void RecentFiles::addQStringList(QStringList& StrList, QString& QStr)
{
    if(!StrList.contains(QStr))
    {
        // the element is not in the list
        QStringList TempList = QStringList();

        TempList.append(QStr);
        for(int i = 0; i < StrList.count(); i++)
        {
            TempList.append(StrList[i]);
        }
        StrList = TempList;
        qDebug() << "RecentFile : add Qstr :" << QStr;
    }
    else
    {
        // handle the case of an already present file that has to go on top of the list
        QStringList TempList = QStringList();

        TempList.append(QStr);
        for(int i = 0; i < StrList.count(); i++)
        {
            if(QStr != StrList[i])
                TempList.append(StrList[i]);
        }
        StrList = TempList;
        qDebug() << "RecentFile : place Qstr on top :" << QStr;
    }
}