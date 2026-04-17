#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include "lib/yyjson.h"

#include <EngineLaser.h>
#include <QString>

class configFile
{
private:
    QString SoundFont_path;
    int instrument_id;
    int volume;
    int port_id;
    accord_t* m_accords;

public:
    configFile(QString&);
    void setAccordJson(yyjson_val* key, accord_t* accords, int id_laser);
    void addNotesArray(yyjson_mut_doc* doc, yyjson_mut_val* arr, int id_laser);
    void write(QString&);

    QString get_soundFont_path();
    int get_instr_id();
    int get_volume();
    int get_port_id();
    accord_t* getAccords();
    accord_t* get_default_accord();

    void set_soundFont_path(QString&);
    void set_instr_id(int);
    void set_volume(int);
    void set_port_id(int);
    void set_accords(accord_t* accordcpy);
};

#endif // CONFIGFILE_H
